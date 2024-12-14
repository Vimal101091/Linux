#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h> 
#include "ioctl_cmd.h"

#define DEVICE_0 "/dev/mychardev0"
#define DEVICE_1 "/dev/mychardev1"
#define BUFFER_SIZE 1024

void test_device(const char *device_path) {
    int fd;
    char write_buf[] = "Hello from user space!";
    char read_buf[BUFFER_SIZE];
    ssize_t ret;
    off_t offset;
    unsigned int length;          // Declare 'length' at the top
    unsigned char fill_char = 'X'; // Declare 'fill_char' at the top
    void *kernel_address;         // Declare 'kernel_address' at the top

    printf("Testing device: %s\n", device_path);

    // Open the device
    fd = open(device_path, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return;
    }
    printf("Device opened successfully: %s\n", device_path);

    // Write to the device
    ret = write(fd, write_buf, strlen(write_buf));
    if (ret < 0) {
        perror("Failed to write to the device");
        close(fd);
        return;
    }
    printf("Wrote %zd bytes to the device\n", ret);

    // Seek to the beginning of the device
    offset = lseek(fd, 0, SEEK_SET);
    if (offset < 0) {
        perror("Failed to seek in the device");
        close(fd);
        return;
    }
    printf("Seeked to offset %ld in the device\n", offset);

    // Read from the device
    memset(read_buf, 0, sizeof(read_buf));
    ret = read(fd, read_buf, sizeof(read_buf) - 1);
    if (ret < 0) {
        perror("Failed to read from the device");
        close(fd);
        return;
    }
    printf("Read %zd bytes from the device: %s\n", ret, read_buf);
    

     // Ioctl: Get Buffer Length
    if (ioctl(fd, MSG_IOCTL_GET_LENGTH, &length) < 0) {
        perror("Failed to get buffer length");
        printf("Error: %s\n", strerror(errno));
    } else {
        printf("Buffer length: %u bytes\n", length);
    }

    // Ioctl: Fill Buffer
    if (ioctl(fd, MSG_IOCTL_FILL_BUFFER, &fill_char) < 0) {
        perror("Failed to fill the buffer");
        printf("Error: %s\n", strerror(errno));
    } else {
        printf("Filled buffer with character: '%c'\n", fill_char);
    }

    // Ioctl: Get Kernel Buffer Address
    if (ioctl(fd, MSG_GET_ADDRESS, &kernel_address) < 0) {
        perror("Failed to get kernel buffer address");
        printf("Error: %s\n", strerror(errno));
    } else {
        printf("Kernel buffer address: %p\n", kernel_address);
    }

    // Close the device
    if (close(fd) < 0) {
        perror("Failed to close the device");
        return;
    }
    printf("Device closed successfully: %s\n", device_path);
}

int main() {
    printf("Starting test for character devices...\n");

    // Test both devices
    test_device(DEVICE_0);
    test_device(DEVICE_1);

    printf("Testing completed.\n");
    return 0;
}
