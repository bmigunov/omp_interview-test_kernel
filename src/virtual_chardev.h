#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define VCHARDEV_LICENSE "Dual BSD/GPL"
#define VCHARDEV_AUTHOR "Bogdan Migunov <bogdanmigunov@yandex.ru>"
#define VCHARDEV_DESC "Virtual character device driver."
#define VCHARDEV_VERSION "1.0"

#define DEFAULT_CBUF_SIZE 512




static int vchardev_open(struct inode *, struct file *);
static int vchardev_release(struct inode *, struct file *);
static ssize_t vchardev_read(struct file *, char *, size_t, loff_t *);
static ssize_t vchardev_write(struct file *, const char *, size_t, loff_t *);
