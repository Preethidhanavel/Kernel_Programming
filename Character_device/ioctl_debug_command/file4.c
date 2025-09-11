#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

#define DEVICE "/dev/ioctldev"           // Path to the character device
#define MAJOR_NUM 240                     // Major number used in IOCTL commands
#define IOCTL_RESET_BUFFER _IO(MAJOR_NUM, 0)   // IOCTL command to reset buffer
#define IOCTL_GET_COUNT    _IOR(MAJOR_NUM, 1, int) // IOCTL command to get write count
#define IOCTL_SET_DEBUG _IOW(MAJOR_NUM, 2, int)    // IOCTL command to enable/disable debug

int main() 
{
    // Open the device file for reading and writing
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) { 
        perror("open");  // Print error if device cannot be opened
        return 1; 
    }

    // Enable debug mode in the kernel driver
    int debug = 1;
    ioctl(fd, IOCTL_SET_DEBUG, &debug);  // Send debug flag to driver

    // Write a message to the device
    char msg[] = "Testing debug mode!";
    write(fd, msg, strlen(msg));         // This will trigger extra printk if debug is enabled

    // Close the device
    close(fd);
    return 0;
}


