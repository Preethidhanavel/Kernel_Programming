#include<stdio.h>
#include<string.h>
#include<unistd.h>
int main()
{
	int fd[2];
	if(pipe(fd)<0)
	{
		perror("pipe creation failed\n");
		return 0;
	}

	if(fork()==0)
	{
		char c[50];
		printf("in child\n");
		scanf(" %[^\n]",c);
		write(fd[1],c,strlen(c)+1);
	}
	else
	{
		char c[50];
		printf("in parent\n");
		read(fd[0],c,sizeof(c));
		printf("%s\n",c);
	}
}
