#include <stdio.h>
#include <stdlib.h>   // ✅ FIX ADDED
#include <unistd.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include "env_final.h"

mqd_t mq;
sem_t *sem_sp, *sem_pd;
int shm_fd;

void cleanup(int sig)
{
    printf("\n[Processor] Cleaning up...\n");

    mq_close(mq);

    sem_close(sem_sp);
    sem_close(sem_pd);

    sem_unlink(SEM_PD);

    close(shm_fd);
    shm_unlink(SHM_NAME);

    exit(0);
}

int main()
{
    signal(SIGINT, cleanup);

    while((mq = mq_open(MQ_NAME, O_RDONLY)) == (mqd_t)-1)
        sleep(1);

    printf("[Processor] Started...\n");

    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(struct sensor_data));

    struct sensor_data *shared = mmap(NULL,
                                      sizeof(struct sensor_data),
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED,
                                      shm_fd,
                                      0);

    shared->temperature = -1;
    shared->humidity    = -1;
    shared->aqi         = -1;

    sem_sp = sem_open(SEM_SP, 0);
    sem_pd = sem_open(SEM_PD, O_CREAT, 0666, 0);

    struct sensor_data data;

    while(1)
    {
        sem_wait(sem_sp);

        mq_receive(mq, (char*)&data, sizeof(data), NULL);

        *shared = data;

        printf("[Processor] Temp=%d Hum=%d AQI=%d\n",
               data.temperature, data.humidity, data.aqi);

        sem_post(sem_pd);
    }
}
