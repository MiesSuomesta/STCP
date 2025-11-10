#include <linux/kernel.h>
#include <linux/module.h>

#define STCP_INTRINSIC_PANIC(name)                      \
    do {                                                \
        pr_err("stcp: illegal call to %s\n", name);     \
        BUG();                                          \
    } while (0)

void __addtf3(void)      { STCP_INTRINSIC_PANIC("__addtf3"); }
void __divtf3(void)      { STCP_INTRINSIC_PANIC("__divtf3"); }
void __eqtf2(void)       { STCP_INTRINSIC_PANIC("__eqtf2"); }
void __extendhfsf2(void) { STCP_INTRINSIC_PANIC("__extendhfsf2"); }
void __floatsitf(void)   { STCP_INTRINSIC_PANIC("__floatsitf"); }
void __floattitf(void)   { STCP_INTRINSIC_PANIC("__floattitf"); }
void __gttf2(void)       { STCP_INTRINSIC_PANIC("__gttf2"); }
void __letf2(void)       { STCP_INTRINSIC_PANIC("__letf2"); }
void __lttf2(void)       { STCP_INTRINSIC_PANIC("__lttf2"); }
void __multf3(void)      { STCP_INTRINSIC_PANIC("__multf3"); }
void __netf2(void)       { STCP_INTRINSIC_PANIC("__netf2"); }
void __subtf3(void)      { STCP_INTRINSIC_PANIC("__subtf3"); }
void __truncsfhf2(void)  { STCP_INTRINSIC_PANIC("__truncsfhf2"); }
void __udivti3(void)     { STCP_INTRINSIC_PANIC("__udivti3"); }
void __umodti3(void)     { STCP_INTRINSIC_PANIC("__umodti3"); }
void __unordtf2(void)    { STCP_INTRINSIC_PANIC("__unordtf2"); }
void fma(void)           { STCP_INTRINSIC_PANIC("fma"); }

