#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd = open("/dev/polldev", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct pollfd pfd = { .fd = fd, .events = POLLIN | POLLOUT };

    printf("Waiting for events (poll)...\n");
    int ret = poll(&pfd, 1, 10000); // 10 second timeout

    if (ret == 0) {
        printf("Timeout!\n");
    } else {
        if (pfd.revents & POLLIN) {
            char buf[256];
            int n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                printf("Received: %s\n", buf);
            } else {
                printf("Read error or no data.\n");
            }
        }

        if (pfd.revents & POLLOUT) {
            const char *msg = "Hello from userspace!";
            int n = write(fd, msg, strlen(msg));
            if (n > 0) {
                printf("Wrote: %s\n", msg);
            } else {
                printf("Write failed.\n");
            }
        }

        if (pfd.revents & POLLERR)
            printf("Error on FD\n");

        if (pfd.revents & POLLHUP)
            printf("Device closed (hang up)\n");
    }

    close(fd);
    return 0;
}

