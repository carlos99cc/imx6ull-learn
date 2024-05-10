#include <stdlib.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include "sys/types.h"
#include "unistd.h"

/*消息队列的接收*/

#define BUFFER_SIZE 512

struct msgbuf 
    {
        long mtype;       /* message type, must be > 0 */
        char mtext[BUFFER_SIZE];    /* message data */
    };

int main()
{
    int qid;
    key_t key;
    struct msgbuf msg;

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

    do
    {
        memset(msg.mtext, 0, BUFFER_SIZE);
        if (msgrcv(qid, (void*)&msg, BUFFER_SIZE, 0, 0) < 0)
        {
            perror("msgrcv:");
            exit(1);
        }
        printf("the message from process %ld : %s",msg.mtype,msg.mtext);
    }while(strncmp(msg.mtext, "quit", 4));

    /*从系统的内核中移走消息队列*/
    if (msgctl(qid, IPC_RMID, NULL))
    {
        perror("msgctl:");
        exit(1);
    }
    exit(0);
}