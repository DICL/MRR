#ifndef _DEFINITIONS_
#define _DEFINITIONS_

#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

#define MR_PATH "/home/youngmoon01/mr_storage/"
#define HDMR_PATH "/user/youngmoon01/mr_storage/"
#define LIB_PATH "/home/youngmoon01/MRR/src/"
#define BUF_SIZE (4*1024) // 4 KB sized buffer
//#define BUF_SIZE (512*1024) // 512 KB sized buffer
#define BUF_CUT 32
#define TASK_SLOT 4
#define HDFS_PATH "/home/youngmoon01/hadoop-2.2.0/include/"
#define JAVA_LIB "/home/youngmoon01/jdk1.7.0_17/jre/lib/amd64/server/"
#define HDFS_LIB "/home/youngmoon01/hadoop-2.2.0/lib/native/"
#define HADOOP_FLAG "-lhdfs"
#define JAVA_FLAG "-ljvm"

#define DHT_PATH "/scratch/youngmoon01/mr_storage/"

#define RAVENLEADER "192.168.1.201"
#define ADDRESSPREFIX "192.168.1."
#define HASHLENGTH 6

#define BACKLOG 16384

enum mr_role
{
	JOB,
	MAP,
	REDUCE
};

enum task_status
{
	WAITING,
	RUNNING,
	COMPLETED
};

enum job_stage
{
	INITIAL_STAGE,
	MAP_STAGE,
	REDUCE_STAGE,
	COMPLETED_STAGE // not used but reserved for future use
};

enum filetype // file(input and output) type for the task side in the dht mode
{
	LOCAL,
	DISTANT,
	NOTOPENED
};

enum file_client_role 
{
	READ,
	WRITE,
	UNDEFINED
};

// non-blocking write
int nbwrite(int fd, char* buf, char* contents) // when the content should be specified
{
	int written_bytes;
	memset(buf, 0, BUF_SIZE);
	strcpy(buf, contents);
	while((written_bytes = write(fd, buf, BUF_CUT*(strlen(buf)/BUF_CUT+1))) < 0)
	{
		if(errno == EAGAIN)
		{
			cout<<"\twrite function failed due to EAGAIN, retrying..."<<endl;
		}
		else if(errno == EBADF)
		{
			cout<<"\twrite function failed due to EBADF, retrying..."<<endl;
		}
		else if(errno == EFAULT)
		{
			cout<<"\twrite function failed due to EFAULT, retrying..."<<endl;
		}
		else if(errno == EFBIG)
		{
			cout<<"\twrite function failed due to EFBIG, retrying..."<<endl;
		}
		else if(errno == EINTR)
		{
			cout<<"\twrite function failed due to EINTR, retrying..."<<endl;
		}
		else if(errno == EINVAL)
		{
			cout<<"\twrite function failed due to EINVAL, retrying..."<<endl;
		}
		else if(errno == EIO)
		{
			cout<<"\twrite function failed due to EIO, retrying..."<<endl;
		}
		else if(errno == ENOSPC)
		{
			cout<<"\twrite function failed due to ENOSPC, retrying..."<<endl;
		}
		else if(errno == EPIPE)
		{
			cout<<"\twrite function failed due to EPIPE, retrying..."<<endl;
		}
		else
		{
			cout<<"\twrite function failed due to unknown reason(debug needed)..."<<endl;
			return -1;
		}

		// sleep 1 milli seconds to prevent busy waiting
		usleep(1000);
cout<<"does this delay"<<endl;
usleep(100000);
	}
	if(written_bytes != BUF_CUT*((int)strlen(buf)/BUF_CUT+1))
	{
		int progress = written_bytes;
		int remain = BUF_CUT*(strlen(buf)/BUF_CUT+1) - written_bytes;
		while(remain > 0)
		{
			written_bytes = write(fd, buf+progress, remain);

			if(written_bytes > 0)
			{
				progress += written_bytes;
				remain -= written_bytes;
			}

			// sleep 1 milli seconds to prevent busy waiting
			usleep(1000);
cout<<"does this delay"<<endl;
usleep(100000);
		}
	}
	return written_bytes;
}

