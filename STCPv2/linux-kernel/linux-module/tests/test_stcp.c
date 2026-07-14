#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#ifndef AF_STCP
#define AF_STCP 45
#endif
int main(void){int fd=socket(AF_STCP,SOCK_STREAM,253);if(fd<0){perror("socket");return 1;}puts("AF_STCP socket created");close(fd);return 0;}
