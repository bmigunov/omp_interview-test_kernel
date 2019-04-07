#include "virtual_chardev.h"




static unsigned int buffer_size = DEFAULT_BUFFER_SIZE;
module_param(buffer_size, uint, S_IRUGO);
static dev_t dev;

static struct vchardev vchardev_glob =
{
    .cbuf.buffer = NULL,
    .cbuf.head =   0,
    .cbuf.tail =   0,
};
static struct file_operations vchardev_fops =
{
    .owner =          THIS_MODULE,
    .open =           vchardev_open,
    .release =        vchardev_release,
    .read =           vchardev_read,
    .write =          vchardev_write,
    .unlocked_ioctl = vchardev_ioctl,
};
static struct cdev this_cdev;


static int __init vchardev_initialize(void)
{
    int result;

    printk(KERN_WARNING "Initializing virtual character device module...\n");
    printk(KERN_DEBUG "Circular buffer size parameter: %d\nAllocating memory\n",
           buffer_size);
    vchardev_glob.cbuf.buffer = kmalloc(buffer_size, GFP_KERNEL);
    memset(vchardev_glob.cbuf.buffer, 0, buffer_size);
    init_waitqueue_head(&vchardev_glob.inq);
    init_waitqueue_head(&vchardev_glob.outq);

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
    kfree(vchardev_glob.cbuf.buffer);
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
    unsigned long long int offset_internal = vchardev_glob.cbuf.tail;

    printk(KERN_DEBUG "Someone is trying to read from the device file.\n");

    if(*offset >= buffer_size)
    {
        offset_internal += *offset % buffer_size;
    }
    else
    {
        offset_internal += *offset;
    }
    if(offset_internal >= buffer_size)
    {
        offset_internal = offset_internal % buffer_size;
    }
    printk(KERN_DEBUG "Read offset %llu\n", offset_internal);

    printk(KERN_DEBUG "Pre-read tail position: %lu; Pre-read head position: %lu\n",
           vchardev_glob.cbuf.tail, vchardev_glob.cbuf.head);

    while(vchardev_glob.cbuf.tail == vchardev_glob.cbuf.head)
    {
        if(filp->f_flags & O_NONBLOCK)
        {
            break;
        }
        else
        {
            printk(KERN_DEBUG "Circular buffer is empty. Sending read to sleep, O_NONBLOCK is set\n");
            if(wait_event_interruptible(vchardev_glob.inq,
                                        (vchardev_glob.cbuf.tail != vchardev_glob.cbuf.head)))
            {
                return -ERESTARTSYS;
            }
        }
    }
    if(offset_internal + len < buffer_size)
    {
        if(copy_to_user(buffer, vchardev_glob.cbuf.buffer + offset_internal,
                        len))
        {
            return -EFAULT;
        }
        memset(vchardev_glob.cbuf.buffer + offset_internal, '\0', len);
        vchardev_glob.cbuf.tail = offset_internal + len;
    }
    else
    {
        len = buffer_size;
        if(copy_to_user(buffer, vchardev_glob.cbuf.buffer + offset_internal,
                        len - offset_internal))
        {
            return -EFAULT;
        }
        if(copy_to_user(buffer + len - offset_internal, vchardev_glob.cbuf.buffer,
                        offset_internal))
        {
            return -EFAULT;
        }
        memset(vchardev_glob.cbuf.buffer, '\0', buffer_size);
        vchardev_glob.cbuf.tail = vchardev_glob.cbuf.head;
    }

    *offset += len;
    wake_up_interruptible(&vchardev_glob.outq);
    return len;
}

static ssize_t vchardev_write(struct file *filp, const char __user *buffer,
                              size_t len, loff_t *offset)
{
    unsigned long long int offset_internal = vchardev_glob.cbuf.head;
    unsigned int i, leftover;

    printk(KERN_DEBUG "Someone is trying to write to the device file.\n");

    if(*offset >= buffer_size)
    {
        offset_internal += *offset % buffer_size;
    }
    else
    {
        offset_internal += *offset;
    }
    if(offset_internal >= buffer_size)
    {
        offset_internal = offset_internal % buffer_size;
    }
    printk(KERN_DEBUG "Offset: %llu\n", offset_internal);

    printk(KERN_DEBUG "Pre-write head position: %lu; Pre-write tail position: %lu\n",
           vchardev_glob.cbuf.head, vchardev_glob.cbuf.tail);

    while(vchardev_glob.cbuf.head == vchardev_glob.cbuf.tail - 1)
    {
        if(filp->f_flags & O_NONBLOCK)
        {
            break;
        }
        else
        {
            printk(KERN_DEBUG "O_NONBLOCK write\n Sending write to sleep, circular buffer is full\n");
            if(wait_event_interruptible(vchardev_glob.outq,
                                        (vchardev_glob.cbuf.head != vchardev_glob.cbuf.tail - 1)))
            {
                return -ERESTARTSYS;
            }
        }
    }
    if(offset_internal + len <= buffer_size)
    {
        if(copy_from_user(vchardev_glob.cbuf.buffer + offset_internal, buffer,
                          len))
        {
            return -EFAULT;
        }
        vchardev_glob.cbuf.head = offset_internal + len;
    }
    else
    {
        leftover = (offset_internal + len) % buffer_size;

        if(len <= buffer_size)
        {
            if(copy_from_user(vchardev_glob.cbuf.buffer + offset_internal,
                              buffer, len - leftover))
            {
                return -EFAULT;
            }
            if(copy_from_user(vchardev_glob.cbuf.buffer,
                              buffer + len - leftover,
                              leftover))
            {
                return -EFAULT;
            }
        }
        else
        {
            printk(KERN_DEBUG "Buffer overflow case\nLeftover: %d", leftover);

            for(i = 0; i < (len + offset_internal) / buffer_size; ++i)
            {
                printk(KERN_DEBUG "i: %d; copying first part\n", i);
                if(copy_from_user(vchardev_glob.cbuf.buffer + offset_internal,
                                  buffer + i * buffer_size,
                                  buffer_size - offset_internal))
                {
                    return -EFAULT;
                }
                if(i != (len + offset_internal) / buffer_size - 1)
                {
                    printk (KERN_DEBUG "copying second part\n");
                    if(copy_from_user(vchardev_glob.cbuf.buffer,
                                      buffer + (i + 1) * buffer_size - offset_internal,
                                      offset_internal))
                    {
                        return -EFAULT;
                    }
                }
            }
            printk("i: %d; copying leftover\n", i);
            if(copy_from_user(vchardev_glob.cbuf.buffer,
                              buffer + i * buffer_size - offset_internal,
                              leftover))
            {
                return -EFAULT;
            }
        }

        vchardev_glob.cbuf.head = leftover;
        vchardev_glob.cbuf.tail = vchardev_glob.cbuf.head + 1;
    }

    *offset += len;

    printk(KERN_DEBUG "Post-write head position: %lu; Post-write tail position: %lu\n",
           vchardev_glob.cbuf.head, vchardev_glob.cbuf.tail);
    printk(KERN_DEBUG "Chars written: %lu\n", len);

    wake_up_interruptible(&vchardev_glob.inq);
    return len;
}

long int vchardev_ioctl(struct file *filp, unsigned int cmd,
                        unsigned long arg)
{
    return 0;
}


module_init(vchardev_initialize);
module_exit(vchardev_cleanup);

MODULE_LICENSE(VCHARDEV_LICENSE);
MODULE_VERSION(VCHARDEV_VERSION);
MODULE_AUTHOR(VCHARDEV_AUTHOR);
MODULE_DESCRIPTION(VCHARDEV_DESC);
