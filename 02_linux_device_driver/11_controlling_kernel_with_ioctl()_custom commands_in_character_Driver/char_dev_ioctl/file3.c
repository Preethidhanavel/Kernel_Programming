#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

#define DEVICE "/dev/ioctldev"       // Path to the character device
#define MAJOR_NUM 240                // Major number used in IOCTL commands
#define IOCTL_RESET_BUFFER _IO(MAJOR_NUM, 0)  // IOCTL command to reset buffer
#define IOCTL_GET_COUNT    _IOR(MAJOR_NUM, 1, int) // IOCTL command to get write count

int main() 
{
    // Open the device for reading and writing
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");  // Print error if device cannot be opened
        return 1;
    }

    // Write a message to the device
    char msg[] = "Hello ioctl!";
    write(fd, msg, strlen(msg));

    // Get the current write count from the driver using ioctl
    int count;
    ioctl(fd, IOCTL_GET_COUNT, &count); 
    printf("Write count = %d\n", count); // Print write count

    // Reset the device buffer using ioctl
    ioctl(fd, IOCTL_RESET_BUFFER);
    printf("Buffer reset.\n"); // Confirm buffer reset

    // Close the device
    close(fd);
    return 0;
}
