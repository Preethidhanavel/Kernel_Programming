#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include"ioctl_data.h"
#include<unistd.h>
#define IOCTL_GET_RESULT _IOWR('a','b',struct data)
int main()
{
int fd = open("/dev/mydev1",O_RDWR);

struct data d={
.arr = {1,2,3,4,5,6},
.size=6
};

ioctl(fd,IOCTL_GET_RESULT,&d);

printf("MAX = %d \n",d.max);
printf("MIN = %d\n",d.min);

close(fd);
return 0;
}


