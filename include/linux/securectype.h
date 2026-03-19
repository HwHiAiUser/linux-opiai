#ifndef _LINUX_HISI_SECURECTYPE_H
#define _LINUX_HISI_SECURECTYPE_H

#include <linux/errno.h>
#include <linux/stddef.h>
#include <linux/types.h>

typedef int errno_t;

#ifndef EOK
#define EOK 0
#endif

#ifndef EINVAL_AND_RESET
#define EINVAL_AND_RESET 150
#endif

#ifndef ERANGE_AND_RESET
#define ERANGE_AND_RESET 162
#endif

#ifndef EOVERLAP_AND_RESET
#define EOVERLAP_AND_RESET 182
#endif

#endif
