#ifndef HOST_KERNEL_H
#define HOST_KERNEL_H
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define __packed __attribute__((packed))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))
#define ARG_UNUSED(x) (void)(x)
#define K_FOREVER -1
#define K_NO_WAIT 0
#define K_PRIO_PREEMPT(x) (x)
#define K_SECONDS(x) ((x)*1000)
#define K_THREAD_STACK_DEFINE(name,size) unsigned char name[size]
#define K_THREAD_STACK_SIZEOF(name) sizeof(name)
#define k_malloc malloc
#define k_free free
static inline int64_t k_uptime_get(void) {struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (int64_t)t.tv_sec*1000+t.tv_nsec/1000000;}
static inline uint64_t k_uptime_ticks(void) {struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (uint64_t)t.tv_sec*1000000+t.tv_nsec/1000;}
static inline uint64_t k_ticks_to_us_floor64(uint64_t x) {return x;}
static inline void k_sleep(int ms) {usleep(ms*1000);}
struct k_sem {sem_t value;};
static inline int k_sem_init(struct k_sem *s,int value,int limit) {(void)limit;return sem_init(&s->value,0,value);}
static inline int k_sem_take(struct k_sem *s,int wait) {(void)wait;return sem_wait(&s->value);}
static inline void k_sem_give(struct k_sem *s) {sem_post(&s->value);}
struct k_thread {pthread_t thread;void (*entry)(void*,void*,void*);void *a,*b,*c;};
static void *host_entry(void *v) {struct k_thread*t=v;t->entry(t->a,t->b,t->c);return NULL;}
static inline struct k_thread *k_thread_create(struct k_thread*t,void*stack,size_t size,void(*entry)(void*,void*,void*),void*a,void*b,void*c,int priority,int flags,int delay) {(void)stack;(void)size;(void)priority;(void)flags;(void)delay;t->entry=entry;t->a=a;t->b=b;t->c=c;pthread_create(&t->thread,NULL,host_entry,t);return t;}
static inline int k_thread_join(struct k_thread*t,int timeout) {(void)timeout;return pthread_join(t->thread,NULL);}
#define CONFIG_BENCH_MAX_CHUNK_SIZE 65536
#define CONFIG_BENCH_FULL_RX_BUFFER_SIZE 4096
#define CONFIG_BENCH_SENDER_STACK_SIZE 4096
#define CONFIG_ECHO_SERVER_HOST "127.0.0.1"
#define CONFIG_ECHO_SERVER_PORT "19000"
#define CONFIG_BENCH_CHUNK_SIZE 4096
#define CONFIG_BENCH_TOTAL_BYTES 1048576
#define CONFIG_ECHO_SOCKET_TIMEOUT_MS 5000
#define CONFIG_BENCH_REPORT_INTERVAL_MS 0
#define CONFIG_BENCH_PAUSE_SECONDS 0
typedef int atomic_t;
static inline int atomic_cas(atomic_t *v,int a,int b) {if(*v!=a)return 0;*v=b;return 1;}
static inline void atomic_clear(atomic_t *v) {*v=0;}
struct k_mutex {pthread_mutex_t m;};
static inline int k_mutex_init(struct k_mutex*m) {return pthread_mutex_init(&m->m,NULL);}
static inline int k_mutex_lock(struct k_mutex*m,int wait) {(void)wait;return pthread_mutex_lock(&m->m);}
static inline int k_mutex_unlock(struct k_mutex*m) {return pthread_mutex_unlock(&m->m);}
static inline void k_thread_name_set(struct k_thread*t,const char*n) {(void)t;(void)n;}
#define CONFIG_BENCH_WORKER_STACK_SIZE 4096
#define CONFIG_BENCH_WORKER_PRIORITY 5

#endif
