#ifndef BASE_CONVERT_IOCTL_H
#define BASE_CONVERT_IOCTL_H

#include <linux/ioctl.h>

#define BASE_MAGIC 'k'

#define IOCTL_SET_FROM_BASE   _IOW(BASE_MAGIC, 1, int)
#define IOCTL_SET_TO_BASE     _IOW(BASE_MAGIC, 2, int)

#endif