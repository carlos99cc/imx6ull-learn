#include "signal.h"
#include "unistd.h"
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/wait.h>


/*共享内存创建步骤：
1. 创建共享内存，使用shmget（），在进程中创建一个待映射的共享内存区域
2. 映射共享内存。把内核的共享内存映射到具体进程中空间中（刚创建的内存区域）*/

#define BUFFER_SIZE 2048

int main()
{
    pid_t pid;
    int shmid;
    char *shm_addr;
    char flag[] = "WROTE";
    char buff[BUFFER_SIZE];

    /*创建共享内存区域*/
    if ((shmid = shmget(IPC_PRIVATE, BUFFER_SIZE, 0666)) < 0)
    {
        perror("shmget");
        exit(1);
    }
    else
    {
        printf("successfully create shared-memory: %d \n",shmid);
    }

    /*显示共享内存情况*/
    system("ipcs -m");

    pid = fork();
    if(pid == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0) 
    {
        /*映射内核共享内存区域到已创建的内存区域*/
        if ((shm_addr = shmat(shmid, 0, 0)) == (void *)-1)
        {
            perror("child process :shmat");
            exit(1);
        }
        else
        {
            printf("child process :attach shared-memory : %p\n",shm_addr);
        }
        system("ipcs -m");

        /*通过检查在共享内存的头部是否有标志字符串“WROTE”来确认父进程是否已经向共享内存写入有效数据*/
        while (strncmp(shm_addr, flag, strlen(flag)))
        {
            printf("child process: wait for enable data...\n");
            sleep(5);
        }

        /*已经确认有标识符字符串，获取共享内存的有效数据并显示*/
        strcpy(buff, shm_addr + strlen(flag));
        printf("child process: shared-memory : %s\n",buff);

        /*解除共享内存的映射*/
        if ((shmdt(shm_addr)) < 0)
        {
            perror("shmdt");
            exit(1);
        }
        else
        {
            printf("child process : deattach shared-memory\n");
        }
        system("ipcs -m");

        /*删除共享内存*/
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
        {
            perror("child process: shmctl(IPC_RMID)\n");
            exit(1);
        }
        else
        {
            printf("delete shared-memory \n");
        }
        system("ipcs -m");
    }

    /*父进程内容处理*/
    else
    {
        /*映射共享内存*/
        if ((shm_addr = shmat(shmid, 0, 0)) == (void *)-1)
        {
            perror("father process :shmat");
            exit(1);
        }
        else
        {
            printf("father process :attach shared-memory : %p\n",shm_addr);
        }

        sleep(1);

        printf("\n input some string :\n");
        fgets(buff, BUFFER_SIZE, stdin);
        strncpy(shm_addr + strlen(flag), buff, strlen(buff));
        strncpy(shm_addr, flag, strlen(flag));

        /*解除共享内存的映射*/
        if ((shmdt(shm_addr)) < 0)
        {
            perror("shmdt");
            exit(1);
        }
        else
        {
            printf("father process : deattach shared-memory\n");
        }
        system("ipcs -m");

        waitpid(pid, NULL, 0);
        printf("finished\n");
        
    }
    exit(0);
}