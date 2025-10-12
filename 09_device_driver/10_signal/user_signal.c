#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>

#define REG_CURRENT_TASK _IOW('s', 1, int32_t*)  // IOCTL command to register current task
#define SIGETX 44                                 // Custom signal number from kernel

static int done = 0;  // Flag to exit program
int check = 0;         // Flag to check if signal received

// Ctrl-C (SIGINT) handler
void ctrl_c_handler(int n, siginfo_t *info, void *unused)
{
    if (n == SIGINT) {
        printf("\nReceived Ctrl-C\n");
        done = 1;  // Set exit flag
    }
}

// Custom signal handler for SIGETX from kernel
void sig_event_handler(int n, siginfo_t *info, void *unused)
{
    if (n == SIGETX) {
        check = info->si_int;  // Read value sent from kernel
        printf("Received signal from kernel: Value = %u\n", check);
    }
}

int main()
{
    int fd;
    int32_t value, number;
    struct sigaction act;

    // Install Ctrl-C handler to allow clean exit
    sigemptyset(&act.sa_mask);
    act.sa_flags = (SA_SIGINFO | SA_RESETHAND);
    act.sa_sigaction = ctrl_c_handler;
    sigaction(SIGINT, &act, NULL);

    // Install custom signal handler for SIGETX
    sigemptyset(&act.sa_mask);
    act.sa_flags = (SA_SIGINFO | SA_RESTART);
    act.sa_sigaction = sig_event_handler;
    sigaction(SIGETX, &act, NULL);

    printf("Installed signal handler for SIGETX = %d\n", SIGETX);

    // Open the device file
    printf("\nOpening Driver\n");
    fd = open("/dev/dev_device", O_RDWR);
    if (fd < 0) {
        printf("Cannot open device file...\n");
        return 0;
    }

    // Register this process with kernel for signal notifications
    printf("Registering application ...");
    if (ioctl(fd, REG_CURRENT_TASK, (int32_t*)&number)) {
        printf("Failed\n");
        close(fd);
        exit(1);
    }
    printf("Done!!!\n");

    // Main loop waiting for signals
    while (!done) {
        printf("Waiting for signal...\n");

        // Blocking check until signal received or Ctrl-C pressed
        while (!done && !check);
        check = 0;  // Reset after handling
    }

    // Close device before exit
    printf("Closing Driver\n");
    close(fd);
}
