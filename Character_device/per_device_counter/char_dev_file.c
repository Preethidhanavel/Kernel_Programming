#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define NUM_DEVICES 3      // Number of minor devices
#define BUF_SIZE 256       // Buffer size for reading/writing

int main()
{
    int fd[NUM_DEVICES];              // Array to store file descriptors
    char dev_name[32];                // Buffer to hold device file name
    char read_buf[BUF_SIZE + 64];     // Buffer to read device data (extra space for write count)
    int i;

    // Messages to write to each device
    const char *messages[NUM_DEVICES] = {
        "Hello from device 0",
        "Message for device 1",
        "Data to device 2"
    };

    // ----------------------------
    // Open all devices
    // ----------------------------
    for (i = 0; i < NUM_DEVICES; i++) {
        snprintf(dev_name, sizeof(dev_name), "/dev/mydev%d", i);  // Generate device path
        fd[i] = open(dev_name, O_RDWR);                            // Open device for read/write
        if (fd[i] < 0) {
            perror("Failed to open device");
            return 1; // Exit if failed
        }
        printf("Opened %s\n", dev_name);
    }

    // ----------------------------
    // Write to each device
    // ----------------------------
    for (i = 0; i < NUM_DEVICES; i++) {
        if (write(fd[i], messages[i], strlen(messages[i])) < 0) {
            perror("Write failed");      // Error writing
        } else {
            printf("Wrote to /dev/mydev%d: %s\n", i, messages[i]);
        }
    }

    // ----------------------------
    // Read from each device
    // ----------------------------
    for (i = 0; i < NUM_DEVICES; i++) {
        lseek(fd[i], 0, SEEK_SET); // Reset file offset to start
        int n = read(fd[i], read_buf, sizeof(read_buf)-1);  // Read device content
        if (n < 0) {
            perror("Read failed");
        } else {
            read_buf[n] = '\0'; // Null-terminate the string
            printf("Read from /dev/mydev%d:\n%s\n", i, read_buf);
        }
    }

    // ----------------------------
    // Close all devices
    // ----------------------------
    for (i = 0; i < NUM_DEVICES; i++) {
        close(fd[i]);
    }

    return 0;
}
