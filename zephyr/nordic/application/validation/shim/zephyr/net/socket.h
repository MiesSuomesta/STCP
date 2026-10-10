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
#define zsock_recv recv
#define zsock_socket socket
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
