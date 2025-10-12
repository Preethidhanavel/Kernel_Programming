#include<stdio.h>

// Define a global variable 'x' with value 7
int x = 7;

// Create another global variable 'y' as an alias to 'x'
extern int y __attribute__((alias("x")));

// Define a static function 'func' that adds two numbers
static int func(int a, int b)
{
    printf("The result is %d\n", a + b);
    return a + b;
}

// Create another function 'add' as an alias for 'func'
static int add(int a, int b) __attribute__((alias("func")));

int main()
{
    // Calling 'add' which internally uses 'func'
    add(3, 6);

    // Direct call to 'func'
    func(5, 10);

    // Print value of 'y' (which is alias of 'x' -> 7)
    printf("%d\n", y);

    return 0;
}
