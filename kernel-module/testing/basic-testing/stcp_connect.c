#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#define IPPROTO_STCP 253
int main(){
  int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_STCP);
  struct sockaddr_in sa = { .sin_family=AF_INET, .sin_port=htons(5678) };
  inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);
  int r = connect(fd, (struct sockaddr*)&sa, sizeof(sa));
  printf("connect -> %d (errno OK jos -1)\n", r);
  close(fd);
  return 0;
}
