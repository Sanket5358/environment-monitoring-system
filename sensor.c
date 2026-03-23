#include <stdio.h>
#include <stdlib.h>   // ✅ FIX ADDED
#include <unistd.h>
#include <mqueue.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include "env_final.h"

mqd_t mq;
sem_t *sem_sp, *sem_ps;

void cleanup(int sig)
{
    printf("\n[Sensor] Cleaning up...\n");

    mq_close(mq);
    mq_unlink(MQ_NAME);

    sem_close(sem_sp);
    sem_close(sem_ps);

    sem_unlink(SEM_SP);
    sem_unlink(SEM_PS);

    exit(0);
}

int main()
{
    signal(SIGINT, cleanup);

    struct mq_attr attr = {0, 10, sizeof(struct sensor_data), 0};

    mq = mq_open(MQ_NAME, O_CREAT | O_WRONLY, 0666, &attr);

    sem_sp = sem_open(SEM_SP, O_CREAT, 0666, 0);
    sem_ps = sem_open(SEM_PS, O_CREAT, 0666, 0);

    srand(42);

    struct sensor_data data;
    printf("[Sensor] Started...\n");

    sem_post(sem_ps);

    while(1)
    {
        sem_wait(sem_ps);

        data.temperature = rand() % 60;
        data.humidity    = rand() % 100;
        data.aqi         = rand() % 350;

        mq_send(mq, (char*)&data, sizeof(data), 0);

        printf("[Sensor] Temp=%d Hum=%d AQI=%d\n",
               data.temperature, data.humidity, data.aqi);

        sem_post(sem_sp);
    }
}
