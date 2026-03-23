#ifndef ENV_FINAL_H
#define ENV_FINAL_H

#define MQ_NAME   "/env_mq_final"
#define SHM_NAME  "/env_shm_final"

#define SEM_SP "/sem_sp_final"
#define SEM_PD "/sem_pd_final"
#define SEM_DS "/sem_ds_final"
#define SEM_PS "/sem_ps_final"   // order control

struct sensor_data
{
    int temperature;
    int humidity;
    int aqi;
};

#endif
