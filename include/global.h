#include <open62541/client.h>
#include <pthread.h>

UA_Client *client;
pthread_mutex_t lock[2];
