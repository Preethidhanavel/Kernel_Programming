#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>

// IOCTL command to check threshold condition
#define THRESHOLD_CHECK _IOWR('a', 0x11, int)

// Expected return values from kernel
#define TH_HIGH 0x22
#define TH_LOW  0x33
#define TH_WITH_IN_LIMIT 0x44

int main()
{
    int temp = 0;

    // Open the device file /dev/mydevice
    int fd = open("/dev/mydevice", O_RDWR);

    // Check if file opened successfully
    if (fd == -1) {
        perror("No device file created\n");
        return -1;
    }

    // Read a temperature value from the driver
    read(fd, &temp, sizeof(temp));
    printf("Temp : %d\n", temp);

    int arg = 0;

    printf("\nVerifying the sensor reading...\n");

    // Calling ioctl to check threshold condition
    ioctl(fd, THRESHOLD_CHECK, &arg);

    // Compare returned argument value with defined states
    if (arg == TH_HIGH)
        printf("Temperature is higher than the limit\n");

    else if (arg == TH_LOW)
        printf("Temperature is lower than the limit\n");

    else if (arg == TH_WITH_IN_LIMIT)
        printf("Temperature is within limit\n");

    // Close the device file
    close(fd);

    return 0;
}
