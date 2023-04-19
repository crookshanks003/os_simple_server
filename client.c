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

client_t client;

int key = -1;
int res_id = -1;

void clean_conn_ch() {
	conn_ch.res_shm->code = -1;
	conn_ch.res_shm->key = -1;
	strcpy(conn_ch.res_shm->name, "");
	strcpy(conn_ch.res_shm->msg, "");
}

int connect(char *name) {
	sem_wait(conn_ch.req_sem);
	strcpy(conn_ch.req_shm, name);
	sem_post(conn_ch.req_sem);

	int timeout = 0;
	while (1) {
		if (timeout / 10 == TIMEOUT) {
			printf("connection timed out\n");
			return -1;
		}
		sem_wait(conn_ch.res_sem);
		if (strlen(conn_ch.res_shm->msg) > 0) {
			printf("%s\n", conn_ch.res_shm->msg);

			if (conn_ch.res_shm->code != 0) {
				clean_conn_ch();
				sem_post(conn_ch.res_sem);
				return -1;
			}
			client.key = conn_ch.res_shm->key;

			clean_conn_ch();
			sem_post(conn_ch.res_sem);
			printf("connected to the server\n");
			break;
		}
		sem_post(conn_ch.res_sem);
		usleep(100);
		timeout++;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("A client-name must be provided as an argument");
		exit(1);
	}
	strcpy(client.name, argv[1]);

	if (connect_channel_create(&conn_ch) < 0) {
		exit(1);
	}

	int status = connect(client.name);
	if (status < 0) {
		printf("exiting...");
		exit(1);
	}

	if (comm_channel_create(client.key, client.name, &client.ch) != 0) {
		printf("failed to connect to comm channel for\n");
		exit(1);
	}

	res_id = client.ch.res_shm->id;

	sem_wait(client.ch.sem);
	client.ch.req_shm->action = ARITHMETIC;
	client.ch.req_shm->n1 = 50;
	client.ch.req_shm->n2 = 80;
	client.ch.req_shm->op = '+';
	client.ch.req_shm->id += 1;
	sem_post(client.ch.sem);

	int timeout = 0;
	while (1) {
		if (timeout / 10 == TIMEOUT) {
			printf("connection timed out\n");
			return -1;
		}
		sem_wait(client.ch.sem);
		if (client.ch.res_shm->id > res_id) {
			res_id = client.ch.res_shm->id;

			printf("id: %d  result: %d\n", res_id, client.ch.res_shm->result);
			sem_post(client.ch.sem);
			break;
		}
		sem_post(client.ch.sem);
		usleep(100);
		timeout++;
	}

	sleep(5);

	sem_wait(client.ch.sem);
	printf("deregistering...");
	client.ch.req_shm->action = DEREGISTER;
	client.ch.req_shm->n1 = key;
	client.ch.req_shm->id += 1;
	sem_post(client.ch.sem);

	comm_channel_exit(&client.ch);
	connect_channel_exit(&conn_ch);
}
