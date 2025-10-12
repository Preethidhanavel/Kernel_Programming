#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

int main() {
    // Open the device file in read/write mode
    int fd = open("/dev/polldev", O_RDWR);
    if (fd < 0) {
        perror("open");   // Print error if device not found
        return 1;
    }

    // Setup pollfd structure: monitor both read and write events
    struct pollfd pfd = { .fd = fd, .events = POLLIN | POLLOUT };

    printf("Waiting for events (poll)...\n");

    // Wait for up to 10 seconds for events (read/write availability)
    int ret = poll(&pfd, 1, 10000); // timeout = 10000ms (10s)

    if (ret == 0) {
        // No events happened during timeout
        printf("Timeout!\n");
    } 
    else {
        // If data is available to read
        if (pfd.revents & POLLIN) {
            char buf[256];
            int n = read(fd, buf, sizeof(buf) - 1); // Read from device
            if (n > 0) {
                buf[n] = '\0';   // Null terminate
                printf("Received: %s\n", buf);
            } 
            else {
                printf("Read error or no data.\n");
            }
        }

        // If device is ready to accept writes
        if (pfd.revents & POLLOUT) {
            const char *msg = "Hello from userspace!";
            int n = write(fd, msg, strlen(msg)); // Write to device
            if (n > 0) {
                printf("Wrote: %s\n", msg);
            } 
            else {
                printf("Write failed.\n");
            }
        }

        // If an error occurred on the file descriptor
        if (pfd.revents & POLLERR)
            printf("Error on FD\n");

        // If the device was closed/hung up
        if (pfd.revents & POLLHUP)
            printf("Device closed (hang up)\n");
    }

    // Close the device file
    close(fd);
    return 0;
}
