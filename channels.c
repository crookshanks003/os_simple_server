#include "channels.h"
#include <semaphore.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/shm.h>

int connect_channel_create(connect_channel_t *ch) {
	int req_shmid = shmget(CONNECT_REQ_KEY, 1024, IPC_CREAT | 0666);
	if (req_shmid < 0) {
		perror("failed to create shared mem for connect req");
		return -1;
	}

	char *req_shmaddr = (char *)shmat(req_shmid, NULL, 0);
	if (req_shmaddr == (char *)-1) {
		perror("failed to attach to shared mem for connect req");
		return -1;
	}

	int res_shmid = shmget(CONNECT_RES_KEY, sizeof(connect_response_t), IPC_CREAT | 0666);
	if (res_shmid < 0) {
		perror("failed to create shared mem for connect res");
		return -1;
	}

	connect_response_t *res_shmaddr = (connect_response_t *)shmat(res_shmid, NULL, 0);
	if (req_shmaddr == (char *)-1) {
		perror("failed to attach to shared mem for connect res");
		return -1;
	}

	sem_t *sem = sem_open(CONNECT_SEM_NAME, O_CREAT, 0666, 1);
	if (sem == SEM_FAILED) {
		perror("failed to open sem for connect channel");
		return -1;
	}

	sem_t *res_sem = sem_open(CONNECT_RES_SEM_NAME, O_CREAT, 0666, 1);
	if (sem == SEM_FAILED) {
		perror("failed to open sem for connect channel");
		return -1;
	}

	ch->req_shm = req_shmaddr;
	ch->res_shm = res_shmaddr;
	ch->req_sem = sem;
	ch->res_sem = res_sem;

	printf("[INFO] connect channel created\n");

	return 0;
}

void connect_channel_exit(connect_channel_t *ch) {
	shmdt(ch->req_shm);
	shmdt(ch->res_shm);
	return;
}

int hash(const char *name) {
	int key = 0;
	int l = 2;
	int c = 0;

	while ((c = *name++)) {
		key += (c * l);
		l++;
	}

	return key;
}

int comm_channel_create(int key, char *name, comm_channel_t *ch) {
	int req_shmid = shmget(key + 113, sizeof(comm_request_t), IPC_CREAT | 0666);
	if (req_shmid < 0) {
		printf("failed to shmget req");
		return -1;
	}
	comm_request_t *req_shm = (comm_request_t *)shmat(req_shmid, NULL, 0);
	if (req_shm == (comm_request_t *)-1) {
		printf("failed to attach to shared mem for req");
		return -1;
	}

	int res_shmid = shmget(key + 115, sizeof(comm_response_t), IPC_CREAT | 0666);
	if (res_shmid < 0) {
		printf("failed to create shared mem for res");
		return -1;
	}
	comm_response_t *res_shm = (comm_response_t *)shmat(res_shmid, NULL, 0);
	if (res_shm == (comm_response_t *)-1) {
		printf("failed to attach to shared mem for res");
		return -1;
	}

	char sem_name[256];
	sprintf(sem_name, "/%s", name);
	sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 1);
	if (sem == SEM_FAILED) {
		perror("failed to open sem for connect channel");
		return -1;
	}

	ch->res_shm = res_shm;
	ch->req_shm = req_shm;
	ch->sem = sem;
	ch->req_shmid = req_shmid;
	ch->res_shmid = res_shmid;

	printf("[INFO] communication channel created\n");

	return 0;
}

void comm_channel_exit(comm_channel_t *ch) {
	shmdt(ch->res_shm);
	shmdt(ch->req_shm);
}

void clean_comm_channel_request(comm_channel_t *ch) {
	ch->req_shm->n1 = 0;
	ch->req_shm->n2 = 0;
	ch->req_shm->op = (char)0;
	ch->req_shm->action = -1;
}
