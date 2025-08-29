#include<stdio.h>
int x=7;
extern int y __attribute__((alias("x")));
static int func(int a,int b)
{
	printf("The result is %d\n",a+b);
	return a+b;
}
static int add(int a,int b) __attribute__((alias("func")));
int main()
{
	add(3,6);
	func(5,10);
printf("%d\n",y);
}
