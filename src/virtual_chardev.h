#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/string.h>


#define VCHARDEV_LICENSE "Dual BSD/GPL"
#define VCHARDEV_AUTHOR  "Bogdan Migunov <bogdanmigunov@yandex.ru>"
#define VCHARDEV_DESC    "Virtual character device driver."
#define VCHARDEV_VERSION "1.0"
#define VCHARDEV_NAME    "vchardev"

#define DEFAULT_BUFFER_SIZE 8

#define VCHARDEV_IOC_MAGIC        '@'
#define VCHARDEV_IOC_SET_BLOCK    _IO(VCHARDEV_IOC_MAGIC, 0)
#define VCHARDEV_IOC_SET_NONBLOCK _IO(VCHARDEV_IOC_MAGIC, 1)
#define VCHARDEV_IOC_GET_LAST_OP  _IOR(VCHARDEV_IOC_MAGIC, 2, struct op_info)
#define VCHARDEV_IOC_MAXNR        2




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

struct op_info
{
    struct timespec time;
    pid_t           pid;
    uid_t           uid;
};

struct vchardev
{
    struct circular_buffer cbuf;
    struct op_info         last_op;

    wait_queue_head_t      inq;
    wait_queue_head_t      outq;
};
