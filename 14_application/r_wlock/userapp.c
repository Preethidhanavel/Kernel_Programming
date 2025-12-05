// read_stats.c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    int fd = open("/dev/rwstats", O_RDONLY);
    if (fd < 0) {
        perror("open /dev/rwstats");
        return 1;
    }

    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf)-1);
    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }
    buf[n] = '\0';
    printf("%s", buf);

    close(fd);
    return 0;
}

