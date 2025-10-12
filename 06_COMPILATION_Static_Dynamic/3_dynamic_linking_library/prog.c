#include <stdio.h>         // Standard input-output library
#include "math_test.h"     // header file (contains function declarations for add, sub, mul)

int main()
{
    int a = 3, b = 8;   // Initialize two integers a and b

    // Call add, sub, and mul functions and print the results
    printf("add:%d\tsub:%d\tmul:%d\n", add(a, b), sub(a, b), mul(a, b));

    return 0;   // Return 0 to indicate successful execution
}
