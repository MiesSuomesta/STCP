
# --- Paxsudos IT Build metadata ---
BUILD_DATE_UTC := $(shell date -u +"%Y-%m-%d %H:%M:%S UTC")
GIT_SHA := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")

# Injektoi kernelikäännökseen näkyväksi


obj-m += stcp_rust.o

#		stcp_rust_got_debug.o
#		stcp_rust_func_debug.o

stcp_rust-y := \
                debug.o \
                state.o \
                init.o \
                stcp_proto_ops_helpers.o \
                stcp_protocol.o \
                stcp_proto_ops.o \
                ffi_random.o \
                lifespan.o \
                proto_operations.o \
                rust_alloc.o \
                rust_log_with_printk.o \
                stcp_kernel_crypto.o \
                stcp_tcp_recv.o \
                stcp_tcp_send.o \
		handshake_worker.o \
                stcp_misc.o \
                the_stcp_struct.o

stcp_rust-y += the_stcp_kernel_module.o

ccflags-y += -I$(src)/include
ccflags-y += -I$(src)
ccflags-y += -DSTCP_BUILD_DATE="\"$(BUILD_DATE_UTC)\"" -DSTCP_GIT_SHA="\"$(GIT_SHA)\""

