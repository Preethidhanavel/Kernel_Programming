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
fd=open("f1",O_WRONLY);
fd1=open("f2",O_RDONLY);
if((fd<0)||(fd1<0)){
perror("open");
return 0;
}
printf("f1 opened\n");
char a[50],b[50];
while(1){
scanf(" %[^\n]",a);
write(fd,a,strlen(a)+1);
printf("written successfully\n");
read(fd1,b,sizeof(b));
printf("message read by p1 %s\n",b);
}

}
