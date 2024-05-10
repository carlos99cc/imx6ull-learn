
#include "unistd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

/*多个进程通过互斥锁来访问同一临界资源*/

#define THREAD_NUMBER 3 //线程数目
#define REPEAT_NUMBER 5 //每个线程的小任务数目
#define DELAT_TIME_LEVELS 10.0 //小任务之间的最大间隔

pthread_mutex_t mutex;  //定义一个互斥锁

/*多个线程的临界资源（线程主函数）*/
void* thrd_func(void* arg)
{
    int thrd_num = (int)(long)arg;
    int delay_time = 0,cout = 0;
    int res;

    /*互斥锁上锁*/
    res = pthread_mutex_lock(&mutex);

    if (res)
    {
        printf("thread %d lock failed\n",thrd_num);
        pthread_exit(NULL);
    }
    printf("thread %d lock is starting\n",thrd_num);

    for (cout = 0; cout < REPEAT_NUMBER; cout++)
    {
        delay_time = (int)(rand()*DELAT_TIME_LEVELS/(RAND_MAX))+1;//产生1-10之间的数
        sleep(delay_time);
        printf("\t thread  %d : job %d delay = %d\n",thrd_num,cout,delay_time);
    }
    printf("thread %d finished \n",thrd_num);
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t thread[THREAD_NUMBER];
    int no = 0,res;
    void* thrd_ret;
    /*这样做的目的是确保每次程序运行时，随机数生成器的初始状态不同，从而产生不同的随机数序列，
    这对于需要重复运行的程序（如模拟、调试或加密）来说是很有用的。如果每次都使用相同的种子，rand() 函数将生成相同的随机数序列。*/
    srand(time(NULL));

    /*互斥锁初始化*/
    pthread_mutex_init(&mutex, NULL);

    for (no = 0; no < THREAD_NUMBER; no++) 
    {
        res = pthread_create(&thread[no], NULL, thrd_func, (void*)(long)no);
        if (res != 0)
        {
            printf("create thread  %d failed\n",no);
            exit(res);
        }
    }
        printf("successfully creat threads \n waitting for threads to finish...\n");
        for (no = 0; no < THREAD_NUMBER;no++)
        {
            res = pthread_join(thread[no], &thrd_ret);
            if (!res )
            {
                printf("thread %d joined \n",no);
            }
            else
            {
                printf("create thread  %d failed\n",no);
            }
            /*互斥锁解锁*/
            pthread_mutex_unlock(&mutex);
        }   
        pthread_mutex_destroy(&mutex);
        return 0;
}