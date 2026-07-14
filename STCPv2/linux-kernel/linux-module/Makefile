ifneq ($(KERNELRELEASE),)
obj-m += stcp.o
stcp-y := src/module.o src/proto.o src/socket.o src/rust_shim.o src/debug.o
ccflags-y += -I$(src)/include
# Rust object is linked after building rust/target/stcp-kernel-core.o.
stcp-y += rust_core.o
else
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

.PHONY: all rust module clean
all: rust module

rust:
	$(MAKE) -C rust
	cp rust/target/stcp-kernel-core.o rust_core.o

module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(MAKE) -C rust clean
	rm -f rust_core.o
endif
