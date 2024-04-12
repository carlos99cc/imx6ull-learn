#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <uchar.h>
#include <unistd.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

int fd;//定义一个文件句柄，用于后续屏幕文件的接收
int fd_hkz16;
struct fb_var_screeninfo var;//创建屏幕驱动代码中一些关于屏幕信息的结构体var
int screen_size;
unsigned char *chmem;//开辟内存中用于储存一帧数据，的首地址
unsigned char *fbmem;
unsigned char *hzkmem;

struct stat hzk_stat;//用于储存fd_hkz16对应文件的状态信息
unsigned int line_width;//一帧数据中，一行像素点的字节大小
unsigned int pixel_width;//一帧数据中，一个像素点的字节大小

/*
点亮一个像素点的代码
点亮为1，变暗为0
*/
void lcd_put_pixel(int x,int y,unsigned int color)
{
    unsigned char *pen_8 = fbmem+y*line_width+x*pixel_width;
    //在fbmen一帧数据首地址的前提下，进行地址的偏移，偏移量为自己设定的下x，y像素点位置
    //这行代码在内存中开辟一个uchar的一个字节大小的空间。
    unsigned short *pen_16;//内存中开辟2个字节大小的无符号整型空间，pen_16存的是地址
    unsigned int *pen_32;//开辟4个字节大小无符号整型空间

    unsigned int red, green, blue;

    pen_16 = (unsigned short *)pen_8;
    pen_32 = (unsigned int *)pen_8;   
    //转换指针类型，用于访问16位深和32位深的像素数据

    switch (var.bits_per_pixel) {
        case 8:  //color有32位大小，这里强制使用低8位，高的位被舍弃
        {
            *pen_8 = color;
            break;
        }
        case 16://color有32位大小，通常高八位表示透明度。低24位表示颜色。红绿蓝各8位
        {
            //像素点颜色信息565（红绿蓝位数）
            red = (color >> 16) & 0xff;//通过移位和掩码取出红色的8位
            green = (color >> 8) & 0xff;//get green 8 bits
            blue = (color >> 0) & 0xff; //get blue 8 bits
            color = ((red >>3) << 11) | ((green >> 2)<<5) | (blue>>3);
            //通过移位，将红色放高5位，绿色中6位，蓝色低5位
            *pen_16 = color;
            break;
        }
        case 32:
        {
            *pen_32 = color;
            break;
        }
        default:
		{
			printf("can't surport %dbpp\n", var.bits_per_pixel);
			break;
		}
    }        
}

void lcd_put_chinese(int x,int y,unsigned char *str)
{
    unsigned int area = str[0] - 0xA1;
    unsigned int position = str[1] - 0xA1;
    unsigned char *dots = hzkmem + (area * 94 +position)*32;//中文字在HZ16中，按照区和位置划分，每个区94个汉字。减去0xa1是为了确定区和位的地点
    //每个汉字由16*16的点阵确定。每个汉字占32个字节。在基地址chmem的前提下，进行地址偏移
    //这样dots的首地址就是（area,position）汉字所对应的地址
    unsigned char byte;

    int i,j,b;
    for ( i = 0; i < 16; i++)
    {
        for ( j = 0; j < 2; j++)
        {
            byte = dots[i*2+j];
            for (b=7; b>=0; b--)
            {
                if (byte & (1<<b))
                {
                    //显示汉字，点亮
                    lcd_put_pixel(x+j*8+7-b, y+i, 0xc71585 );
                }
                else {
                    lcd_put_pixel(x+j*8+7-b, y+i, 0);//显示黑色
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    unsigned char str[]="国";
    fd = open("/dev/fb0", O_RDWR);
    if (fd<0)
    {
        printf("can not open /dev/fb0 \n");
        return -1;
    }
    if (ioctl(fd, FBIOGET_VSCREENINFO,&var ))
    {
        printf("can not get var \n");
        return -1;
    }

    line_width  = var.xres * var.bits_per_pixel / 8;
	pixel_width = var.bits_per_pixel / 8;
	screen_size = var.xres * var.yres * var.bits_per_pixel / 8;
	fbmem = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (fbmem == (unsigned char *)-1)
    {
        printf("can not mmap \n");
        return -1 ;
    }

    fd_hkz16 = open("HZK16", O_RDONLY);
    if (fd_hkz16 < 0   )
    {
        printf("can not open HZK16\n");
        return -1  ;
    }
    if (fstat(fd_hkz16, &hzk_stat))//这是一个系统调用，用于获取文件描述符 fd_hzk16 对应文件的状态信息，并将结果保存在结构体 hzk_stat 中
    {       //如果成功读取到信息并保存到hzk_stat结构体中，就返回0
        printf("can not get fstat\n");
        return -1;
    }
    hzkmem = (unsigned char *)mmap(NULL, hzk_stat.st_size, PROT_READ, MAP_SHARED, fd_hkz16, 0);
    if (hzkmem == (unsigned char *)-1)
    {
        printf("can not mmap for hkz16\n");
        return -1;
    }

    memset(fbmem, 0, screen_size);

    printf("chinese code: %02x %02x\n", str[0], str[1]);
    lcd_put_chinese(var.xres/2, var.yres/2, str);

    munmap(fbmem, screen_size);

    close(fd);

    return 0;
}
