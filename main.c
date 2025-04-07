#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "include/scraper.h"
#include "include/opc_client.h"

GenerationData shared_gen;
ExchangeData shared_exch;
FrequencyData shared_freq;

sem_t sem_empty;
sem_t sem_full;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *fetch_thread(void *arg) {
    while (1) {
        sem_wait(&sem_empty);
        pthread_mutex_lock(&mutex);

        scraper(&shared_gen, &shared_exch, &shared_freq);

        pthread_mutex_unlock(&mutex);
        sem_post(&sem_full);

        sleep(15); // cykliczne odpytanie
    }
    return NULL;
}

void *send_thread(void *arg) {
    while (1) {
        sem_wait(&sem_full);
        pthread_mutex_lock(&mutex);

        send_to_opcua("opc.tcp://localhost:4840", &shared_gen, &shared_exch, &shared_freq);

        pthread_mutex_unlock(&mutex);
        sem_post(&sem_empty);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;

    sem_init(&sem_empty, 0, 1);
    sem_init(&sem_full, 0, 0);

    pthread_create(&t1, NULL, fetch_thread, NULL);
    pthread_create(&t2, NULL, send_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    pthread_mutex_destroy(&mutex);

    return 0;
}
