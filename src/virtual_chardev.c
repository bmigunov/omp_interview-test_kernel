#include "virtual_chardev.h"




static unsigned int cbuf_size = DEFAULT_CBUF_SIZE;
module_param(cbuf_size, uint, S_IRUGO);
static dev_t dev;

static struct vchardev vchardev_glob =
{
    .cbuf = NULL,
};
static struct file_operations vchardev_fops =
{
    .owner =   THIS_MODULE,
    .open =    vchardev_open,
    .release = vchardev_release,
    .read =    vchardev_read,
    .write =   vchardev_write,
};
static struct cdev this_cdev;


static int __init vchardev_initialize(void)
{
    int result;

    printk(KERN_WARNING "Initializing virtual character device module...\n");
    printk(KERN_DEBUG "Circular buffer size parameter: %d\nAllocating memory\n",
           cbuf_size);
    vchardev_glob.cbuf = kmalloc(cbuf_size, GFP_KERNEL);
    memset(vchardev_glob.cbuf, 0, cbuf_size);

    printk(KERN_DEBUG "Allocating device numbers\n");
    result = alloc_chrdev_region(&dev, 0, 1, VCHARDEV_NAME);
    if(result < 0)
    {
        printk(KERN_ERR "Unable to allocate device numbers\n");
        return result;
    }
    printk(KERN_DEBUG "Major number: %d;\nMinor number: %d\n", MAJOR(dev),
           MINOR(dev));

    printk(KERN_DEBUG "cdev initialization\n");
    cdev_init(&this_cdev, &vchardev_fops);
    this_cdev.owner = THIS_MODULE;
    this_cdev.ops = &vchardev_fops;
    printk(KERN_DEBUG "cdev addition\n");
    result = cdev_add(&this_cdev, dev, 1);
    if(result)
    {
        printk(KERN_ERR "Error %d adding cdev\n", result);
    }

    return 0;
}

static void __exit vchardev_cleanup(void)
{
    printk(KERN_WARNING "Cleaning everything up and shutting down...\n");

    printk(KERN_DEBUG "Deleting cdev\n");
    cdev_del(&this_cdev);

    printk(KERN_DEBUG "Unregistering device numbers\n");
    unregister_chrdev_region(dev, 1);

    printk(KERN_DEBUG "Freeing allocated buffer\n");
    kfree(vchardev_glob.cbuf);
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

static ssize_t vchardev_read(struct file *filp, char __user *buffer, size_t len,
                             loff_t *offset)
{
    printk(KERN_DEBUG "Someone is trying to read from the device file.\n");

    if(*offset > cbuf_size)
    {
        return 0;
    }
    if(*offset + len > cbuf_size)
    {
        len = cbuf_size - *offset;
    }

    printk(KERN_DEBUG "Copying data to user\n");
    if(copy_to_user(buffer, vchardev_glob.cbuf, len))
    {
        printk(KERN_ERR "READ FAILURE\n");
        return -EFAULT;
    }

    *offset += len;
    return len;
}

static ssize_t vchardev_write(struct file *filp, const char __user *buffer,
                              size_t len, loff_t *offset)
{
    printk(KERN_DEBUG "Someone is trying to write to the device file.\n");

    printk(KERN_DEBUG "Copying data from user\n");
    if(copy_from_user(vchardev_glob.cbuf, buffer, len))
    {
        printk(KERN_ERR "WRITE FAILURE\n");
        return -EFAULT;
    }

    offset += len;

    return len;
}


module_init(vchardev_initialize);
module_exit(vchardev_cleanup);

MODULE_LICENSE(VCHARDEV_LICENSE);
MODULE_AUTHOR(VCHARDEV_AUTHOR);
MODULE_DESCRIPTION(VCHARDEV_DESC);
MODULE_VERSION(VCHARDEV_VERSION);
