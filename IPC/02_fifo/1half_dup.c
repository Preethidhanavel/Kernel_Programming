#include<stdio.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<unistd.h>
int main()
{
int fd,fd1;
mkfifo("f1",0644);
mkfifo("f2",0644);
fd=open("f1",O_RDONLY);
fd1=open("f2",O_WRONLY);
if((fd<0)||(fd1<0)){
perror("open");
return 0;
}
printf("p2  opened\n");
char a[50],b[50];
while(1){
read(fd,b,sizeof(b));
printf("message read by p2 %s\n",b);
scanf(" %[^\n]",a);
write(fd1,a,strlen(a)+1);
printf("written successfully\n");
}

}
