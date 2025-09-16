#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

int main() 
{
    int fd = open("/dev/polldev", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    printf("Waiting for data (poll)...\n");
    int ret = poll(&pfd, 1, 10000); // 10 second timeout

    if (ret == 0) {
        printf("Timeout!\n");
    } 
    else if (pfd.revents & POLLIN) {
        char buf[256];
        int n = read(fd, buf, sizeof(buf));
        buf[n] = '\0';
        printf("Received: %s\n", buf);
    }

    close(fd);
    return 0;
}
