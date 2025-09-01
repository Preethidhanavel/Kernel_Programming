#ifndef _SYMBOL_H       // Header guard start: prevents multiple inclusion of this file
#define _SYMBOL_H

// Define a structure named 'test' with two integers and one character
struct test {
    int a;   
    int b;   
    char c; 
};

// Declare an external variable 'foo' of type 'struct test'
extern struct test foo;

#endif  // End of header guard
