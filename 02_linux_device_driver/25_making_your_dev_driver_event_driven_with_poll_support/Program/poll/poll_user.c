#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

int main() {
    int fd = open("/dev/gpiobtn", O_RDONLY);
    if (fd < 0) return 1;

    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    printf("Waiting for button press...\n");
    while (1) {
        int ret = poll(&pfd, 1, -1);  // Infinite wait
        if (ret > 0 && (pfd.revents & POLLIN)) {
            char buf[64];
            read(fd, buf, sizeof(buf));
            printf(">> %s", buf);
        }
    }

    close(fd);
    return 0;
}
