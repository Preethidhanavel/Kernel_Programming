#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>   // open
#include <unistd.h>  // read, write, close
#include <string.h>  // strlen

#define DEVICE "/dev/mychardev"

int main()
{
    int fd;
    char buffer[100];
    ssize_t bytes_read, bytes_written;

    // Open device file
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    // Write to device
    const char *msg = "Testing from app";
    bytes_written = write(fd, msg, strlen(msg));
    if (bytes_written < 0) {
        perror("Failed to write to device");
        close(fd);
        return EXIT_FAILURE;
    }

    // Read from device
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        perror("Failed to read from device");
        close(fd);
        return EXIT_FAILURE;
    }

    // Null-terminate and print output
    buffer[bytes_read] = '\0';
    printf("Read from device: %s", buffer);

    // Close device
    close(fd);

    return EXIT_SUCCESS;
}

