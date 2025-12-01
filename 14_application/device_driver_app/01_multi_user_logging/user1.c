#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#define DEVICE "/dev/mblog"  // Device file path

// IOCTL command definitions (must match kernel module)
#define MBLOG_CLEAR     _IO('m', 1)
#define MBLOG_GET_SIZE  _IOR('m', 2, size_t)

// Function to clear buffer using ioctl()
void clear_buffer(int fd)
{
    if (ioctl(fd, MBLOG_CLEAR) == 0)
        printf("[+] Buffer cleared successfully!\n");
    else
        perror("[-] Failed to clear buffer");
}

// Function to get current stored data size using ioctl()
void get_size(int fd)
{
    size_t size = 0;
    if (ioctl(fd, MBLOG_GET_SIZE, &size) == 0)
        printf("[*] Current buffer size: %lu bytes\n", size);
    else
        perror("[-] Failed to get size");
}

// Function to write user message to the device file
void write_log(int fd)
{
    char msg[256];

    printf("Enter log message: ");
    getchar(); // clear leftover newline in input buffer
    fgets(msg, sizeof(msg), stdin); // read message from user

    int len = strlen(msg);

    // Write message to device
    if (write(fd, msg, len) >= 0)
        printf("[+] Write successful!\n");
    else
        perror("[-] Write failed");
}

// Function to read stored logs from kernel module
void read_log(int fd)
{
    size_t size;
    ioctl(fd, MBLOG_GET_SIZE, &size);  // Get stored length

    if (size == 0) {
        printf("[!] Buffer is empty\n");
        return;
    }

    // Allocate memory dynamically to store logs
    char *buf = malloc(size + 1);
    if (!buf) {
        perror("malloc");
        return;
    }

    lseek(fd, 0, SEEK_SET); // Move file pointer to start

    ssize_t r = read(fd, buf, size); // Read stored data
    buf[r] = '\0'; // Null-terminate string

    printf("\n==== STORED LOGS ====\n%s\n", buf);

    free(buf);
}

int main()
{
    int choice;

    // Open the device file
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/mblog");
        return 1;
    }

    printf("===== MULTI-USER LOG MENU =====\n");

    while (1) {
        // Simple menu interface
        printf("\nSelect an option:\n");
        printf("1. Write log\n");
        printf("2. Read log\n");
        printf("3. Get buffer size\n");
        printf("4. Clear buffer\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        // Perform action based on user input
        switch (choice) {
            case 1: write_log(fd); break;
            case 2: read_log(fd); break;
            case 3: get_size(fd); break;
            case 4: clear_buffer(fd); break;
            case 5:
                printf("Exiting...\n");
                close(fd); // Close device before exiting
                return 0;
            default:
                printf("[!] Invalid option\n");
        }
    }
}
