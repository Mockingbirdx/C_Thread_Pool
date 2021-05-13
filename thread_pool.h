#include<pthread.h>

#ifndef _THREAD_POOL_
#define _THREAD_POOL_

enum STATUS{
    SUCCESS = 1,
    JOB_FULL = -1,
    POINTER_NULL = -2,
    SHUTDOWN = -3
};

typedef struct{
    void *(*function)(void *);
    void *arg;
}job;

typedef struct{
    pthread_cond_t todo;
    pthread_mutex_t lock;

    job *jobs;
    pthread_t *threads;

    int front;
    int rear;
    int max_size;
    int size;

    int num_thread;
    int shutdown;
}thread_pool;

//Max number of threads is 10
thread_pool* thread_pool_create(int thread_num);

//be careful to *arg, certify it's address is safe 
int thread_pool_add_job(thread_pool *pool, void *(*function)(void *arg), void *arg);

int thread_pool_destroy(thread_pool *pool);

#endif