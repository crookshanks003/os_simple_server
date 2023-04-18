#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "channels.h"

#define MAX_CLIENTS 100

typedef struct {
	char name[256];
	int key;
} client_t;

int client_count = 0;
client_t clients[MAX_CLIENTS];

connect_channel_t connect_chan;
comm_channel_t channels[MAX_CLIENTS];
pthread_t workers[MAX_CLIENTS];

void *comm_channel_worker(void *args) {
	while (1) {
		sleep(1);
	}

	return NULL;
}

void *handle_connection_request(void *args) {
	while (1) {
		if (strlen(connect_chan.req_shm) > 0) {
			sem_wait(connect_chan.req_sem);
			printf("[INCOMING] connect: %s", connect_chan.req_shm);

			const char *msg;
			int code;
			int key;

			int found = 0;
			for (int i = 0; i < client_count; i++) {
				if (strcmp(clients[i].name, connect_chan.req_shm) == 0) {
					found = 1;
					connect_chan.res_shm->code = CODE_INVALID_REQ;
					strcpy(connect_chan.res_shm->msg, "name already exist");
					break;
				}
			}

			if (found == 0) {
				key = comm_channel_create(connect_chan.req_shm, &channels[client_count]);
				if (key == -1) {
					code = CODE_SERVER_ERR;
					msg = "something went wrong";
				} else {
					int status = pthread_create(&workers[client_count], NULL, comm_channel_worker, NULL);
					if (status != 0) {
						perror("failed to create comm thread");
						code = CODE_SERVER_ERR;
						msg = "something went wrong";
					} else {
						strcpy(clients[client_count].name, connect_chan.req_shm);
						clients[client_count].key = key;
						client_count++;

						code = CODE_SUCCESS;
						msg = "success";
					}
				}
			}

			sem_wait(connect_chan.res_sem);
			printf("[RESPONSE] name: %s\tcode: %d\tmsg: %s\tkey: %d", connect_chan.req_shm, code, msg, key);
			connect_chan.res_shm->code = code;
			connect_chan.res_shm->key = key;
			strcpy(connect_chan.res_shm->name, connect_chan.req_shm);
			strcpy(connect_chan.res_shm->msg, msg);
			sem_post(connect_chan.res_sem);

			strcpy(connect_chan.req_shm, "");
			sem_post(connect_chan.req_sem);
		}

		usleep(100);
	}

	return NULL;
}

int main() {
	if (connect_channel_create(&connect_chan) < 0) {
		exit(1);
	}

	pthread_t th;
	int i = pthread_create(&th, NULL, handle_connection_request, NULL);
	if (i != 0) {
		perror("failed to create connect thread");
		exit(1);
	}

	pthread_join(th, NULL);

	connect_channel_exit(&connect_chan);

	return 0;
}
