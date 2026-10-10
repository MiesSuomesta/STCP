#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#define zsock_pollfd pollfd
#define zsock_poll poll
#define ZSOCK_POLLIN POLLIN
#define ZSOCK_POLLOUT POLLOUT
#define ZSOCK_POLLERR POLLERR
#define ZSOCK_POLLHUP POLLHUP
#define ZSOCK_POLLNVAL POLLNVAL
#define zsock_send send
#ifdef HOST_READ_SLICE
static inline ssize_t host_recv(int fd,void *data,size_t len,int flags) {
 int type=0; socklen_t size=sizeof(type);
 if(getsockopt(fd,SOL_SOCKET,SO_TYPE,&type,&size)==0 && type==SOCK_STREAM && len>HOST_READ_SLICE)
  len=HOST_READ_SLICE;
 return recv(fd,data,len,flags);
}
#define zsock_recv host_recv
#else
#define zsock_recv recv
#endif
#ifdef HOST_SIMULATE_STCP
/* Socket selection is tested, but STCP encryption/reliability is NOT simulated. */
static inline int host_socket(int family,int type,int protocol) {
 if(family==45) {
  if(type!=SOCK_STREAM || (protocol!=253 && protocol!=254)){errno=EPROTONOSUPPORT;return -1;}
#ifdef HOST_SIMULATE_STCP_STREAM
  return socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
#else
  int fd=socket(AF_INET,protocol==254 ? SOCK_DGRAM : SOCK_STREAM,protocol==254 ? IPPROTO_UDP : IPPROTO_TCP);
  int buffer=4*1024*1024;
  if(fd>=0)setsockopt(fd,SOL_SOCKET,SO_RCVBUF,&buffer,sizeof(buffer));
  return fd;
#endif
 }
 int fd=socket(family,type,protocol);
 int buffer=4*1024*1024;
 if(fd>=0)setsockopt(fd,SOL_SOCKET,SO_RCVBUF,&buffer,sizeof(buffer));
 return fd;
}
#define zsock_socket host_socket
#else
#define zsock_socket socket
#endif
#define zsock_close close
#define zsock_connect connect
#define zsock_fcntl fcntl
#define zsock_getsockopt getsockopt
#define zsock_setsockopt setsockopt
#define zsock_addrinfo addrinfo
#define zsock_getaddrinfo getaddrinfo
#define zsock_freeaddrinfo freeaddrinfo
#define zsock_inet_pton inet_pton
#define zsock_timeval timeval
