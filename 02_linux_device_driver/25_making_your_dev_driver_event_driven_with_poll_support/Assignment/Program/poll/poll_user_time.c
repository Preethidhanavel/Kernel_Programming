#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

int main() 
{
    // Open the device file for reading
    int fd = open("/dev/gpiobtn", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Set up poll structure: watch fd for "readable" events
    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    printf("Waiting for button press (5s timeout)...\n");

    while (1) {
        // poll() waits up to 5000 ms for an event
        int ret = poll(&pfd, 1, 5000);

        if (ret < 0) {
            // Error case
            perror("poll");
            break;
        } 
        else if (ret == 0) {
            // Timeout (no event in 5 seconds)
            printf("No button press within 5 seconds\n");
        } 
        else if (pfd.revents & POLLIN) {
            // Button press event → read data from driver
            char buf[64];
            int n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0'; // Null terminate string
                printf(">> %s", buf);
            }
        }
    }

    // Close the device before exiting
    close(fd);
    return 0;
}
