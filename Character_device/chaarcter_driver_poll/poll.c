#include <stdio.h>      // For printf, perror
#include <fcntl.h>      // For open() and file control options
#include <poll.h>       // For poll() system call
#include <unistd.h>     // For read(), close()

int main() 
{
    // Open the device file in read-only mode
    int fd = open("/dev/polldev", O_RDONLY);
    if (fd < 0) {
        perror("open");   // Print error if device can't be opened
        return 1;
    }

    // Setup poll structure: watch 'fd' for data availability (POLLIN)
    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    printf("Waiting for data (poll)...\n");

    // Call poll(): wait for up to 10 seconds (10000 ms)
    // Returns >0 if event occurred, 0 if timeout, <0 on error
    int ret = poll(&pfd, 1, 10000); 

    if (ret == 0) {
        // No data arrived within timeout
        printf("Timeout!\n");
    } 
    else if (pfd.revents & POLLIN) {
        // Data is ready to be read
        char buf[256];
        int n = read(fd, buf, sizeof(buf));  // Read data from device
        buf[n] = '\0';                       // Null-terminate string
        printf("Received: %s\n", buf);       // Print received data
    }

    // Close the device file
    close(fd);
    return 0;
}
