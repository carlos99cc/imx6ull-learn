#include <stdlib.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include "unistd.h"
/*消息队列发送端进程步骤
1. 创建消息队列
2. 输入信息
2. 添加消息队列*/

#define BUFFER_SIZE 512

struct msgbuf 
    {
        long mtype;       /* message type, must be > 0 */
        char mtext[BUFFER_SIZE];    /* message data */
    };

int main()
{
    struct msgbuf msg;
    int qid;
    key_t key;

    /*创建不同路径和关键字产生的key*/
    if ((key = ftok(".", 'a')) == -1)
    {
        perror("ftok");
        exit(1);
    }

    /*创建消息队列*/
    qid = msgget(key, 0666|IPC_CREAT);
    if (qid == -1 )
    {
        perror("msgget");
        exit(1);
    }
    printf("open queue: %d\n",qid);

    /*输入消息*/
    while (1)
    {
        printf("please input some message:\n");   
        if (fgets(msg.mtext,BUFFER_SIZE,stdin) == NULL)
        {
            puts("no message!");
            exit(1);
        }
        msg.mtype = getpid();

        /*添加消息队列*/
        if ((msgsnd(qid, &msg, BUFFER_SIZE, 0)) < 0)
        {
            perror("msgsnd:");
            exit(1);
        }
        
        if (strncmp(msg.mtext, "quit", 4) == 0)
        {
            break;
        }
    }
    exit(0);
    

}

