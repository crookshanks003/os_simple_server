#include "channels.h"
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

	return 0;
}

void connect_channel_exit(connect_channel_t *ch) {
	sem_close(ch->req_sem);
	sem_close(ch->res_sem);
	shmdt(ch->req_shm);
	shmdt(ch->res_shm);
	return;
}

// creates an unique hash for the given string
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

int comm_channel_create(char *name, comm_channel_t *ch) {
	int key = hash(name);

	int req_fd = shmget(key + 113, 1024, IPC_CREAT | 0666);
	if (req_fd < 0) {
		printf("failed to shmget req");
		return -1;
	}
	char *req_shm = (char *)shmat(req_fd, NULL, 0);
	if (req_shm == (char *)-1) {
		printf("failed to attach to shared mem for req");
		return -1;
	}

	int res_fd = shmget(key + 115, 1024, IPC_CREAT | 0666);
	if (res_fd < 0) {
		printf("failed to create shared mem for res");
		return -1;
	}
	char *res_shm = (char *)shmat(res_fd, NULL, 0);
	if (res_shm == (char *)-1) {
		printf("failed to attach to shared mem for res");
		return -1;
	}

	ch->res_shm = res_shm;
	ch->req_shm = req_shm;

	return key;
}
