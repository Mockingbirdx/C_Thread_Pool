#include<stdio.h>
#include <unistd.h>
#include"thread_pool.h"

void task(int *val) 
{
    for(int i = 0;i < 3;i++){
        printf("---%d---\n",*val);
        sleep(1);
    }
}

int main(int argc, char const *argv[])
{   
    int num = 0;
    int r = scanf("%d",&num);
    thread_pool *pool = thread_pool_create(num);
    if (pool == NULL) {
        return 0;
    }
    int array[10] = {0};
    for (int i = 0;i < 4;i++) {
        array[i] = i;
        int ret = thread_pool_add_job(pool, (void *)&task,(void *)&array[i]);
        while (ret == JOB_FULL) {   
            sleep(5);
            ret = thread_pool_add_job(pool, (void *)&task,(void *)&i);
        }
        sleep(1);
    }
    thread_pool_destroy(pool);
    return 0;
}
