#include "channels.h"
#include <sys/shm.h>

int connect_channel_create(connect_channel_t *ch) {
	int req_shmid = shmget(CONNECT_REQ_KEY, 1024, IPC_CREAT | 0666);
	if (req_shmid < 0) {
		perror("failed to create shared mem for connect channel");
		return -1;
	}

	char *req_shmaddr = (char *)shmat(req_shmid, NULL, 0);
	if (req_shmaddr == (char *)-1) {
		perror("failed to attach to shared mem for connect channel");
		return -1;
	}

	int res_shmid = shmget(CONNECT_RES_KEY, 1024, IPC_CREAT | 0666);
	if (res_shmid < 0) {
		perror("failed to create shared mem for connect channel");
		return -1;
	}

	char *res_shmaddr = (char *)shmat(res_shmid, NULL, 0);
	if (req_shmaddr == (char *)-1) {
		perror("failed to attach to shared mem for connect channel");
		return -1;
	}

	sem_t *sem = sem_open(CONNECT_SEM_NAME, O_CREAT, 0666, 1);
	if (sem == SEM_FAILED) {
		perror("failed to open sem for connect channel");
		return -1;
	}

	ch->req_shm = req_shmaddr;
	ch->res_shm = res_shmaddr;
	ch->sem = sem;

	return 0;
}

void connect_channel_exit(connect_channel_t *ch) {
	sem_close(ch->sem);
	shmdt(ch->req_shm);
	shmdt(ch->res_shm);
	return;
}
