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
enum response_code { CODE_SUCCESS, CODE_INVALID_REQ, CODE_SERVER_ERR };

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

enum actions {
	ARITHMETIC,
	EVEN_OR_ODD,
	IS_PRIME,
	DEREGISTER,
};

typedef struct {
	int id;
	int n1;
	int n2;
	char op;
	int action;
} comm_request_t;

typedef struct {
	int id;
	int result;
	int code;
	char msg[256];
} comm_response_t;

// channel type for the communication channel
typedef struct {
	int req_shmid;
	int res_shmid;
	comm_request_t *req_shm;
	comm_response_t *res_shm;
	sem_t *sem;
} comm_channel_t;

__BEGIN_DECLS

/* Creates and connects to the connect channel. The shared memory object is stored in ch. */
extern int connect_channel_create(connect_channel_t *ch);

/* Call all the cleanup functions for the given channel.*/
extern void connect_channel_exit(connect_channel_t *ch);

/* Creates an unique hash for the given string*/
extern int hash(const char *name);

/* Create communication channel. The shared memory object is stored in ch. */
extern int comm_channel_create(int key, char *name, comm_channel_t *ch);

/* Call all the cleanup functions for the given channel.*/
extern void comm_channel_exit(comm_channel_t *ch);

/* Sets all the values for request object to default*/
extern void clean_comm_channel_request(comm_channel_t *ch);

__END_DECLS
