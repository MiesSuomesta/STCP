#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <stcp/debug.h>

void stcp_dump_bt(void)
{
    void *lr;
    void *sp;

    __asm volatile ("mov %0, lr" : "=r"(lr));
    __asm volatile ("mov %0, sp" : "=r"(sp));

    printk("---- STCP BACKTRACE ----\n");
    printk("LR=%p SP=%p\n", lr, sp);

    uintptr_t *stack = (uintptr_t *)sp;

    for (int i = 0; i < 16; i++) {
        uintptr_t addr = stack[i];

        if (addr >= 0x00000000 && addr < 0x00100000) {
            printk("#%d %p\n", i, (void *)addr);
        }
    }

    printk("------------------------\n");
}