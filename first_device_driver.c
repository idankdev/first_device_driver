#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h> 

#define DEVICE_NAME "first_device_driver"
#define DEVICE_CLASS "device_class"
#define mem_size        256


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Idan");
MODULE_DESCRIPTION("Hello World Device Driver");
MODULE_VERSION("1.0");

static dev_t dev = 0;
static struct cdev cdev;
static struct class *device_class;
static uint8_t *kernel_buffer;
uint8_t ret = 0;

static int __init hello_world_init(void);
static void __exit hello_world_exit(void);
static int      open(struct inode *inode, struct file *file);
static int      release(struct inode *inode, struct file *file);
static ssize_t  read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  write(struct file *filp, const char *buf, size_t len, loff_t * off);

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = read,
        .write          = write,
        .open           = open,
        .release        = release,
};
 
static int open(struct inode *inode, struct file *file)
{
        if((kernel_buffer = kmalloc(mem_size , GFP_KERNEL)) == 0){
            printk(KERN_INFO "Unable to allocate memory in kernel\n");
            return -1;
        }
        printk(KERN_INFO "Device File Opened!\n");
        return 0;
}
 
static int release(struct inode *inode, struct file *file)
{
        kfree(kernel_buffer);
        printk(KERN_INFO "Device File Closed!\n");
        return 0;
}
 
static ssize_t read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        copy_to_user(buf, kernel_buffer, mem_size);
        printk(KERN_INFO "Read syscall was invoked!\n");
        return mem_size;
}
 
static ssize_t write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        copy_from_user(kernel_buffer, buf, len);
        printk(KERN_INFO "Write syscall was invoked!\n");
        return len;
}


static int __init hello_world_init(void)
{
        if((alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME)) <0){
                printk(KERN_INFO "Failed allocating major number for device!\n");
                return -1;
        }
        printk(KERN_INFO "Major number = %d Minor number = %d \n",MAJOR(dev), MINOR(dev));

        cdev_init(&cdev,&fops);

        ret =  cdev_add(&cdev,dev,1);
        if(ret < 0){
            printk(KERN_INFO "Failed adding the device to the system!\n");
            unregister_chrdev_region(dev,1);
            return ret;
        }

        device_class = class_create(THIS_MODULE,DEVICE_CLASS);
        if(device_class == NULL){
            printk(KERN_INFO "Failed creating the struct class for device!\n");
            unregister_chrdev_region(dev,1);
            return -1;
        }

        if(device_create(device_class,NULL,dev,NULL,DEVICE_NAME) == NULL){
            printk(KERN_INFO "Failed creating the Device!\n");
            class_destroy(device_class);
            unregister_chrdev_region(dev,1);
            return -1;
        }

        printk(KERN_INFO "Kernel Module - Inserted!\n");
        return 0;
}
 
static void __exit hello_world_exit(void)
{
        device_destroy(device_class,dev);
        class_destroy(device_class);
        cdev_del(&cdev);
        unregister_chrdev_region(dev, 1);
        printk(KERN_INFO "Kernel Module - Removed!\n");
}
 
module_init(hello_world_init);
module_exit(hello_world_exit);
 
