#include<stdio.h>
#include"math.h"
int main()
{
	int a=2,b=3,res;
	int op;
	printf("Enter the option:");
	scanf("%d",&op);
	while(1){

		switch(op)
		{
			case 1:res=add(a,b);
		       		break;
			case 2:res=sub(a,b);
		       		break;	
			case 3:res=mul(a,b);
		       		break;
			default:printf("invalid option\n");
				return 0;
		}
	}
		printf("the result is %d\n",res);
		return 0;
}
