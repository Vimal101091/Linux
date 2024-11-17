#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define DEVICE_0 "/dev/mychardev0"
#define DEVICE_1 "/dev/mychardev1"
#define BUFFER_SIZE 1024

void test_device(const char *device_path) {
    int fd;
    char write_buf[] = "Hello from user space!";
    char read_buf[BUFFER_SIZE];
    ssize_t ret;
    off_t offset;

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
