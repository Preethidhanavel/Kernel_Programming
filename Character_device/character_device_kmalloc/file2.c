#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define DEVICE "/dev/device1"  // Path to the character device
#define MSG_LEN 100            // Maximum buffer size for reading/writing
#define NUM_MSGS 3             // Number of messages to send

int main() {
    int fd;                   // File descriptor for the device
    char buffer[MSG_LEN];     // Buffer to read data from the device
    const char *messages[NUM_MSGS] = {  // Messages to write
        "First message from user",
        "Second message from user",
        "Third message from user"
    };

    // Open the device for reading and writing
    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return EXIT_FAILURE;  // Exit if device cannot be opened
    }

    // Write multiple messages to the device
    for (int i = 0; i < NUM_MSGS; i++) {
        if (write(fd, messages[i], strlen(messages[i])) < 0) {
            perror("Failed to write to device");
            close(fd);
            return EXIT_FAILURE;  // Exit if write fails
        }
        printf("Written: %s\n", messages[i]);  // Confirm message written
    }

    // Reset file offset to the beginning (like rewind)
    lseek(fd, 0, SEEK_SET);

    printf("\nReading messages back from device:\n");
    // Read messages until device buffer is empty
    while (1) {
        ssize_t n = read(fd, buffer, sizeof(buffer) - 1); // Read from device
        if (n <= 0) break;  // Stop if no more data
        buffer[n] = '\0';   // Null-terminate string
        printf("Read: %s\n", buffer);  // Print data read
    }

    // Close the device
    close(fd);
    return EXIT_SUCCESS;
}
