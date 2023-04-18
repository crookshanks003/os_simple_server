#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "channels.h"

typedef struct {
	char name[256];
	char key[256];
} client_info_t;

connect_channel_t chan;

void *handle_connection_request(void *args) {
	while (1) {
		if (strlen(chan.req_shm) > 0) {
			printf("%s\n", chan.req_shm);

			sem_wait(chan.sem);
			strcpy(chan.req_shm, "");
			sem_post(chan.sem);
		}

		usleep(100);
	}

	return NULL;
}

int main() {
	if (connect_channel_create(&chan) < 0) {
		exit(1);
	}

	pthread_t th;
	int i = pthread_create(&th, NULL, handle_connection_request, NULL);
	if (i != 0) {
		perror("failed to create connect thread");
		exit(1);
	}

	pthread_join(th, NULL);

	connect_channel_exit(&chan);

	return 0;
}
