#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <unistd.h>
#include <poll.h>


/*./get_info  /dev/input/event1
*/


int main(int argc, char **argv)
{
    int fd;
    int err;
    int len;
    int ret;
    int bit;
    int i;
    nfds_t nfds = 1;
    unsigned char byte;
    unsigned int evbit[2];
    struct pollfd *fds;
    struct input_event event;
    struct input_id id;
    char *ev_names[] = {
        "EV_SYN	",		
        "EV_KEY	",	
        "EV_REL	",		
        "EV_ABS	",		
        "EV_MSC	",		
        "EV_SW	",	    
        "NULL",			
        "NULL",			
        "NULL",			
        "NULL",			
        "NULL",			
        "NULL",			
        "NULL",			
        "NULL",			
        "NULL",			
        "NULL",			
        "NULL",			
        "EV_LED	",		
        "EV_SND	",		
        "NULL",			
        "EV_REP	",		
        "EV_FF	",		
        "EV_PWR	",		
        "EV_FF_STTUS "
    };

    if (argc < 2)
    {
        printf("Usage: %s <dev> [nonblock]\n",argv[0]);
        return -1;
    }
    fd = open(argv[1], O_RDWR | O_NONBLOCK);
    if (fd < 0)
    {
        printf("open %s erro\n",argv[1]);
        return -1;
    }
    err = ioctl(fd, EVIOCGID,&id);//ioctl() 调用用于获取输入设备的事件类型信息
    if (err == 0)
    {
        printf("bustype = 0x%x\n",id.bustype);
        printf("vendor = 0x%x\n",id.vendor);
        printf("product = 0x%x\n",id.product);
        printf("version = 0x%x\n",id.version);
    }
    len = ioctl(fd, EVIOCGBIT(0,sizeof(evbit)), &evbit);//返回值是读到多少数据
    if (len > 0 && len <= sizeof(evbit))//表明读到了数据
    {
        printf("already support event :  ");
        for (i=0; i<len; i++)
        {
            byte = ((unsigned char *)evbit)[i];//将evbit这个整型数组（8个字节内容） 强制转化成uchar *型，evbit是首地址，所以转换的时候用uchar*，现在每个evbit【i】都是uchar，有8个。这里取出第i个
            for (bit = 0; bit < 8; bit++)
			{
				if (byte & (1<<bit)) {
					printf("%s ", ev_names[i*8 + bit]);
				}
			}
        }
        printf("\n");
        
    }
    while (1) {
        fds->fd = fd;
        fds->events = POLLIN;
        fds->revents = 0;
        ret = poll(fds, nfds, 5000);
        if (ret > 0)
        {
            // len = read(fd, &event, sizeof(event));
            // if (len == sizeof(event))
            // {
            //     printf("get event : type: 0x%x, code : 0x%x, value : 0x%x\n",event.type,event.code,event.value);
            // }
            while (read(fd, &event, sizeof(event)) == sizeof(event))//一次poll多次读
            {
                printf("get event : type: 0x%x, code : 0x%x, value : 0x%x\n",event.type,event.code,event.value);
            }
        }

        else if(ret == 0)
        {
            printf("time out\n");
        }
        else
        {
            printf("poll err \n");
        }
   }
    return 0;
}