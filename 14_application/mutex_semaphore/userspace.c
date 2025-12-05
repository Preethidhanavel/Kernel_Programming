// park_test.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#define DEV_PATH "/dev/park_dev"
#define PARK  _IOR('p', 1, int)
#define LEAVE _IOW('p', 2, int)

int main(void)
{
    int fd = open(DEV_PATH, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open %s: %s\n", DEV_PATH, strerror(errno));
        return 1;
    }

    while (1) {
        printf("\nCommands:\n 1) park (allocate slot)\n 2) leave (free slot)\n 3) quit\nChoose: ");
        int cmd;
        if (scanf("%d", &cmd) != 1) {
            while (getchar() != '\n'); /* flush */
            continue;
        }

        if (cmd == 1) {
            int slot = -1;
            int ret = ioctl(fd, PARK, &slot);
            if (ret < 0) {
                printf("PARK ioctl failed: %s\n", strerror(errno));
            } else {
                /* your driver returns slot in user copy and also returns slot as ret */
                printf("PARK succeeded: slot=%d (ioctl returned %d)\n", slot, ret);
            }
        } else if (cmd == 2) {
            printf("Enter slot number to free: ");
            int slot;
            if (scanf("%d", &slot) != 1) {
                while (getchar() != '\n');
                continue;
            }
            int ret = ioctl(fd, LEAVE, &slot);
            if (ret < 0) {
                printf("LEAVE ioctl failed: %s\n", strerror(errno));
            } else {
                printf("LEAVE succeeded for slot=%d\n", slot);
            }
        } else if (cmd == 3) {
            break;
        } else {
            printf("unknown command\n");
        }
    }

    close(fd);
    return 0;
}

