#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>   // for open()
#include <unistd.h>  // for read(), write(), close()
#include <string.h>  // for strlen()

#define DEVICE "/dev/mychardev"   // Path to the device file

int main()
{
    int fd;                        // File descriptor for the device
    char buffer[100];              // Buffer to store data read from device
    ssize_t bytes_read, bytes_written;

    // Open the device file in Read/Write mode
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) 
    {  // If open fails
        perror("Failed to open device");
        return EXIT_FAILURE;
    }

    // Write a message to the device
    const char *msg = "Testing from app";
    bytes_written = write(fd, msg, strlen(msg));
    if (bytes_written < 0) 
    {  // If write fails
        perror("Failed to write to device");
        close(fd);
        return EXIT_FAILURE;
    }

    // Read message back from the device
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) 
    {  // If read fails
        perror("Failed to read from device");
        close(fd);
        return EXIT_FAILURE;
    }

    // Add null terminator to make it a valid C string
    buffer[bytes_read] = '\0';

    // Print the message received from the driver
    printf("Read from device: %s", buffer);

    // Close the device file
    close(fd);

    return EXIT_SUCCESS;
}
