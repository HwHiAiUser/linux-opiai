/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */

#ifndef DRV_SNAPSHOT_H
#define DRV_SNAPSHOT_H
#include <linux/printk.h>
#include "drv_log.h"

#define SD_REGION_NAME     "AOS_SD"
#define DP_REGION_NAME     "AOS_DP"
#define AOS_REGION_NAME    "AOS_DP_CORE"

#define MODULE_ID_OFFSET                    12
#define REGION_TYPE_OFFSET                  8

#define DP_DRV1_BLOCK_ID                    12U // for ufs, pcie rc
#define DP_DRV2_BLOCK_ID                    13U // for xgmac
#define DP_DRV3_BLOCK_ID                    14U // for can, zip, sec
#define DP_DRV4_BLOCK_ID                    19U // for tsdrv
#define DP_DRV5_BLOCK_ID                    20U // for ipc

#define DP_DRV6_BLOCK_ID                    24U // for mdc pcie

#define AOS_CORE_DRV1_BLOCK_ID              31U // for can
#define AOS_CORE_DRV2_BLOCK_ID              32U // for ipc

#define SD_DRV1_BLOCK_ID                    42U // for ufs, pcie rc
#define SD_DRV2_BLOCK_ID                    43U // for xgmac
#define SD_DRV3_BLOCK_ID                    46U // for can, zip, sec
#define SD_DRV4_BLOCK_ID                    47U // for tsdrv
#define SD_DRV5_BLOCK_ID                    48U // for ipc

#define INVALID_RESULT       0xFFFFFFFFU

// Driver Available MagicNum Range : 0 ~ 0xFF000000
#define SNAPSHOT_MAGIC_NUM_BASE 0xA5A5A500U

/*
* Black Box Exception ID Specifications
*
+──────────────+─────────────+─────────────+─────────────+─────────────+────────────────────────────────────+
| Exception    | Exception   | Alarm       | Sub-system  | Module      |          Exception Info            |
| Location     | Type        | Severity    | ID          | ID          +─────────────────+──────────────────+
|              |             |             |             |             | Region Type     | Snapshot Status  |
+──────────────+─────────────+─────────────+─────────────+─────────────+─────────────────+──────────────────+
| bit31~bit30  | bit29~bit28 | bit27~bit25 | bit24~bit17 | bit16~bit12 | bit11~bit8      | bit7~bit0        |
+──────────────+─────────────+─────────────+───────────────+───────────+─────────────────+──────────────────+
*/
#define DRV_SNAPSHOT_EXCEPTION_ID_BASE 0xA8020000U   // module_expection_id = base + moduleID + exceptionInfo

typedef enum {
    UFS_MODULE_ID = 1,
    CAN_MODULE_ID = 2,
    PCIE_MODULE_ID = 3,
    TS_PLATFORM_MODULE_ID = 4,
    ZIP_MODULE_ID = 5,
    SPI_MODULE_ID = 6,
    IPC_MODULE_ID = 7,
    SEC_MODULE_ID = 8,
    SERDES_MODULE_ID = 9,
    INITRD_MODULE_ID = 10,
    EMMC_MODULE_ID = 11,
    SSD_MODULE_ID = 12
} DRV_MODULE_ID;

typedef enum {
    REGION_TYPE_DP = 0,
    REGION_TYPE_SD = 1,
    REGION_TYPE_AOS = 2,
    REGION_TYPE_MAX
} REGION_TYPE;

typedef enum {
    SNAPSHOT_STATUS_STARTUP = 1,
    SNAPSHOT_STATUS_SUSPEND = 2,
    SNAPSHOT_STATUS_RESUME = 3,
    SNAPSHOT_STATUS_REMOVE = 4
} SNAPSHOT_STATUS;

struct ModuleExceptionInfo {
    unsigned int module_id;
    unsigned int block_id[REGION_TYPE_MAX];
};

#define snapshot_log_err(fmt, ...)    drv_err("drv_snapshot", fmt, ##__VA_ARGS__)
#define snapshot_log_info(fmt, ...)    drv_info("drv_snapshot", fmt, ##__VA_ARGS__)

void drv_snapshot_bootdot_init(DRV_MODULE_ID module_id, SNAPSHOT_STATUS status, unsigned int expect_status);
void drv_snapshot_bootdot_set(DRV_MODULE_ID module_id, unsigned int current_status);
#endif
