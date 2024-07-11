#include "linux/cdev.h"
#include "linux/kern_levels.h"
#include "linux/swap.h"
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/backing-dev.h>
#include <linux/shmem_fs.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/uio.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/uaccess.h>

#define test 0

static unsigned char hello_buf[100];
static struct class *hello_class;
static unsigned long copy;

static int hello_open(struct inode *node, struct file *filp)
{
    printk("%s   %s    %d\n",__FILE__,__FUNCTION__,__LINE__);
    return 0;
}

static ssize_t hello_read(struct file *filp, char __user *buf, size_t size, loff_t *offset)
{
    unsigned long len = size > 100 ? 100:size;
    printk("%s   %s    %d\n",__FILE__,__FUNCTION__,__LINE__);
    copy = copy_to_user(buf, hello_buf, len);
    return len;
}

static ssize_t hello_write(struct file *filp, const char __user *buf, size_t size, loff_t * offset)
{
    unsigned long len = size > 100 ? 100:size;
    printk("%s   %s    %d\n",__FILE__,__FUNCTION__,__LINE__);
    copy = copy_from_user(hello_buf,buf,len);
    return len;
}

static int hello_release(struct inode *node, struct file *filp)
{
    printk("%s   %s    %d\n",__FILE__,__FUNCTION__,__LINE__);
    return 0;
}

/*1. creat file_operation struct*/

static const struct file_operations hello_drv = {
	.owner      = THIS_MODULE,
	.read		= hello_read,
	.write		= hello_write,
	.open		= hello_open,
    .release    = hello_release,
};

/*2. register_chrdev function*/

/*3. entry */

#if test

static int major;
//此时创建的主设备号下的所有次设备号都是用这个驱动程序

static int hello_init(void)
{
    major = register_chrdev(0, "100ask_hello", &hello_drv); 
    hello_class = class_create(THIS_MODULE, "hello_class");
	if (IS_ERR(hello_class)) {
		printk("%s: failed to allocate class\n", __func__);
		return PTR_ERR(hello_class);
	}

    device_create(hello_class, NULL, MKDEV(major, 0), NULL,"hello"); //创建一个/dev/hello的设备节点
    return 0;
}


/*4. exit*/
static void hello_exit(void)
{
    device_destroy(hello_class, MKDEV(major, 0));
    class_destroy(hello_class);
    unregister_chrdev(major, "100ask_hello");
}
#endif // DEBUG

#if !test

//此代码可以设置次设备号，并限定设备号的大小。这样一个主设备号的所有次设备号就不必用用一个驱动，不同次设备号可以用不同驱动程序。
static struct cdev hello_cdev;
static dev_t dev; //dev是一个整数。高12位是主设备号

static int hello_init(void)
{

    int rc;

    rc = alloc_chrdev_region(&dev, 0, 1, "hello");
    if (rc < 0)
    {
        printk(KERN_ERR "can't get alloc_chrdev_region number\n");
        return rc;
    }

    cdev_init(&hello_cdev, &hello_drv);
    rc = cdev_add(&hello_cdev, dev, 1);
    if (rc < 0)
    {
        printk(KERN_ERR "can't get cdev_add number\n");
        return rc;
    }

    hello_class = class_create(THIS_MODULE, "hello_class");
    if (IS_ERR(hello_class)) {
    printk("%s: failed to allocate class\n", __func__);
    return PTR_ERR(hello_class);
    }

    device_create(hello_class, NULL, dev, NULL,"hello"); //创建一个/dev/hello的设备节点
    return 0;
}

static void hello_exit(void)
{
    device_destroy(hello_class, dev);
    class_destroy(hello_class);

    unregister_chrdev_region(dev, 1); //释放主设备号的区域
}
#endif // test

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");


