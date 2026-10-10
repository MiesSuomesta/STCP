#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "echo_benchmark.h"
int main(int argc,char**argv) {
 if(argc<3 || argc>5)return 2;
 struct bench_config cfg;bench_config_defaults(&cfg);
 snprintf(cfg.port,sizeof cfg.port,"%s",argv[1]);cfg.total_bytes=(uint32_t)strtoul(argv[2],NULL,10);
 if(argc>=4) {
  const char *t=argv[3];
  if(!strcmp(t,"udp"))cfg.transport=BENCH_TRANSPORT_UDP;
  else if(!strcmp(t,"stcp-tcp"))cfg.transport=BENCH_TRANSPORT_STCP;
  else if(!strcmp(t,"stcp-udp"))cfg.transport=BENCH_TRANSPORT_STCP_UDP;
  else if(strcmp(t,"tcp"))return 2;
 }
 if(argc==5)cfg.chunk_size=(uint32_t)strtoul(argv[4],NULL,10);
 cfg.timeout_ms=1000;
 int rc=bench_run_all(&cfg);if(rc)fprintf(stderr,"rc=%d\n",rc);return rc?1:0;
}
