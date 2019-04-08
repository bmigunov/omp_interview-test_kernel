#ifndef __VIRTUAL_CHARDEV_IOCTL_H
#define __VIRTUAL_CHARDEV_IOCTL_H


#ifdef VIRTUAL_CHARDEV_KERNMOD
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/time.h>
#else
#include <ioctl.h>
#include <sys/types.h>
#include <time.h>
#endif /*VIRTUAL_CHARDEV_KERNMOD */


#define VCHARDEV_IOC_MAGIC        '@'
#define VCHARDEV_IOC_SET_BLOCK    _IO(VCHARDEV_IOC_MAGIC, 0)
#define VCHARDEV_IOC_SET_NONBLOCK _IO(VCHARDEV_IOC_MAGIC, 1)
#define VCHARDEV_IOC_GET_LAST_OP  _IOR(VCHARDEV_IOC_MAGIC, 2, struct op_info)
#define VCHARDEV_IOC_MAXNR        2




struct op_info
{
    struct timespec time;
    pid_t           pid;
    uid_t           uid;
};


#endif /* __VIRTUAL_CHARDEV_IOCTL_H */
