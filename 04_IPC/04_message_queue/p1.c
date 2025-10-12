// write the message into the message queue..

#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<string.h>
#include<stdlib.h>
struct msgbuf
{
	long mtype;
	char mtext[20];
};
int main(int argc,char *argv[])
{
	if(argc!=3)
	{
		printf("usage: a.out mtype mtext\n");
		return 0;
	}

	int msgid;
	msgid=msgget(5,IPC_CREAT|0644);

	if(msgid<0)
	{
		perror("msgget");
		return 0;
	}

	printf("msg.id=%d \n",msgid);

	struct msgbuf vi;
	vi.mtype=atoi(argv[1]);
	strcpy(vi.mtext,argv[2]);
	msgsnd(msgid,&vi,strlen(vi.mtext)+1,0);
	printf("msg send successfully..\n");


}

