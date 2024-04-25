#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>



/*服务端代码流程：
1.socket 读取网络文件，返回fd文件句柄
2.bind  绑定自己的IP和端口
3.listen 监听自己的端口是否有数据传输
4.accept  接收一条连接线路
5.send/receive  开始发送或接收数据*/

#define SERVER_PORT 8888
#define BACKLOG     10

int main(int argc, char **argv)
{
    int iSocketServer;
    int iSocketClient;
    int iRet;
    int iClientNum = -1;
    socklen_t iAddrLen;
    struct sockaddr_in tSocketServerAddr;
    struct sockaddr_in tSocketClientAddr;

    int iRecvLen;
    unsigned char ucRecvBuf[1000];

    iSocketServer = socket(AF_INET,  SOCK_STREAM, 0);
    if (-1 == iSocketServer)
    {
        printf("socket error!\n");
        return -1;
    }

    tSocketServerAddr.sin_family = AF_INET;
    tSocketServerAddr.sin_port = htons(SERVER_PORT);//host to net, short
    tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;//监测本机电脑所有ip
    memset(tSocketServerAddr.sin_zero, 0, 8);//用于将一块内存区域的内容全部设置为指定的值。

    iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
    if (-1 == iRet)
    {
        printf("bind error!\n");
        return -1;
    }

    iRet = listen(iSocketServer, BACKLOG);//同时监听10路数据
    if (-1 == iRet)
	{
		printf("listen error!\n");
		return -1;
	}
    while (1)
    {
        //将客户端的ip地址保存在结构体tSocketClientAddr中
        iAddrLen = sizeof(struct sockaddr);
        iSocketClient = accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, &iAddrLen);
        if (-1 != iSocketClient)
        {
            iClientNum++;
            printf("Get connect from client %d : %s\n",  iClientNum, inet_ntoa(tSocketClientAddr.sin_addr));
            if (!fork())//创建一个进程（子进程）
            {
                /*子进程源码*/
                while (1)
                {
                    /*接收客户端发过来的数据并显示出来
                    从iSocketClient的客户端读到的数据储存到ucRecvBuf中
                    最多读取999个数据*/
                    iRecvLen = recv(iSocketClient, ucRecvBuf, 999, 0);
                    if (iRecvLen <= 0)
                    {
                        close(iSocketClient);
                        return -1;
                    }
                    else
                    {
                        ucRecvBuf[iRecvLen] = '\0';
                        printf("Get Msg From Client %d: %s\n", iClientNum, ucRecvBuf);
                    }
                }
            }
        }
    }
    close(iSocketServer);
    return 0;
}