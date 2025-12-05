#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

int main() 
{
    int fd = open("/dev/gpiobtn", O_RDONLY | O_NONBLOCK);
    if (fd < 0) return 1;

    int epfd = epoll_create1(0);
    struct epoll_event ev = { .events = EPOLLIN, .data.fd = fd };
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);

    char buf[64];

    printf("Waiting for button events using epoll...\n");
    while (1) {
        struct epoll_event events[5];
        int n = epoll_wait(epfd, events, 5, -1);

        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == fd) {
                int r = read(fd, buf, sizeof(buf));
                buf[r] = '\0';
                printf(">> %s", buf);
            }
        }
    }

 
    close(fd);
    return 0;
}
