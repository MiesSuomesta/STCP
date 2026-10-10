#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "echo_benchmark.h"
int main(int argc,char **argv) {
 if(argc!=2)return 2;
 signal(SIGPIPE,SIG_IGN);
 struct bench_config cfg;bench_config_defaults(&cfg);
 snprintf(cfg.port,sizeof(cfg.port),"%s",argv[1]);cfg.total_bytes=4096;cfg.timeout_ms=1000;
 int rc=bench_run_matrix(&cfg);return rc<0?1:0;
}
