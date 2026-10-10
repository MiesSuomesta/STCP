/* Test only: exercise native STCP server branches over POSIX TCP.
 * This does not test the kernel module, handshake, crypto or reliability.
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
static inline int simulated_socket(int family,int type,int protocol) {
    if (family==45) {
        if (type!=SOCK_STREAM || (protocol!=253 && protocol!=254)) {
            errno=EPROTONOSUPPORT; return -1;
        }
        return socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    }
    return socket(family,type,protocol);
}
#define socket simulated_socket
