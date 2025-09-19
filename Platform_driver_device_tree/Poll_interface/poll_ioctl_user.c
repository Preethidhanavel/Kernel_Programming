#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <errno.h>

/* Must match driver definitions */
#define GPIOBTN_MAGIC     'G'
#define GPIOBTN_RESET     _IO(GPIOBTN_MAGIC, 0)
#define GPIOBTN_LED_ON    _IO(GPIOBTN_MAGIC, 1)
#define GPIOBTN_LED_OFF   _IO(GPIOBTN_MAGIC, 2)
#define GPIOBTN_NOTIFY_EN _IO(GPIOBTN_MAGIC, 3)
#define GPIOBTN_NOTIFY_DIS _IO(GPIOBTN_MAGIC, 4)

/* Print usage instructions */
static void usage(const char *prog)
{
    printf("Usage: %s <device> <command>\n", prog);
    printf("  device  : /dev/gpiobtnX\n");
    printf("  command : read | write1 | reset | led_on | led_off |\n");
    printf("            notify_on | notify_off | monitor\n");
}

int main(int argc, char *argv[])
{
    int fd;
    char buf[64];

    /* Check arguments */
    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    /* Open the device file */
    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* ---------- Read button press count ---------- */
    if (!strcmp(argv[2], "read")) {
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';  // null terminate for printing
            printf("Read from %s: %s", argv[1], buf);
        } else if (n == 0) {
            printf("EOF reached\n");
        } else {
            perror("read");
        }
    }
    /* ---------- Write '1' to toggle LED ---------- */
    else if (!strcmp(argv[2], "write1")) {
        if (write(fd, "1", 1) < 0)
            perror("write");
        else
            printf("Wrote '1' to %s (toggle LED)\n", argv[1]);
    }
    /* ---------- IOCTL: Reset button press counter ---------- */
    else if (!strcmp(argv[2], "reset")) {
        if (ioctl(fd, GPIOBTN_RESET) < 0)
            perror("ioctl reset");
        else
            printf("Press counter reset\n");
    }
    /* ---------- IOCTL: Turn LED on ---------- */
    else if (!strcmp(argv[2], "led_on")) {
        if (ioctl(fd, GPIOBTN_LED_ON) < 0)
            perror("ioctl led_on");
        else
            printf("LED turned ON\n");
    }
    /* ---------- IOCTL: Turn LED off ---------- */
    else if (!strcmp(argv[2], "led_off")) {
        if (ioctl(fd, GPIOBTN_LED_OFF) < 0)
            perror("ioctl led_off");
        else
            printf("LED turned OFF\n");
    }
    /* ---------- IOCTL: Enable notifications ---------- */
    else if (!strcmp(argv[2], "notify_on")) {
        if (ioctl(fd, GPIOBTN_NOTIFY_EN) < 0)
            perror("ioctl notify_on");
        else
            printf("Notifications ENABLED\n");
    }
    /* ---------- IOCTL: Disable notifications ---------- */
    else if (!strcmp(argv[2], "notify_off")) {
        if (ioctl(fd, GPIOBTN_NOTIFY_DIS) < 0)
            perror("ioctl notify_off");
        else
            printf("Notifications DISABLED\n");
    }
    /* ---------- Monitor button presses (blocking using poll) ---------- */
    else if (!strcmp(argv[2], "monitor")) {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;

        printf("Monitoring %s... press Ctrl+C to stop\n", argv[1]);
        while (1) {
            int ret = poll(&pfd, 1, -1); // block until button press
            if (ret < 0) {
                if (errno == EINTR)
                    break; // interrupted by signal
                perror("poll");
                break;
            }
            if (pfd.revents & POLLIN) {
                int n = read(fd, buf, sizeof(buf) - 1);
                if (n > 0) {
                    buf[n] = '\0';
                    printf("%s", buf);
                }
            }
        }
    }
    else {
        usage(argv[0]);
    }

    close(fd);
    return 0;
}
