#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
struct k_spinlock { int dummy; };
typedef int k_spinlock_key_t;
static pthread_mutex_t test_mutex = PTHREAD_MUTEX_INITIALIZER;
static inline int k_spin_lock(struct k_spinlock *p) {(void)p; pthread_mutex_lock(&test_mutex);return 0;}
static inline void k_spin_unlock(struct k_spinlock *p,int key) {(void)p;(void)key;pthread_mutex_unlock(&test_mutex);}
