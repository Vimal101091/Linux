# Kernel module details
obj-m := chardev.o
DEVICE_NAME_0 := /dev/mychardev0
DEVICE_NAME_1 := /dev/mychardev1

# User application details
USERAPP := userapp
USERAPP_SRC := userapp.c

# Kernel build directory
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Default target: Build both kernel module and user app
all: build_kernel_module build_userapp

# Build the kernel module
build_kernel_module:
	make -C $(KDIR) M=$(PWD) modules

# Build the user application
build_userapp:
	gcc -o $(USERAPP) $(USERAPP_SRC)

# Clean up build artifacts for kernel module and user app
clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f $(USERAPP)

# Insert kernel module into the kernel
load:
	sudo insmod chardev.ko
	sudo chmod 666 $(DEVICE_NAME_0)
	sudo chmod 666 $(DEVICE_NAME_1)

# Remove kernel module from the kernel
unload:
	sudo rmmod chardev

# Show kernel log messages
dmesg:
	dmesg | tail -n 20

# Run the user application
run-userapp:
	./$(USERAPP)

# Full clean, build, load kernel module, and run user application
test: clean build_kernel_module load build_userapp run-userapp
