#include <zephyr/kernel.h>
#include <stdint.h>
#include <stdlib.h>

struct RustSpawnContext {
    struct k_thread *thread_ptr;
    k_thread_stack_t *stack_ptr;
    size_t stack_size;
    void (*entry)(void *);
    void *arg;
};

// Sisäinen Zephyr-säikeen entry
static void rust_thread_entry(void *p1, void *p2, void *p3) {
    struct RustSpawnContext *ctx = (struct RustSpawnContext *)p1;

    // Suorita varsinainen Rustin logiikka
    ctx->entry(ctx->arg);

    // Vapauta muistialueet (tulevat Rustista Box::into_raw)
    free(ctx->thread_ptr);
    free(ctx->stack_ptr);
    free(ctx);
}

void spawn_with_arg(struct RustSpawnContext *ctx) {
    k_thread_create(
        ctx->thread_ptr,
        ctx->stack_ptr,
        ctx->stack_size,
        rust_thread_entry,
        ctx, NULL, NULL,
        5, 0, K_NO_WAIT
    );
}

