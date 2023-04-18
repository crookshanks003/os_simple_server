#include "channels.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>

// time in ms after which the client disconnects
#define TIMEOUT 3000

connect_channel_t conn_ch;

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("A client-name must be provided as an argument");
		exit(1);
	}

	if (connect_channel_create(&conn_ch) < 0) {
		exit(1);
	}

	sem_post(conn_ch.res_sem);
	sem_wait(conn_ch.req_sem);
	strcpy(conn_ch.req_shm, argv[1]);
	sem_post(conn_ch.req_sem);

	int timeout = 0;
	while (1) {
		if (timeout / 10 == TIMEOUT) {
			printf("connection timed out");
			break;
		}
		sem_wait(conn_ch.res_sem);
		if (strlen(conn_ch.res_shm->msg) > 0) {
			printf("%s\n", conn_ch.res_shm->msg);
			printf("%d\n", conn_ch.res_shm->key);
			conn_ch.res_shm->code = -1;
			conn_ch.res_shm->key = -1;
			strcpy(conn_ch.res_shm->name, "");
			strcpy(conn_ch.res_shm->msg, "");
			sem_post(conn_ch.res_sem);
			break;
		}
		sem_post(conn_ch.res_sem);
		usleep(100);
		timeout++;
	}

	connect_channel_exit(&conn_ch);
}
