#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/ioctl.h>

// Define ioctl command (must match the driver definition)
#define POLLDRV_IOC_MAGIC 'p'
#define POLLDRV_IOC_SET_READ_READY _IOW(POLLDRV_IOC_MAGIC, 1, int)

int main() 
{
    // Open the device file for read and write
    int fd = open("/dev/polldev", O_RDWR);
    if (fd < 0) {
        perror("open");  // Print error if device cannot be opened
        return 1;
    }

    // Tell the driver to enable "read readiness" using ioctl
    int flag = 1;
    if (ioctl(fd, POLLDRV_IOC_SET_READ_READY, &flag) < 0) {
        perror("ioctl"); // Print error if ioctl fails
        close(fd);
        return 1;
    }

    // Prepare pollfd structure to monitor events
    struct pollfd pfd = { .fd = fd, .events = POLLIN | POLLOUT };

    printf("Waiting for events...\n");
    // Wait for events (max 10 seconds timeout)
    int ret = poll(&pfd, 1, 10000);

    if (ret == 0) {
        // No event within 10 seconds
        printf("Timeout!\n");
    } 
    else {
        // Check which events occurred
        if (pfd.revents & POLLIN) {
            printf("Device is ready for reading\n");
        }
        if (pfd.revents & POLLOUT) {
            printf("Device is ready for writing\n");
        }
    }

    // Close device file
    close(fd);
    return 0;
}
