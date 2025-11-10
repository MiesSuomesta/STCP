#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#define IPPROTO_STCP 253
void* thr(void* arg){
  arg = arg;
  for(int i=0;i<20000;i++){
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_STCP);
    if(fd>=0) close(fd);
  }
  return NULL;
}
int main(){
  const int N=8;
  pthread_t t[N];
  for(int i=0;i<N;i++) pthread_create(&t[i],0,thr,0);
  for(int i=0;i<N;i++) pthread_join(t[i],0);
  puts("ok");
  return 0;
}
