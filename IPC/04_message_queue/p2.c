// read the message from message queue..

#include<stdio.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<stdlib.h>
struct msgbuf
{
	long mtype;
	char mtext[20];
};
int main(int argc,char *argv[])
{
	if(argc!=2)
	{
		printf("usage: a.out mtype\n");
		return 0;
	}
	int msgid;
	msgid=msgget(5,IPC_CREAT|0644);
	if(msgid<0)
	{
		perror("msgget");
		return 0;
	}

	struct msgbuf v;
	v.mtype=atoi(argv[1]);
	msgrcv(msgid,&v,sizeof(v.mtext),v.mtype,0);
	printf("recived msg= %s\n",v.mtext);
	

}
