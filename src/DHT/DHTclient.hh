//
// @brief Blocking client for distributed hash table 
//  
// :TODO: UDP Sockets may be more suitable ?
// :TODO: Which hash function use ?
//
// Preprocessor {{{
#ifndef __DHTCLIENT_HH__
#define __DHTCLIENT_HH__
// 
#include <string.h>
#include <utils.hh>  //! Where the hash function is
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <stdint.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>

#ifndef MRR_IP_LENGTH
#define MRR_IP_LENGTH 64 
#endif
// }}}
//
class DHTclient {
 public:
  DHTclient (const char * ip, int port) {
   strncpy (this->ip, ip, MRR_IP_LENGTH);
   this->port = port;
  }

  virtual ~DHTclient ( close (); );
  
  int lookup (const char * key); 
  bool bind ();
  bool close ();
 
 protected:
  bool server_request (int key);
  int server_receive ();

 protected:
  char ip [MRR_IP_LENGTH];
  int port;
  int server_fd, client_fd;
  struct sockaddr_in server_addr; 
  struct sockaddr_in client_addr; 
};

#endif