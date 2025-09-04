#include <stdio.h>    // For printf
#include <math.h>     // For sqrt() function
#include <unistd.h>   // For getpid() function
#include <stdlib.h>   // For rand(), srand()

int main()
{
    int n;

    // Seed the random number generator with the process ID
    // This ensures different random numbers each time the program runs
    srand(getpid());

    // Generate a random number
    n = rand();

    // Print the number and its square root
    printf("squareroot of %d =  %f\n", n, sqrt(n));

    return 0;   // Exit program successfully
}
