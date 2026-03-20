#ifndef _LINUX_ASCEND_SMMU_H
#define _LINUX_ASCEND_SMMU_H

#include <linux/types.h>

int svm_get_pasid(pid_t vpid, int dev_id);

#endif
