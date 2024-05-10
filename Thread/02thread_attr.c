#include "unistd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

/*创建一个线程。具有绑定和分离属性。而且主进程通过标志变量获得线程结束的消息*/

#define REPEAT_NUMBER 3 //线程中的小任务数
#define DELAY_TIME_LEVELS 10.0 //小任务之间的最大时间间隔

int finish_flag = 0;

void* thrd_func(void* arg)
{
    int delay_time = 0;
    int count = 0;
    int thrd_num = (int)(long)arg;
    printf("thread %d is starting:\n",thrd_num);

    /*小任务开始：*/
    for (count = 0; count < REPEAT_NUMBER; count++)
    {
        delay_time = (int)(rand()*DELAY_TIME_LEVELS/(RAND_MAX))+1;
        sleep(delay_time);
        printf("\t thread %d : job %d delay = %d \n",thrd_num,count,delay_time);
    }

    printf("thread %d finished \n",thrd_num);
    finish_flag = 1;
    pthread_exit(NULL);
}

int main()
{
    pthread_t thrd;
    pthread_attr_t attr;
    int no = 0, res;
    void* thrd_res;

    srand(time(NULL));
    /*初始化线程的属性对象*/
    res = pthread_attr_init(&attr);
    if (res != 0)
    {
        printf("create attribute failed\n");
        exit(res);
    }

    /*设置线程绑定属性*/
    res = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    /*设置线程分离属性*/
    res += pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (res != 0)
    {
        printf("setting attribute failed \n");
        exit(res);
    }

    /*创建线程*/
    res = pthread_create(&thrd, &attr, thrd_func,(void* )(long)no);
    if (res != 0)
    {
        printf("create thread failed \n");
        exit(res);
    }

    /*释放线程属性对象*/
    pthread_attr_destroy(&attr);
    printf("create thread succeed\n");

    while (!finish_flag)
    {
        printf("waitting for thread to finish...\n");
        sleep(2);
    }
    return 0;
}