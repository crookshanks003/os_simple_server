#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <sys/shm.h>

#define CONNECT_SEM_NAME "/connect_sem_req"
#define CONNECT_RES_SEM_NAME "/connect_sem_res"
#define CONNECT_REQ_KEY 1234
#define CONNECT_RES_KEY 1235

// response codes for the connection request
enum { CODE_SUCCESS, CODE_INVALID_REQ, CODE_SERVER_ERR };

// response type for the connection request
typedef struct {
	char name[256];
	int code;
	char msg[256];
	int key;
} connect_response_t;

// channel type for the connection channel
typedef struct {
	char *req_shm;
	connect_response_t *res_shm;
	sem_t *req_sem;
	sem_t *res_sem;
} connect_channel_t;

// channel type for the communication channel
typedef struct {
	char *req_shm;
	char *res_shm;
} comm_channel_t;

__BEGIN_DECLS

/* Creates and connects to the connect channel. The shared memory object is stored in ch. */
extern int connect_channel_create(connect_channel_t *ch);

/* Call all the cleanup functions for the given channel.  */
extern void connect_channel_exit(connect_channel_t *ch);

/* Create communication channel and returns a unique key. The shared memory object is stored in ch. */
extern int comm_channel_create(char *name, comm_channel_t *ch);

__END_DECLS
