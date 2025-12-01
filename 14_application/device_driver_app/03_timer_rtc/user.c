#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include "time.h" // Header containing TIME_LOCK and TIME_UNLOCK macros

int main(int argc, char *argv[])
{
    // Open the device file /dev/timer_rtc with read/write access
    int fd = open("/dev/timer_rtc", O_RDWR);
    if (fd < 0) {
        perror("open"); // Print error if device open fails
        return 1;
    }

    int val;
    
    // Convert first argument (string) into integer operation
    int op = atoi(argv[1]);

    // If user selected operation 1 -> Lock timer
    if (op == 1)
    {
        // Call TIME_LOCK ioctl command
        if (ioctl(fd, TIME_LOCK, &val) < 0) {
            perror("TIME_LOCK failed");
            close(fd);
            return 1;
        }
        printf("Timer Locked Successfully\n");
    }

    // If user selected operation 2 -> Unlock timer
    if (op == 2)
    {
        // Call TIME_UNLOCK ioctl command
        if (ioctl(fd, TIME_UNLOCK, &val) < 0) {
            perror("TIME_UNLOCK failed");
            close(fd);
            return 1;
        }
        printf("Timer Unlocked Successfully\n");
    }

    // Close device file before exiting
    close(fd);
    return 0;
}
