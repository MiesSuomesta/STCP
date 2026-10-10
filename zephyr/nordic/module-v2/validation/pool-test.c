#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <stcp/stcp_buffer_pool.h>
static void *worker(void *arg) {
    (void)arg;
    for (int i=0;i<10000;i++) {
        unsigned char *p=stcp_buffer_alloc(1092);
        assert(p);assert((uintptr_t)p%16==0);
        memset(p,0xa5,1092);assert(stcp_buffer_free(p));
    }
    return NULL;
}
int main(void) {
    void *p[32];
    for(int i=0;i<32;i++){p[i]=stcp_buffer_alloc(1092);assert(p[i]);for(int j=0;j<i;j++)assert(p[i]!=p[j]);}
    assert(!stcp_buffer_alloc(1092));
    void *a=stcp_buffer_alloc(8192),*b=stcp_buffer_alloc(4096);
    assert(a&&b&&!stcp_buffer_alloc(1153)&&!stcp_buffer_alloc(8193));
    assert(stcp_buffer_free(a)&&stcp_buffer_free(b));
    for(int i=0;i<32;i++){memset(p[i],0xff,1152);assert(stcp_buffer_free(p[i]));}
    unsigned char *x=stcp_buffer_alloc(1081);assert(x);for(int i=0;i<1152;i++)assert(x[i]==0);assert(stcp_buffer_free(x));
    int foreign=0;assert(!stcp_buffer_free(&foreign));assert(stcp_buffer_free(NULL));
    pthread_t t[8];for(int i=0;i<8;i++)assert(!pthread_create(&t[i],NULL,worker,NULL));
    for(int i=0;i<8;i++)assert(!pthread_join(t[i],NULL));
    stcp_buffer_pool_report();return 0;
}
