#include "ecfs.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/time.h>

using namespace std;

fileclient::fileclient()
{
    serverfd = -1;
    token = NULL;
}

fileclient::~fileclient()
{
    close (serverfd);
    serverfd = -1;
}

int fileclient::connect_to_server()
{
    int fd;
    int buffersize = 8388608; // 8 MB buffer
    struct sockaddr_un serveraddr;
    // SOCK_STREAM -> tcp
    fd = socket (AF_UNIX, SOCK_STREAM, 0);
    
    if (fd < 0)
    {
        cout << "[fileclient]Openning unix socket failed" << endl;
        exit (1);
    }
    
    Settings setted;
    setted.load();
    string ipc_path = setted.get<string>("path.ipc");

    memset ( (void*) &serveraddr, 0, sizeof (struct sockaddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy (serveraddr.sun_path, ipc_path.c_str());
    
    while (connect (fd, (struct sockaddr *) &serveraddr, sizeof (serveraddr)) < 0)
    {
        // sleep for 1 miilisecond
        usleep (1000);
    }
    
    // set socket to be nonblocking
    fcntl (fd, F_SETFL, O_NONBLOCK);
    setsockopt (fd, SOL_SOCKET, SO_SNDBUF, &buffersize, (socklen_t) sizeof (buffersize));
    setsockopt (fd, SOL_SOCKET, SO_RCVBUF, &buffersize, (socklen_t) sizeof (buffersize));
    serverfd = fd;
    Iwritebuffer.set_fd (fd);
    Owritebuffer.set_fd (fd);
    return fd;
}

void fileclient::configure_buffer_initial (string jobdirpath, string appname, string inputfilepath, bool isIcache)
{
    // buffer for Iwrite
    string initial;
    
    if (isIcache)
    {
        initial = "ICwrite ";
        initial.append (jobdirpath);
        initial.append (" ");
        initial.append (appname);
        initial.append (" ");
        initial.append (inputfilepath);
        initial.append ("\n");
    }
    else
    {
        initial = "Iwrite ";
        initial.append (jobdirpath);
        initial.append ("\n");
    }
    
    Iwritebuffer.flush();
    Iwritebuffer.configure_initial (initial);
}

// close should be done exclusively with close_server() function
bool fileclient::write_record (string filename, string data, datatype atype)
{
    if (atype == INTERMEDIATE)
    {
        // generate request string
        string str = filename;
        str.append (" ");
        str.append (data);
        Iwritebuffer.add_record (str);
    }
    else if (atype == OUTPUT)       // atype == OUTPUT
    {
        // generate request string
        if (Owritepath == filename)     // serial write
        {
            Owritebuffer.add_record (data);
        }
        else     // different output
        {
            Owritepath = filename;
            // flush and configure initial string of Owritebuffer
            Owritebuffer.flush();
            string initial = "Owrite ";
            initial.append (filename);
            initial.append ("\n");
            Owritebuffer.configure_initial (initial);
            Owritebuffer.add_record (data);
        }
    }
    else     // atype == RAW
    {
        cout << "[fileclient]Wrong data type for write_record()" << endl;
    }
    
//cout<<"\033[0;33m\trecord sent from client: \033[0m"<<write_buf<<endl;
    return true;
}

void fileclient::close_server()
{
    close (serverfd);
}

void fileclient::wait_write (set<int>* peerids)     // wait until write is done
{
    // flush the write buffer
    Iwritebuffer.flush();
    Owritebuffer.flush();
    // generate request string
    string str;
    
    if (peerids != NULL)   // map task
    {
        str = "MWwrite";
    }
    else // reduce task
    {
        str = "RWwrite";
    }
    
    // send the message to the fileserver
    memset (write_buf, 0, BUF_SIZE);
    strcpy (write_buf, str.c_str());
    nbwrite (serverfd, write_buf);
    
    if (peerids != NULL)     // map task
    {
        // receive the node list to which idata is written
        int readbytes;
        
        while (1)
        {
            readbytes = nbread (serverfd, read_buf);
            
            if (readbytes == 0)     // closed abnormally
            {
                cout << "[fileclient]Connection from file server abnormally closed" << endl;
            }
            else if (readbytes > 0)
            {
                if (read_buf[0] == 0)
                {
                    break;
                }
                else
                {
                    char* token;
                    int id;
                    token = strtok (read_buf, " ");
                    
                    while (token != NULL)
                    {
                        id = atoi (token);
                        peerids->insert (id);
                        // tokenize next peer id
                        token = strtok (NULL, " ");
                    }
                    
                    break;
                }
            }
            else     // didn't receive packets
            {
                continue;
            }
            
            usleep (1000);
        }
        
        // wait close from fileserver
        while (1)
        {
            readbytes = nbread (serverfd, read_buf);
            
            if (readbytes == 0)     // close detected
            {
                break;
            }
            else if (readbytes > 0)
            {
                if (read_buf[0] == 0)
                {
                    break;
                }
            }
            
            usleep (1000);
        }
        
        close (serverfd);
        return;
    }
    else     // reduce task
    {
        // don't need to wait for the peer ids
        close (serverfd);
        return;
    }
}

bool fileclient::read_request (string request, datatype atype)
{
    // generate request string to send file server
    readdatatype = atype;
    string str;
    
    if (atype == RAW)
    {
        str = "Rread ";
        str.append (request);
        memset (write_buf, 0, BUF_SIZE);
        strcpy (write_buf, str.c_str());
        nbwrite (serverfd, write_buf);
        // return true if 1 is returned, false if 0 is returned
        int readbytes;
        
        while (1)
        {
            readbytes = nbread (serverfd, read_buf);
            
            if (readbytes == 0)     // connection closed abnormally
            {
                close_server();
                cout << "[fileclient]Connection abnormally closed" << endl;
                return false;
            }
            else if (readbytes < 0)
            {
                continue;
            }
            else
            {
                if (read_buf[0] == 1)     // intermediate cache hit
                {
                    while (1)
                    {
                        readbytes = nbread (serverfd, read_buf);
                        
                        if (readbytes == 0)     // connection closed abnormally
                        {
                            close_server();
                            cout << "[fileclient]Connection closed abnormally" << endl;
                            return false;
                        }
                        else if (readbytes < 0)
                        {
                            // sleeps for 1 millisecond
                            usleep (1000);
                            continue;
                        }
                        else
                        {
                            if (read_buf[0] == 1)     // distributing intermediate data finished
                            {
                                return true;
                            }
                            else     // key arrived
                            {
                                cout << "[fileclient]Unexpected message during waiting distributing idata" << endl;
                            }
                        }
                    }
                    
                    return true;
                }
                else if (read_buf[0] == 0)       // intermediate cache miss
                {
                    return false;
                }
            }
        }
        
        return true;
    }
    else if (atype == INTERMEDIATE)
    {
        str = "Iread ";
        str.append (request);
        memset (write_buf, 0, BUF_SIZE);
        strcpy (write_buf, str.c_str());
        nbwrite (serverfd, write_buf);
        return false;
    }
    else     // atype <- OUTPUT
    {
        cout << "[fileclient]Unexpected datatype in the read_request()" << endl;
    }
    
    return false;
}

bool fileclient::read_record (string& record)     // read through the socket with blocking way and token each record
{
    if (readdatatype == RAW)
    {
        if (token != NULL)
        {
            token = strtok (ptr, "\n");
        }
        
        if (token == NULL)
        {
            int readbytes;
            
            while (1)
            {
                readbytes = nbread (serverfd, read_buf);
                
                if (readbytes == 0)     // connection closed abnormally
                {
                    cout << "[fileclient]Connection abnormally closed" << endl;
                    close_server();
                    // return empty string
                    record = "";
                    return false;
                }
                else if (readbytes < 0)
                {
                    continue;
                }
                else     // successful read
                {
                    //cout<<"\tread stream at client: "<<read_buf<<endl;
                    //cout<<"\tread stream bytes: "<<readbytes<<endl;
                    if (read_buf[0] == -1)     // the read stream is finished(Eread)
                    {
                        //cout<<"\t\tEnd of read stream at client"<<endl;
                        record = "";
                        return false;
                    }
                    else
                    {
                        token = strtok (read_buf, "\n");
                        
                        if (token == NULL)
                        {
                            continue;
                        }
                        else
                        {
                            ptr = read_buf + strlen (token) + 1;
                            record = token;
                            //cout<<"read stream at client side: "<<record<<endl;
                            return true;
                        }
                    }
                    
                    //cout<<"\033[0;32m\trecord received in client: \033[0m"<<*record<<endl;
                    //cout<<"\033[0;32m\treadbytes: \033[0m"<<readbytes<<endl;
                }
            }
            
            return false;
        }
        else
        {
            record = token;
            ptr = token + strlen (token) + 1;
            //cout<<"read stream at client side: "<<record<<endl;
            return true;
        }
    }
    else     // INTERMEDIATE
    {
        if (token != NULL)
        {
            token = strtok (ptr, "\n");
        }
        
        if (token == NULL)
        {
            int readbytes;
            
            while (1)
            {
                readbytes = nbread (serverfd, read_buf);
                
                if (readbytes == 0)     // connection closed abnormally
                {
                    cout << "[fileclient]Connection abnormally closed" << endl;
                    close_server();
                    // return empty string
                    record = "";
                    return false;
                }
                else if (readbytes < 0)
                {
                    continue;
                    usleep (1000);
                }
                else     // successful read
                {
                    if (read_buf[0] == 0)     // end of key
                    {
                        record = "";
                        return false;
                    }
                    else
                    {
                        token = strtok (read_buf, "\n");
                        
                        if (token == NULL)
                        {
                            continue;
                        }
                        else
                        {
                            ptr = read_buf + strlen (token) + 1;
                            record = token;
                            return true;
                        }
                    }
                }
            }
        }
        else
        {
            record = token;
            ptr = token + strlen (token) + 1;
            return true;
        }
    }
}