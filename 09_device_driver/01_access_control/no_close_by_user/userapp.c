#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>

void *func(void *arg)
{
	int fd;
	
	fd = open("/dev/msg", O_RDWR);
	sleep(5);
	return NULL;
}

int main(int argc, char *argv[])
{
	pthread_t thread1, thread2;

	pthread_create(&thread1,NULL, func, NULL);
	pthread_create(&thread2,NULL, func, NULL);
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	return 0;
}
