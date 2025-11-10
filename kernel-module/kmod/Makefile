
# --- Paxsudos IT Build metadata ---
BUILD_DATE_UTC := $(shell date -u +"%Y-%m-%d %H:%M:%S UTC")
GIT_SHA := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")

# Injektoi kernelikäännökseen näkyväksi


obj-m += stcp_rust.o

#		stcp_rust_got_debug.o
#		stcp_rust_func_debug.o

stcp_rust-y := \
		the_rust_implementation.o \
		rust_log_with_printk.o \
		the_context.o \
		proto.o \
		operations.o \
		lifecycle.o \
		sock_glue.o \
		helpers.o \
		rust_alloc.o \
		ffi_rand.o \
		state.o

ccflags-y += -I$(src)/include
ccflags-y += -I$(src)
ccflags-y += -DSTCP_BUILD_DATE="\"$(BUILD_DATE_UTC)\"" -DSTCP_GIT_SHA="\"$(GIT_SHA)\""

