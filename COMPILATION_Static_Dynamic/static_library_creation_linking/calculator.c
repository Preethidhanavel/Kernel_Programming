#include <stdio.h>     // Standard input-output header
#include "math.h"      // User-defined header file (contains add, sub, mul function declarations)

int main()
{
    int a = 2, b = 3, res;   // Initialize two integers and a variable for result
    int op;                  // Variable to store user's option

    printf("Enter the option:");   // Prompt user for input
    scanf("%d", &op);              // Read option from user

    while (1)   // Infinite loop
    {
        switch (op)   // Check which option user entered
        {
            case 1: res = add(a, b);   // Call add function
                    break;

            case 2: res = sub(a, b);   // Call sub function
                    break;

            case 3: res = mul(a, b);   // Call mul function
                    break;

            default: printf("invalid option\n");  // If option is not valid
                     return 0;   // Exit program
        }
        break;   // Exit while loop after executing one case
    }

    // Print the final result
    printf("the result is %d\n", res);

    return 0;   // Successful program termination
}
