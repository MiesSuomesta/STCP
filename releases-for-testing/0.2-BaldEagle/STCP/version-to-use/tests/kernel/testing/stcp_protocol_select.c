#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define AF_STCP 45
#define STCP_PROTO_TCP 253
#define STCP_PROTO_UDP 254

int main(void)
{
	int tcp_fd;
	int udp_fd;

	tcp_fd = socket(
		AF_STCP,
		SOCK_STREAM,
		STCP_PROTO_TCP
	);

	if (tcp_fd < 0) {
		perror("STCP/TCP socket");
		return 1;
	}

	udp_fd = socket(
		AF_STCP,
		SOCK_STREAM,
		STCP_PROTO_UDP
	);

	if (udp_fd < 0) {
		perror("STCP/UDP socket");
		close(tcp_fd);
		return 1;
	}

	printf(
		"created STCP/TCP fd=%d and STCP/UDP fd=%d\n",
		tcp_fd,
		udp_fd
	);

	close(udp_fd);
	close(tcp_fd);
	return 0;
}
