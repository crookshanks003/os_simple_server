#include "channels.h"
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>

connect_channel_t ch;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("A client-name must be provided as an argument");
		exit(1);
	}

	if (connect_channel_create(&ch) < 0) {
		exit(1);
	}

	sem_wait(ch.sem);
	strcpy(ch.req_shm, argv[1]);
	sem_post(ch.sem);

	connect_channel_exit(&ch);
}
