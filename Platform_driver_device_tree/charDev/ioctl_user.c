#include <stdio.h>      // For printf, perror
#include <fcntl.h>      // For open()
#include <unistd.h>     // For close(), sleep()
#include <sys/ioctl.h>  // For ioctl()

/* IOCTL commands (must match kernel driver definitions) */
#define GPIOBTN_MAGIC   'G'
#define GPIOBTN_RESET   _IO(GPIOBTN_MAGIC, 0)   // Reset press counter
#define GPIOBTN_LED_ON  _IO(GPIOBTN_MAGIC, 1)   // Turn LED ON
#define GPIOBTN_LED_OFF _IO(GPIOBTN_MAGIC, 2)   // Turn LED OFF

int main() 
{
    /* Open the device file created by the driver */
    int fd = open("/dev/gpiobtn", O_RDWR);
    if (fd < 0) {
        perror("open"); // Print error if device can't be opened
        return 1;
    }

    /* Send IOCTL to turn LED ON */
    printf("Turning LED ON\n");
    ioctl(fd, GPIOBTN_LED_ON);

    sleep(2);  // Wait for 2 seconds

    /* Send IOCTL to turn LED OFF */
    printf("Turning LED OFF\n");
    ioctl(fd, GPIOBTN_LED_OFF);

    /* Reset the button press counter */
    printf("Resetting counter\n");
    ioctl(fd, GPIOBTN_RESET);

    /* Close the device */
    close(fd);
    return 0;
}
