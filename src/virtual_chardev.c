#include "virtual_chardev.h"




static unsigned int cbuf_size = DEFAULT_CBUF_SIZE;
module_param(cbuf_size, uint, S_IRUGO);

static struct file_operations vchardev_fops =
{
    .owner =   THIS_MODULE,
    .open =    vchardev_open,
    .release = vchardev_release,
    .read =    vchardev_read,
    .write =   vchardev_write,
};


static int __init vchardev_initialize(void)
{
    printk(KERN_WARNING "Initializing virtual character device module...\n");
    printk(KERN_DEBUG "Circular buffer size parameter: %d\n", cbuf_size);

    return 0;
}

static void __exit vchardev_cleanup(void)
{
    printk(KERN_WARNING "Cleaning everything up and shutting down...\n");
}


static int vchardev_open(struct inode *inode, struct file *filp)
{
    printk(KERN_DEBUG "Someone is trying to open the device file.\n");

    return 0;
}

static int vchardev_release(struct inode *inode, struct file *filp)
{
    printk(KERN_DEBUG "Someone is trying to close the device file.\n");

    return 0;
}

static ssize_t vchardev_read(struct file *filp, char *buffer, size_t len,
                             loff_t *offset)
{
    printk(KERN_DEBUG "Someone is trying to read from the device file.\n");

    return 0;
}

static ssize_t vchardev_write(struct file *filp, const char *buffer, size_t len,
                              loff_t *offset)
{
    printk(KERN_DEBUG "Someone is trying to write to the device file.\n");

    return 0;
}


module_init(vchardev_initialize);
module_exit(vchardev_cleanup);

MODULE_LICENSE(VCHARDEV_LICENSE);
MODULE_AUTHOR(VCHARDEV_AUTHOR);
MODULE_DESCRIPTION(VCHARDEV_DESC);
MODULE_VERSION(VCHARDEV_VERSION);
