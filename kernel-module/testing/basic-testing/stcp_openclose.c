#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#define IPPROTO_STCP 253
int main(){
  for(int i=0;i<100000;i++){
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_STCP);
    if(fd<0){perror("socket"); return 1;}
    close(fd);
  }
  puts("ok");
  return 0;
}
