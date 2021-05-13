#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

#include"thread_pool.h"
#define QUEUE_PLUS 4
#define MAX_THREAD 10

typedef struct id{
    thread_pool *p;
    int id;
}id;

static void *thread_entity(void *arg);

thread_pool* thread_pool_create(int thread_num)
{
    if (thread_num <= 0) {   
        printf("Bad thread number.\n");
        return NULL;
    }
    else if (thread_num > MAX_THREAD) {
        printf("Too much thread.\n");
        return NULL;
    }
    
    thread_pool *pool = NULL;
    if ((pool=(thread_pool *)malloc(sizeof(thread_pool))) == NULL) {
        printf("malloc threadpool false.\n");
        return NULL; 
    }

    pool->num_thread = thread_num;
    pool->shutdown = 0;
    pool->front = 0;
    pool->rear = 0;
    pool->size = 0;
    pool->max_size = QUEUE_PLUS * thread_num;
    
    pool->threads = (pthread_t *)malloc(thread_num * sizeof(pthread_t));
    pool->jobs = (job *)malloc(pool->max_size * sizeof(job));
    if (pool->threads == NULL) {
        printf("*threads: Call malloc failed.");
        return NULL;
    }
    if (pool->jobs == NULL) {
        printf("*jobs: Call malloc failed.");
        return NULL;
    }
    memset(pool->threads, 0, thread_num * sizeof(pthread_t));
    memset(pool->jobs, 0, pool->max_size * sizeof(job));

    if (pthread_mutex_init(&(pool->lock), NULL) != 0 ||
        pthread_cond_init(&(pool->todo), NULL) !=0) {
         printf("Init lock or cond false.\n");
         return NULL;
    }

    int alive = 0;
    id id_array[MAX_THREAD] = {0};
    for (int i = 0;i < thread_num;i++){
        id_array[i].id = i;
        id_array[i].p = pool;
        int ret = pthread_create(&(pool->threads[i]), NULL, thread_entity, (void *)&id_array[i]);
        if (ret < 0) {
            printf("Create thread failed.");
            return NULL;
        }
        alive++;
    }

    printf("[Pool] Create %d thread...\n",alive);
    return pool;
}

int thread_pool_add_job(thread_pool *pool, void *(*function)(void *arg), void *arg)
{
    if (NULL == pool) {
        perror("Poniter pool is NULL\n");
        return POINTER_NULL;
    }
    else if(pool->size == pool->max_size) {
        return JOB_FULL;
    }
    else if (1 == pool->shutdown) {
        perror("Pool has shutdown.\n");
        return SHUTDOWN;
    }
    
    pthread_mutex_lock(&(pool->lock));
    
    printf("[Pool] Add job...\n");
    pool->jobs[pool->rear].function = function;
    pool->jobs[pool->rear].arg = arg;
    pool->rear = (pool->rear + 1) % pool->max_size;
    pool->size++;
    pthread_cond_signal(&(pool->todo));

    pthread_mutex_unlock(&(pool->lock));  

    return SUCCESS; 
}

int thread_pool_destroy(thread_pool *pool)
{
    if (NULL == pool) {
        perror("Poniter pool is NULL\n");
        return POINTER_NULL;
    }

    pthread_mutex_lock(&(pool->lock));
    pool->shutdown = 1;
    pthread_mutex_unlock(&(pool->lock));

    printf("[Pool] shutdowning...\n");

    for (int i = 0;i < pool->num_thread;i++) {
        pthread_cond_broadcast(&(pool->todo));  
    }
    
    for (int i = 0;i < pool->num_thread;i++) {
        pthread_join(pool->threads[i], NULL); 
    }
    
    pthread_cond_destroy(&(pool->todo));
    pthread_mutex_destroy(&(pool->lock));
    free(pool->jobs);
    free(pool->threads);

    return SUCCESS;
}

static void *thread_entity(void *arg)
{   
    id *ID = (id *)arg;
    int this = ID->id;
    thread_pool *pool = ID->p;
    printf("[%d] Init...\n",this);

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        while (pool->size == 0 && 0 == pool->shutdown) {
            printf("[%d] is waiting...\n",this);
            pthread_cond_wait(&(pool->todo), &(pool->lock)); 
        }    

        if (1 == pool->shutdown) {
            pthread_mutex_unlock(&(pool->lock));
            break;
        }

        job fun;
        fun.function = pool->jobs[pool->front].function;
        fun.arg = pool->jobs[pool->front].arg;

        pool->jobs[pool->front].function = NULL;
        pool->jobs[pool->front].arg = NULL;

        pool->front = (pool->front+1) % pool->max_size;
        pool->size--;

        pthread_mutex_unlock(&(pool->lock));
        
        printf("[%d] start\n",this);
        (*(fun.function))(fun.arg);
        printf("[%d] finish\n",this);
    }    
    printf("[%d] is exiting...\n",this);
    pthread_exit(NULL);
}

