#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include "env_final.h"

struct sensor_data *shared;
sem_t *sem_pd;
sem_t *sem_ps;
pthread_mutex_t lock;

void cleanup(int sig)
{
    printf("\n[Display] Cleaning up...\n");

    sem_close(sem_pd);
    sem_close(sem_ps);

    exit(0);
}

void* display_thread(void *arg)
{
    while(1)
    {
        sem_wait(sem_pd);

        struct sensor_data local = *shared;

        if(local.temperature == -1)
            continue;

        pthread_mutex_lock(&lock);

        /* 🔥 PRINT TO TERMINAL (stderr) */
        fprintf(stderr, "[Display] Temp=%d Hum=%d AQI=%d\n",
                local.temperature, local.humidity, local.aqi);

        /* 🔥 SEND TO LOGGER (stdout → pipe) */
        dprintf(STDOUT_FILENO,
                "[Display] Temp=%d Hum=%d AQI=%d\n",
                local.temperature, local.humidity, local.aqi);

        if(local.temperature > 40)
        {
            fprintf(stderr, "[ALERT] High Temperature\n");
            dprintf(STDOUT_FILENO, "[ALERT] High Temperature\n");
        }

        if(local.aqi > 200)
        {
            fprintf(stderr, "[ALERT] Poor Air Quality\n");
            dprintf(STDOUT_FILENO, "[ALERT] Poor Air Quality\n");
        }

        fflush(stderr);
        fflush(stdout);

        pthread_mutex_unlock(&lock);

        sem_post(sem_ps);

        usleep(500000);
    }
}

int main()
{
    signal(SIGINT, cleanup);

    printf("[Display] Started...\n");

    int shm_fd;
    while((shm_fd = shm_open(SHM_NAME, O_RDWR, 0666)) == -1)
        sleep(1);

    shared = mmap(NULL, sizeof(struct sensor_data),
                  PROT_READ, MAP_SHARED, shm_fd, 0);

    sem_pd = sem_open(SEM_PD, 0);
    sem_ps = sem_open(SEM_PS, 0);

    pthread_mutex_init(&lock, NULL);

    pthread_t t1;
    pthread_create(&t1, NULL, display_thread, NULL);

    pthread_join(t1, NULL);

    return 0;
}
