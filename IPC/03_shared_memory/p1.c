#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
int main()
{
int shmid;
shmid=shmget(3,50,IPC_CREAT|0644);
if(shmid<0){
perror("shmget");
return 0;
}
printf("shmid=%d\n",shmid);
char *p;
p=shmat(shmid,0,0);
printf("received message:");
printf("%s",p);
}
