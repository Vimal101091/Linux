#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define DEVICE_PATH "/dev/myserialdev"

int main(int argc, char *argv[])
{
    int fd;
    ssize_t bytes_written;

    // Check if the user provided a string as input
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string to write>\n", argv[0]);
        return 1;
    }

    // Open the device
    fd = open(DEVICE_PATH, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return 1;
    }

    // Write the string to the device
    bytes_written = write(fd, argv[1], strlen(argv[1]));
    if (bytes_written < 0) {
        perror("Failed to write to the device");
        close(fd);
        return 1;
    }

    printf("Successfully wrote '%s' (%zd bytes) to %s\n", argv[1], bytes_written, DEVICE_PATH);

    // Close the device
    close(fd);

    return 0;
}
