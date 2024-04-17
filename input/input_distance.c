#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

#include <linux/input.h>
#include <sys/ioctl.h>
#include <tslib.h>

int distance(struct ts_sample_mt *point1, struct ts_sample_mt *point2)
{
	int x = point1->x - point2->x;
	int y = point1->y - point2->y;

	return x*x + y*y;
}

int main(int argc, char **argv)
{
    int ret;
    int i;
    int touch_cnt = 0;//定义出触点个数
    int max_slots;
    int point_pressed[20];
    struct tsdev *ts;
    struct ts_sample_mt **pre_samp_mt;//上一次数据
    struct ts_sample_mt **samp_mt;
    //通常是一个指向多触点样本数组的指针的指针，用于存储来自触摸屏的多个触点的数据。
    struct input_absinfo slot;
    /*这是一个用于描述单个输入轴（如触摸屏的 X 轴或 Y 轴）的绝对轴信息的结构体。
    它包含了关于轴的最小值、最大值、当前值、噪声消除参数（fuzz）和平坦区域（flat）
    等信息。这个结构体通常用于获取或设置触摸屏校准参数。*/
  

    ts = ts_setup(NULL, 0);
    /*ts_setup函数用于初始化触摸屏设备，并返回一个指向触摸屏设备结构体的指针。
    在这行代码中，将NULL传递给ts_setup函数表示不传递任何特定的参数，即：由tslib自己扫描屏幕设备，而不是自己指定
    而将0传递给ts_setup函数可能表示阻塞方式。
    通过这行代码，您可以获取一个指向初始化后的触摸屏设备结构体的指针，后续可以使用这个指针来操作触摸屏设备，
    比如读取触摸屏的坐标信息、设置触摸屏的参数等。
    */
    if (!ts)
    {
        printf("ts_setup err\n");
        return -1;
    }

    if (ioctl(ts_fd(ts), EVIOCGABS(ABS_MT_SLOT), &slot) < 0) {
		perror("ioctl EVIOGABS");
		ts_close(ts);
		return errno;
	}
    max_slots = slot.maximum + 1 - slot.minimum;
    /*如果 slot.minimum 是 0，slot.maximum 是 9，那么 max_slots 将是 10，
    如果 max_slots 的值为10，最多支持max_slots个点触摸*/

    samp_mt = malloc(sizeof(struct ts_sample_mt *));
	if (!samp_mt) {
		ts_close(ts);
		return -ENOMEM;
	}
	samp_mt[0] = calloc(max_slots, sizeof(struct ts_sample_mt));
	if (!samp_mt[0]) {
		free(samp_mt);
		ts_close(ts);
		return -ENOMEM;
	}

	pre_samp_mt = malloc(sizeof(struct ts_sample_mt *));
	if (!pre_samp_mt) {
		ts_close(ts);
		return -ENOMEM;
	}
	pre_samp_mt[0] = calloc(max_slots, sizeof(struct ts_sample_mt));
	if (!pre_samp_mt[0]) {
		free(pre_samp_mt);
		ts_close(ts);
		return -ENOMEM;
	}

	for (i = 0; i < max_slots; i++)//每个触控点最多有max_slots大小
		pre_samp_mt[0][i].valid = 0;//表示访问第一个样本的第i个触控点

    while (1) {
        ret = ts_read_mt(ts, samp_mt, max_slots, 1);

        if (ret < 0) {
			printf("ts_read_mt err \n");
			ts_close(ts);
            return -1;
		}

        for (i = 0; i < max_slots; i++)
		{
			if (samp_mt[0][i].valid)
			{//samp_mt[0][i]新数据
				memcpy(&pre_samp_mt[0][i], &samp_mt[0][i], sizeof(struct ts_sample_mt));
			}
		}

        /*如果当前触点没被松开，并且数据发生改变
        判断有多少个触点*/
        touch_cnt = 0;
		for (i = 0; i < max_slots; i++)
		{
			if (pre_samp_mt[0][i].valid && pre_samp_mt[0][i].tracking_id != -1)
				point_pressed[touch_cnt++] = i;//方便后面计算第一 第二触点的位置
		}

		if (touch_cnt == 2)
		{
			printf("distance: %08d\n", distance(&pre_samp_mt[0][point_pressed[0]], &pre_samp_mt[0][point_pressed[1]]));
		}
    }

    return 0;
}
