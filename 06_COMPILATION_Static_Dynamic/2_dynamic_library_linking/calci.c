#include <stdio.h>   // Standard input-output header

// Function prototypes (declarations)
// These tell the compiler that these functions exist somewhere
int add(int, int);   
int sub(int, int);   
int mul(int, int);   

int main()
{
    int a = 10, b = 8;   // Declare and initialize two integers

    // Call add, sub, and mul functions and print their results
    printf("add %d sub %d mul %d\n", add(a, b), sub(a, b), mul(a, b));

    return 0;   // Return 0 to indicate successful program execution
}
