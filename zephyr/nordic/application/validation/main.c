#include <stdio.h>
#include <stdlib.h>
#include "echo_benchmark.h"
int main(int argc,char**argv) {
 if(argc!=3)return 2;
 struct bench_config cfg;bench_config_defaults(&cfg);
 snprintf(cfg.port,sizeof cfg.port,"%s",argv[1]);cfg.total_bytes=(uint32_t)strtoul(argv[2],NULL,10);
 int rc=bench_run_all(&cfg);if(rc)fprintf(stderr,"rc=%d\n",rc);return rc?1:0;
}