// non-blocking write
int nbwrite(int fd, char* buf) // when the content is already on the buffer
{
	int written_bytes;
	while((written_bytes = write(fd, buf, BUF_CUT*(strlen(buf)/BUF_CUT+1))) < 0)
	{
		if(errno == EAGAIN)
		{
			cout<<"\twrite function failed due to EAGAIN, retrying..."<<endl;
		}
		else if(errno == EBADF)
		{
			cout<<"\twrite function failed due to EBADF, retrying..."<<endl;
		}
		else if(errno == EFAULT)
		{
			cout<<"\twrite function failed due to EFAULT, retrying..."<<endl;
		}
		else if(errno == EFBIG)
		{
			cout<<"\twrite function failed due to EFBIG, retrying..."<<endl;
		}
		else if(errno == EINTR)
		{
			cout<<"\twrite function failed due to EINTR, retrying..."<<endl;
		}
		else if(errno == EINVAL)
		{
			cout<<"\twrite function failed due to EINVAL, retrying..."<<endl;
		}
		else if(errno == EIO)
		{
			cout<<"\twrite function failed due to EIO, retrying..."<<endl;
		}
		else if(errno == ENOSPC)
		{
			cout<<"\twrite function failed due to ENOSPC, retrying..."<<endl;
		}
		else if(errno == EPIPE)
		{
			cout<<"\twrite function failed due to EPIPE, retrying..."<<endl;
		}
		else
		{
			cout<<"\twrite function failed due to unknown reason(debug needed)..."<<endl;
			return -1;
		}

		// sleep 1 milli seconds to prevent busy waiting
		usleep(1000);
	}
	if(written_bytes != BUF_CUT*((int)strlen(buf)/BUF_CUT+1))
	{
		int progress = written_bytes;
		int remain = BUF_CUT*(strlen(buf)/BUF_CUT+1) - written_bytes;
		while(remain > 0)
		{
			written_bytes = write(fd, buf+progress, remain);

			if(written_bytes > 0)
			{
				progress += written_bytes;
				remain -= written_bytes;
			}

			// sleep 1 milli seconds to prevent busy waiting
			usleep(1000);
		}
	}
	return written_bytes;
}

// non-blocking read
int nbread(int fd, char* buf)
{
	int total_readbytes = 0;
	int readbytes = 0;
	memset(buf, 0, BUF_SIZE);

	readbytes = read(fd, buf, BUF_CUT);
	if(readbytes == 0)
	{
		return readbytes;
	}
	else if(readbytes < 0)
	{
		if(errno != EAGAIN)
		{
			if(errno == EBADF)
			{
				cout<<"\t\033[0;32mread function failed due to EBADF error, debug needed\033[0m"<<endl;
			}
			else if(errno == EFAULT)
			{
				cout<<"\t\033[0;32mread function failed due to EFAULT error, debug needed\033[0m"<<endl;
			}
			else if(errno == EINTR)
			{
				cout<<"\t\033[0;32mread function failed due to EINTR error, debug needed\033[0m"<<endl;
			}
			else if(errno == EINVAL)
			{
				cout<<"\t\033[0;32mread function failed due to EINVAL error, debug needed\033[0m"<<endl;
			}
			else if(errno == EIO)
			{
				cout<<"\t\033[0;32mread function failed due to EIO error, debug needed\033[0m"<<endl;
			}
			else if(errno == EISDIR)
			{
				cout<<"\t\033[0;32mread function failed due to EISDIR error, debug needed\033[0m"<<endl;
			}
		}

		return readbytes;
	}
	else
	{
		total_readbytes += readbytes;
		if(buf[total_readbytes-1] == 0 && total_readbytes%BUF_CUT == 0)
		{
			return total_readbytes;
		}
		else
		{
			while(1)
			{
				readbytes = read(fd, buf+total_readbytes, BUF_CUT-(total_readbytes%BUF_CUT));
				if(readbytes == 0)
				{
					cout<<"\t\033[0;32mthe fd was closed during reading the buffer: debug the nbread() function.\033[0m"<<endl;
					cout<<"\t\033[0;32m"<<buf<<"\033[0m"<<endl;
					cout<<"\t\033[0;32m"<<"total_readbytes: "<<total_readbytes<<"\033[0m"<<endl;
					cout<<"\t\033[0;32m"<<"last_character: "<<buf[total_readbytes-1]<<"\033[0m"<<endl;
					return 0;
				}
				else if(readbytes < 0)
				{
					// sleep 1 milli seconds to prevent busy waiting
					usleep(1000);
					continue;
				}
				else
				{
					total_readbytes += readbytes;

					if(buf[total_readbytes-1] != 0 || total_readbytes%BUF_CUT != 0)
						continue;
					else
						return total_readbytes;
				}
			}
		}
		
	}
	return total_readbytes + readbytes;
}

#endif
