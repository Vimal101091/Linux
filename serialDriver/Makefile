# Compiler and flags
CC := gcc
KERNEL_BUILD := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Kernel Module
obj-m := myserialdev.o

# Targets
all: kernel user

kernel:
	@echo "Building kernel module..."
	make -C $(KERNEL_BUILD) M=$(PWD) modules

user:
	@echo "Building user-space application..."
	$(CC) -o userapp userapp.c

clean:
	@echo "Cleaning up..."
	make -C $(KERNEL_BUILD) M=$(PWD) clean
	rm -f userapp
