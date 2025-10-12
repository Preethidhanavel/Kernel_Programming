#include <stdio.h>        // Standard input-output header
#include "math_test.h"    //header file (contains declarations for add, sub, and mul functions)

int main()
{
    int a = 10, b = 8;   // Declare and initialize two integers

    // Call add, sub, and mul functions and print their results
    printf("add %d sub %d mul %d\n", add(a, b), sub(a, b), mul(a, b));

    return 0;   // Indicate successful program termination
}
