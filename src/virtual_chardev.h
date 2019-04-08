#ifndef __VIRTUAL_CHARDEV_H
#define __VIRTUAL_CHARDEV_H

#define VIRTUAL_CHARDEV_KERNMOD
#define VIRTUAL_CHARDEV_DEBUG


#include "virtual_chardev_ioctl.h"
#include <linux/fs.h>


#define VCHARDEV_LICENSE "Dual BSD/GPL"
#define VCHARDEV_AUTHOR  "Bogdan Migunov <bogdanmigunov@yandex.ru>"
#define VCHARDEV_DESC    "Virtual character device driver."
#define VCHARDEV_VERSION "1.0"
#define VCHARDEV_NAME    "vchardev"

#define DEFAULT_BUFFER_SIZE 8




static int     vchardev_open(struct inode *, struct file *);
static int     vchardev_release(struct inode *, struct file *);
static ssize_t vchardev_read(struct file *, char *, size_t, loff_t *);
static ssize_t vchardev_write(struct file *, const char *, size_t, loff_t *);
long int       vchardev_ioctl(struct file *, unsigned int, unsigned long);


struct circular_buffer
{
    char              *buffer;
    unsigned long int head;
    unsigned long int tail;
};

struct vchardev
{
    struct circular_buffer cbuf;
    struct op_info         last_op;

    wait_queue_head_t      inq;
    wait_queue_head_t      outq;
};


#endif /* __VIRTUAL_CHARDEV_H */
