#include <fcntl.h>
#include "asm-generic/errno-base.h"
#include "sys/stat.h"
#include <errno.h>
#include <error.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <semaphore.h>

/*两个线程一边生产一边消费。缓冲区域（临界资源）3个单元。生产和消费速度自行拟定。*/

#define MYFIFO "myfifo"  //缓冲区有名管道的名字
#define BUFFER_SIZE 3 //缓冲区的单元数
#define UNIT_SIZE 5 //缓冲区每个单元的字节大小
#define RUN_TIME 30 //运行时间
#define DELAY_TIME_LEVELS 5.0 //周期的最大值

int fd;
time_t end_time;
sem_t mutex, full, avail; //3个信号量

/*生产者线程*/
void* producer(void* arg)
{
    int real_write;
    int delay_time = 0;

    while (time(NULL) < end_time)
    {
        delay_time = (int)(rand()*DELAY_TIME_LEVELS/(RAND_MAX) )+1; //delay_time的大小为：1或2
        sleep(delay_time);

        /*p操作信号量avail和mutex*/
        sem_wait(&avail);
        sem_wait(&mutex);
        printf("\n producer: delay = %d\n",delay_time);

        /*生产者写入数据*/
        if ((real_write = write(fd, "hello", UNIT_SIZE)) == -1)
        {
            if (errno == EAGAIN)
            {
                printf("the FIFO has not been read yet.please try later\n");
            }
        }
        else
        {
            printf("write %d to the FIFO \n",real_write);
        }

            /*v操作信号量 full和mutex*/
            sem_post(&full);
            sem_post(&mutex);    
    }
    pthread_exit(NULL);
}

/*消费者线程*/
void* customer(void* arg)
{
    unsigned char read_buffer[UNIT_SIZE];
    int real_read;
    int delay_time;

    while (time(NULL)<end_time)
    {
        delay_time = (int)(rand()*DELAY_TIME_LEVELS/(RAND_MAX) )+1; //delay_time的大小为：1或2
        sleep(delay_time);
        /*p操作信号量full和mutex*/
        sem_wait(&full);
        sem_wait(&mutex);
        memset(read_buffer, 0, UNIT_SIZE);
        printf("\n customer: delay_time = %d\n",delay_time);

        if ((real_read = read(fd, read_buffer, UNIT_SIZE)) == -1)   
        {
            if (errno == EAGAIN)
            {
                printf("no data yet\n");
            }
        }
        printf("read %s from FIFO\n",read_buffer);

        /*v操作信号量avail和mutex*/
        sem_post(&avail);
        sem_post(&mutex);

    }
    pthread_exit(NULL);
}

int main()
{
    pthread_t thrd_prd_id,thrd_cst_id;
    // pthread_t mon_th_id;
    int ret;

    srand(time(NULL));
    end_time = time(NULL) + RUN_TIME;

    /*创建有名管道*/
    if ((mkfifo(MYFIFO, 0666|O_CREAT|O_EXCL) < 0) &&(errno != EEXIST))
    {
        printf("cannot creat  fifo\n");
        return errno;
    }

    /*打开管道*/
    fd = open(MYFIFO, O_RDWR);
    if (fd == -1)
    {
        printf("open fifo error!\n");
        return fd;
    }

    /*初始化互斥信号量mutex为1*/
    ret = sem_init(&mutex, 0, 1);
    /*初始化信号量avail为N(单元个数)*/
    ret += sem_init(&avail, 0, BUFFER_SIZE);
    /*初始化full信号量为0*/
    ret += sem_init(&full, 0, 0);
    
    if (ret != 0)
    {
        printf("any semaphore initialization failed\n");
        return ret;
    }

    /*创建两个线程*/
    ret = pthread_create(&thrd_prd_id, NULL, producer, NULL);
    if (ret != 0)
    {
        printf("create producer thread error\n");
        return ret;
    }

    ret = pthread_create(&thrd_cst_id, NULL, customer, NULL);
    if (ret != 0)
    {
        printf("create customer thread error\n");
        return ret;
    }

    pthread_join(thrd_prd_id, NULL);
    pthread_join(thrd_cst_id, NULL);
    close(fd);
    unlink(MYFIFO);
    return 0;
}