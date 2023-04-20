#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <unistd.h>

#include "channels.h"

#define MAX_CLIENTS 100

int client_count = 0;
client_t clients[MAX_CLIENTS];

connect_channel_t connect_chan;
pthread_t workers[MAX_CLIENTS];

int arithmetic(int n1, int n2, char op) {
	if (op == '+') return n1 + n2;
	if (op == '-') return n1 - n2;
	if (op == '*') return n1 * n2;
	if (op == '/') return n1 / n2;

	return -1;
}

int even_or_odd(int n) { return n % 2 == 0; }

int is_prime(int num) {
	if (num <= 1) return 0;
	for (int i = 2; i * i <= num; i++) {
		if (num % i == 0) return 0;
	}
	return 1;
}

void deregister(client_t *cl) {
	shmdt(cl->ch.req_shm);
	shmdt(cl->ch.res_shm);
	shmctl(cl->ch.req_shmid, IPC_RMID, NULL);
	shmctl(cl->ch.res_shmid, IPC_RMID, NULL);
	sem_post(cl->ch.sem);
	sem_close(cl->ch.sem);

	cl->key = -1;
	strcpy(cl->name, "0");

	client_count--;
	printf("[INFO] cleaned communication channel");
}

void *comm_channel_worker(void *args) {
	client_t *cl = (client_t *)args;
	comm_channel_t *ch = &cl->ch;
	int req_id = ch->req_shm->id;

	while (1) {
		sem_wait(ch->sem);
		if (ch->req_shm->id > req_id) {
			req_id = ch->req_shm->id;
			int result;
			int code = CODE_SUCCESS;
			const char *msg = "success";

			printf("[INCOMING] name:%s  id:%d  action:%d  input:%d %c %d\n", cl->name, req_id, ch->req_shm->action,
				   ch->req_shm->n1, ch->req_shm->op, ch->req_shm->n2);

			switch (ch->req_shm->action) {
			case ARITHMETIC:
				result = arithmetic(ch->req_shm->n1, ch->req_shm->n2, ch->req_shm->op);
				break;
			case EVEN_OR_ODD:
				result = even_or_odd(ch->req_shm->n1);
				break;
			case IS_PRIME:
				result = is_prime(ch->req_shm->n1);
				break;
			case DEREGISTER:
				printf("[INFO] %s client deregistered\n", cl->name);
				deregister(cl);
				pthread_exit(0);
				break;
			default:
				code = CODE_INVALID_REQ;
				msg = "unsupported operation";
				result = -1;
				break;
			}

			ch->res_shm->result = result;
			strcpy(ch->res_shm->msg, msg);
			ch->res_shm->code = code;
			ch->res_shm->id += 1;

			clean_comm_channel_request(ch);

			printf("[OUTGOING] name:%s  id:%d  code:%d  result:%d  msg:%s\n", cl->name, ch->res_shm->id, code, result,
				   msg);
			fflush(stdout);
		}
		sem_post(ch->sem);
		usleep(100);
	}
	return NULL;
}

int get_empty_clients_index(char *name) {
	int i = 0;
	while (clients[i].key != 0) {
		if (clients[i].key == -1) break;
		if (strcmp(clients[i].name, name) == 0) return -1;
		i++;
	}
	return i;
}

void *handle_connection_request(void *args) {
	while (1) {
		if (strlen(connect_chan.req_shm) > 0) {
			sem_wait(connect_chan.req_sem);
			printf("[INCOMING] connect: %s\n", connect_chan.req_shm);

			const char *msg;
			int code;
			int key = -1;

			int index = get_empty_clients_index(connect_chan.req_shm);
			if (index < 0) {
				code = CODE_INVALID_REQ;
				msg = "name already exist";
			}

			else {
				key = hash(connect_chan.req_shm);
				int status = comm_channel_create(key, connect_chan.req_shm, &clients[index].ch);
				if (status == -1) {
					code = CODE_SERVER_ERR;
					msg = "something went wrong";
				} else {
					printf("[INFO] creating new thread\n");
					int status = pthread_create(&workers[index], NULL, comm_channel_worker, &clients[index]);
					if (status != 0) {
						perror("failed to create comm thread");
						code = CODE_SERVER_ERR;
						msg = "something went wrong";
					} else {
						strcpy(clients[index].name, connect_chan.req_shm);
						clients[index].key = key;
						client_count++;
						printf("[INFO] %s client connected\n", clients[index].name);

						code = CODE_SUCCESS;
						msg = "success";
					}
				}
			}

			sem_wait(connect_chan.res_sem);
			printf("[OUTGOING] name:%s  code:%d  msg:%s  key:%d\n", connect_chan.req_shm, code, msg, key);
			connect_chan.res_shm->code = code;
			connect_chan.res_shm->key = key;
			strcpy(connect_chan.res_shm->name, connect_chan.req_shm);
			strcpy(connect_chan.res_shm->msg, msg);
			sem_post(connect_chan.res_sem);

			strcpy(connect_chan.req_shm, "");
			sem_post(connect_chan.req_sem);
			fflush(stdout);
		}

		usleep(100);
	}

	return NULL;
}

int main() {
	setbuf(stdout, NULL);

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
