#include <stdio.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define POLLDRV_IOC_MAGIC 'p'
#define POLLDRV_IOC_SET_READ_READY _IOW(POLLDRV_IOC_MAGIC, 1, int)

int main() 
{
    int fd = open("/dev/polldev", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Enable read readiness via ioctl
    int flag = 1;
    if (ioctl(fd, POLLDRV_IOC_SET_READ_READY, &flag) < 0) {
        perror("ioctl");
        close(fd);
        return 1;
    }

    struct pollfd pfd = { .fd = fd, .events = POLLIN | POLLOUT };

    printf("Waiting for events...\n");
    int ret = poll(&pfd, 1, 10000);

    if (ret == 0) {
        printf("Timeout!\n");
    } 
    else {
        if (pfd.revents & POLLIN) {
            printf("Device is ready for reading\n");
        }
        if (pfd.revents & POLLOUT) {
            printf("Device is ready for writing\n");
        }
    }

    close(fd);
    return 0;
}

