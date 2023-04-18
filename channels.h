#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <sys/shm.h>

#define CONNECT_SEM_NAME "/connect_sem"
#define CONNECT_REQ_KEY 1234
#define CONNECT_RES_KEY 1235

typedef struct {
	char *req_shm;
	char *res_shm;
	sem_t *sem;
} connect_channel_t;

__BEGIN_DECLS

/* Creates and connects to the connect channel. The shared memory object is stored in ch. */
extern int connect_channel_create(connect_channel_t *ch);

/* Call all the cleanup functions for the given channel */
extern void connect_channel_exit(connect_channel_t *ch);

__END_DECLS
