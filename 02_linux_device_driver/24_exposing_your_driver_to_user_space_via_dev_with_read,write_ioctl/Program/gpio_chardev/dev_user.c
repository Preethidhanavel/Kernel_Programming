#include <stdio.h>
#include <fcntl.h>      // for open()
#include <unistd.h>     // for read(), write(), close()
#include <sys/ioctl.h>  // for ioctl()

// Define ioctl command (must match with driver)
#define GPIOBTN_RESET _IO('G', 0)

int main() 
{
    // Open the char device created by the driver (/dev/gpiobtn)
    int fd = open("/dev/gpiobtn", O_RDWR);
    if (fd < 0) return 1;   // if open fails, exit with error

    char buf[64];

    // Read the current button press count from the driver
    read(fd, buf, sizeof(buf));
    printf("Read: %s", buf);

    // Write "1" -> tells driver to toggle LED state
     write(fd, "1", 1);

    // Send ioctl command -> reset the press counter to 0
    ioctl(fd, GPIOBTN_RESET);

    // Close the device file
    close(fd);
    return 0;
}
