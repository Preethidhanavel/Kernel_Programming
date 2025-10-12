#include <stdio.h>      // For printf(), perror()
#include <stdlib.h>     // For exit codes
#include <string.h>     // For strcmp()
#include <unistd.h>     // For read(), write(), close()
#include <fcntl.h>      // For open()
#include <sys/ioctl.h>  // For ioctl()

/* IOCTL command definitions (must match kernel driver) */
#define GPIOBTN_MAGIC   'G'
#define GPIOBTN_RESET   _IO(GPIOBTN_MAGIC, 0)   // Reset press counter
#define GPIOBTN_LED_ON  _IO(GPIOBTN_MAGIC, 1)   // Force LED ON
#define GPIOBTN_LED_OFF _IO(GPIOBTN_MAGIC, 2)   // Force LED OFF

/* Helper function to print usage instructions */
static void usage(const char *prog)
{
    printf("Usage: %s <device> <command>\n", prog);
    printf("  device  : /dev/gpiobtnX\n");
    printf("  command : read | write1 | reset | led_on | led_off\n");
}

int main(int argc, char *argv[])
{
    int fd;
    char buf[64];

    /* Check arguments */
    if (argc < 3) {
        usage(argv[0]); // Show usage if not enough args
        return 1;
    }

    /* Open the device file (example: /dev/gpiobtn) */
    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    /* ---- Command Handling ---- */
    if (!strcmp(argv[2], "read")) {
        /* Read button press count from driver */
        int n = read(fd, buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0'; // Null-terminate
            printf("Read from %s: %s", argv[1], buf);
        } 
        else {
            perror("read");
        }
    }
    else if (!strcmp(argv[2], "write1")) {
        /* Write '1' to toggle LED state */
        if (write(fd, "1", 1) < 0)
            perror("write");
        else
            printf("Wrote '1' to %s (toggle LED)\n", argv[1]);
    }
    else if (!strcmp(argv[2], "reset")) {
        /* IOCTL to reset press counter */
        if (ioctl(fd, GPIOBTN_RESET) < 0)
            perror("ioctl RESET");
        else
            printf("Counter reset via ioctl\n");
    }
    else if (!strcmp(argv[2], "led_on")) {
        /* IOCTL to force LED ON */
        if (ioctl(fd, GPIOBTN_LED_ON) < 0)
            perror("ioctl LED_ON");
        else
            printf("LED forced ON via ioctl\n");
    }
    else if (!strcmp(argv[2], "led_off")) {
        /* IOCTL to force LED OFF */
        if (ioctl(fd, GPIOBTN_LED_OFF) < 0)
            perror("ioctl LED_OFF");
        else
            printf("LED forced OFF via ioctl\n");
    }
    else {
        /* Invalid command -> show usage */
        usage(argv[0]);
    }

    /* Close device */
    close(fd);
    return 0;
}
