#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>

#define DEVICE "/dev/ioctldev"        // Path to the character device
#define MAJOR_NUM 240                  // Major number used for IOCTL commands

// IOCTL command definitions
#define IOCTL_RESET_BUFFER _IO(MAJOR_NUM, 0)   // Reset device buffer
#define IOCTL_GET_COUNT    _IOR(MAJOR_NUM, 1, int) // Get number of writes
#define IOCTL_SET_DEBUG    _IOW(MAJOR_NUM, 2, int) // Enable/disable debug mode

int main() 
{
    // Open the device for read/write
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    int debug;

    // -------- Enable debug mode --------
    debug = 1;
    if (ioctl(fd, IOCTL_SET_DEBUG, &debug) < 0) {  // Send IOCTL to enable debug
        perror("ioctl SET_DEBUG ON");
        close(fd);
        return 1;
    }
    printf("Debug mode enabled\n");

    // Write data to device with debug enabled
    char msg1[] = "Hello with debug ON!";
    write(fd, msg1, strlen(msg1));  // Kernel prints extra debug info
    printf("Wrote message with debug enabled\n");

    // -------- Disable debug mode --------
    debug = 0;
    if (ioctl(fd, IOCTL_SET_DEBUG, &debug) < 0) {  // Send IOCTL to disable debug
        perror("ioctl SET_DEBUG OFF");
        close(fd);
        return 1;
    }
    printf("Debug mode disabled\n");

    // Write data to device with debug disabled
    char msg2[] = "Hello with debug OFF!";
    write(fd, msg2, strlen(msg2));  // No extra debug info from kernel
    printf("Wrote message with debug disabled\n");

    // Close the device
    close(fd);
    return 0;
}
