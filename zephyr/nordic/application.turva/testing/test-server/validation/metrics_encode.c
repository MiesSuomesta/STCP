#include "../zephyr/ben2_metrics.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
 uint8_t out[112],req[20];
 struct ben2_metrics m={.carrier=2,.tx_bytes=0x0102030405060708ULL,.elapsed_us=0x1020304050607080ULL};
 ben2_metrics_encode(out,&m);
 assert(out[0]=='M' && out[1]=='T' && out[2]=='R' && out[3]=='2');
 assert(out[11]==112 && out[15]==2 && out[16]==1 && out[23]==8);
 assert(out[104]==0x10 && out[111]==0x80);
 ben2_request_encode(req,3,4096,1048576); assert(req[7]==2 && req[11]==3);
 fwrite(out,1,sizeof out,stdout);
}
