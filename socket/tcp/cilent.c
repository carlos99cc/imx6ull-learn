#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

/*客户端步骤：
1.socket 创建套字节
2. connect  创建一个连接
3. send/recv  发送 接收数据
*/
#define SERVER_PORT 8888

int main(int argc, char **argv)
{

    int iSocketClient;
    int iRet;
    int iSendLen;
    socklen_t iAddrLen;
    char cSendBuf[1000];
    struct sockaddr_in tSocketServerAddr;

    if (argc != 2)
	{
		printf("Usage:\n");
		printf("%s <server_ip>\n", argv[0]);
		return -1;
	}

    iSocketClient = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == iSocketClient)
    {
        printf("socket error!\n");
        return -1;
    }
    //发送连接前，需要确定ipv4协议，传输的端口号，以及与谁连接（ip地址）
    tSocketServerAddr.sin_family = AF_INET;//ipv4协议
    tSocketServerAddr.sin_port = htons(SERVER_PORT);
    /*inet_aton()函数的作用是将一个IPv4地址的字符串表示法转换为网络地址（in_addr结构体），
    并将结果存储在指定的结构体中。*/
    if (0 == inet_aton(argv[1], &tSocketServerAddr.sin_addr))
 	{
		printf("invalid server_ip\n");
		return -1;
	}
    memset(tSocketServerAddr.sin_zero, 0, 8);

    iAddrLen = sizeof(struct sockaddr);
    //发送一个连接
    iRet = connect(iSocketClient, (const struct sockaddr *)&tSocketServerAddr,iAddrLen);
    if (-1 == iRet)
    {
        printf("connect error!\n");
        return -1;
    }
    while (1)
    {
        /*从标准输入（stdin）读取用户输入的字符串cSendBuf
        若成功读取，返回一个非零值*/
        if (fgets(cSendBuf, 999,stdin))
        {
            iSendLen = send(iSocketClient, cSendBuf, strlen(cSendBuf), 0);
            if (iSendLen <= 0)
			{
				close(iSocketClient);
				return -1;
			}
        }
    }
     return 0;
}