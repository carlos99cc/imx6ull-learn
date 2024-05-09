#include "sys/types.h"
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <unistd.h>

#define DELAY_TIME 3
/*用信号量来控制两个（父子）进程间的执行顺序*/

/*信号量编程步骤：*/
/*1. 创建信号量或获取在系统中已经存在的信号量
  2. 初始化信号量。使用semctl函数的SETVAL操作。
  3. 对信号量进行pv操作。p：减量，占用一个资源。v:增量，释放一个资源
  4. 如果不需要信号量，从系统中删除信号量
*/
union semum
{
  int val;
  struct semid_ds *buf;
  unsigned short *array;
};

/*信号量初始化信号量函数*/
int init_sem(int sem_id,int init_value)
{
  union semum sem_union;
  sem_union.val = init_value;
  if (semctl(sem_id, 0, SETVAL,sem_union) == -1)
  {
    perror("Initialize\n");
    return -1;
  }
  return 0;
}

/*从系统中删除信号量函数*/
int del_sem(int sem_id)
{
  union semum sem_union;
  if (semctl(sem_id, 0, IPC_RMID,sem_union) == -1)
  {
    perror("Delete semaphore\n");
    return -1;
  }
  return 0;
}

/*p操作函数*/
int sem_p(int sem_id)
{
  struct sembuf sem_b;
  sem_b.sem_num = 0; //信号量的编号。使用单个信号量时，通常取值0
  sem_b.sem_op = -1;//p操作
  sem_b.sem_flg = SEM_UNDO;//系统自动释放将会在系统中残留的信号量
  if (semop(sem_id, &sem_b, 1) == -1) //nsops:操作数组中操作的个数。通常取1
  {
    perror("p operation\n");
    return -1;
  }
  return 0;
}

/*v操作函数*/
int sem_v(int sem_id)
{
  struct sembuf sem_b;
  sem_b.sem_num = 0; //信号量的编号。使用单个信号量时，通常取值0
  sem_b.sem_op = 1;//v操作
  sem_b.sem_flg = SEM_UNDO;//系统自动释放将会在系统中残留的信号量
  if (semop(sem_id, &sem_b, 1) == -1) //nsops:操作数组中操作的个数。通常取1
  {
    perror("v operation\n");
    return -1;
  }
  return 0;
}

int main(int argc,char** argv)
{

  pid_t result;
  int sem_id;

  sem_id = semget(ftok(".",'a'), 1, 0666|IPC_CREAT);//nsems:需要创建的信号量数目。通常取1
  init_sem(sem_id, 0);//初始化信号量为0

  /*创建子进程*/
  result = fork();
  if (result == -1)
  {
    perror("fork\n");
    return -1;
  }
  else if (result == 0)
  {
    /*子进程*/
    printf("child process will wait for some seconds...\n");
    sleep(DELAY_TIME);
    printf("the returned value is %d in the child process(PID = %d)\n",result,getpid());
    sem_v(sem_id);//子进程释放资源，信号量+1
  }
  /*父进程*/
  else
  {
    sem_p(sem_id);//父进程占用一个资源。信号量-1。但是若初始化的信号量为0，进程被阻塞一直到有资源为止（信号量>0）
    printf("the returned value is %d in the father process(PID = %d) \n",result,getpid());
    sem_v(sem_id);
    del_sem(sem_id);
  }
  
  exit(0);
  return 0;
}
 
//while(waitpid(-1, NULL, WNOHANG)> 0);//一键回收父进程中的所有僵尸子进程资源