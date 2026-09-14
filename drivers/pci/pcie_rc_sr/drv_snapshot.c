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

#include "drv_snapshot.h"

#ifdef SNAPSHOT_ENABLE

#include <linux/errno.h>
#include <linux/string.h>

#ifndef NOT_SUPPORT_OSNAME
#include <linux/osname.h>
#include <linux/bootdot.h>
#else
#include "ascend_kernel_hal.h"
#endif

#ifdef STATIC_SKIP
    #define STATIC
#else
    #define STATIC static
#endif

STATIC struct ModuleExceptionInfo moduleExceptionInfo[] = {
    {
        .module_id = UFS_MODULE_ID,
        .block_id = {DP_DRV1_BLOCK_ID, SD_DRV1_BLOCK_ID, INVALID_RESULT},
    },
    {
        .module_id = CAN_MODULE_ID,
        .block_id = {DP_DRV3_BLOCK_ID, SD_DRV3_BLOCK_ID, AOS_CORE_DRV1_BLOCK_ID},
    },
    {
        .module_id = TS_PLATFORM_MODULE_ID,
        .block_id = {DP_DRV4_BLOCK_ID, SD_DRV4_BLOCK_ID, INVALID_RESULT},
    },
#ifdef CFG_SOC_PLATFORM_MDC_V11
    {
        .module_id = PCIE_MODULE_ID,
        .block_id = {DP_DRV6_BLOCK_ID, INVALID_RESULT, INVALID_RESULT},
    },
#else
    {
        .module_id = PCIE_MODULE_ID,
        .block_id = {DP_DRV1_BLOCK_ID, SD_DRV1_BLOCK_ID, INVALID_RESULT},
    },
#endif
    {
        .module_id = IPC_MODULE_ID,
        .block_id = {DP_DRV5_BLOCK_ID, SD_DRV5_BLOCK_ID, AOS_CORE_DRV2_BLOCK_ID},
    },
    {
        .module_id = ZIP_MODULE_ID,
        .block_id = {DP_DRV3_BLOCK_ID, SD_DRV3_BLOCK_ID, INVALID_RESULT},
    },
    {
        .module_id = SEC_MODULE_ID,
        .block_id = {DP_DRV3_BLOCK_ID, SD_DRV3_BLOCK_ID, INVALID_RESULT},
    },
    {
        .module_id = SERDES_MODULE_ID,
        .block_id = {DP_DRV2_BLOCK_ID, SD_DRV2_BLOCK_ID, INVALID_RESULT},
    },
};


STATIC struct ModuleExceptionInfo* drv_snapshot_get_exception_info(DRV_MODULE_ID module_id)
{
    int i;
    struct ModuleExceptionInfo *info = NULL;

    for (i = 0; i < sizeof(moduleExceptionInfo) / sizeof(struct ModuleExceptionInfo); i++) {
        if (module_id ==  moduleExceptionInfo[i].module_id) {
            info = &moduleExceptionInfo[i];
            return info;
        } else {
            continue;
        }
    }

    (void)snapshot_log_err("can't find module_id(%u) in ModuleExceptionInfo array.\n", module_id);
    return NULL;
}

STATIC unsigned int drv_snapshot_get_region_type(void)
{
#ifdef NOT_SUPPORT_OSNAME
    return REGION_TYPE_DP; // non-MDC use the same reginon type as DP
#else
    char *osname = NULL;
    unsigned int region_type;

    osname = osname_get();
    if (osname == NULL) {
        (void)snapshot_log_err("get os name failed, name is null.\n");
        return INVALID_RESULT;
    }

    if (strncmp(osname, DP_REGION_NAME, strlen(AOS_REGION_NAME)) == 0) {
        region_type = REGION_TYPE_DP;
    } else if (strncmp(osname, SD_REGION_NAME, strlen(AOS_REGION_NAME)) == 0) {
        region_type = REGION_TYPE_SD;
    } else if (strncmp(osname, AOS_REGION_NAME, strlen(AOS_REGION_NAME)) == 0) {
        region_type = REGION_TYPE_AOS;
    } else {  /* other case, not supported */
        (void)snapshot_log_err("get os name invalid, os_name: %.*s.\n", (int)strlen(AOS_REGION_NAME), osname);
        return INVALID_RESULT;
    }

    return region_type;
#endif
}

STATIC unsigned int drv_snapshot_get_blkid(DRV_MODULE_ID module_id)
{
    struct ModuleExceptionInfo *info = NULL;
    unsigned int region_type;

    info = drv_snapshot_get_exception_info(module_id);
    if (info == NULL) {
        (void)snapshot_log_err("can't find [%u] in ModuleExceptionInfo array.\n", module_id);
        return INVALID_RESULT;
    }

    region_type = drv_snapshot_get_region_type();
    if (region_type == INVALID_RESULT) {
        return INVALID_RESULT;
    }

    return info->block_id[region_type];
}

/*
* Black Box Exception ID Specifications
*
+--------------+-------------+-------------+-------------+-------------+------------------------------------+
| Exception    | Exception   | Alarm       | Sub-system  | Module      |          Exception Info            |
| Location     | Type        | Severity    | ID          | ID          +-----------------+------------------+
|              |             |             |             |             | Region Type     | Snapshot Status  |
+-------------─+-------------+-------------+-------------+-------------+-----------------+------------------+
| bit31~bit30  | bit29~bit28 | bit27~bit25 | bit24~bit17 | bit16~bit12 | bit11~bit8      | bit7~bit0        |
+-------------─+-------------+-------------+-------------+-------------+-----------------+------------------+
*/
STATIC unsigned int drv_snapshot_get_exception_id(DRV_MODULE_ID module_id, SNAPSHOT_STATUS status)
{
    unsigned int region_type;
    unsigned int exception_id;

    region_type = drv_snapshot_get_region_type();
    if (region_type == INVALID_RESULT) {
        return INVALID_RESULT;
    }

    exception_id = DRV_SNAPSHOT_EXCEPTION_ID_BASE | (module_id << MODULE_ID_OFFSET) |
                   (region_type << REGION_TYPE_OFFSET) | status;
    return exception_id;
}

// Driver Available MagicNum Range : 0 ~ 0xFF000000
STATIC unsigned int drv_snapshot_get_magic_num(DRV_MODULE_ID module_id)
{
    return SNAPSHOT_MAGIC_NUM_BASE | module_id;
}

void drv_snapshot_bootdot_init(DRV_MODULE_ID module_id, SNAPSHOT_STATUS status, unsigned int expect_status)
{
    unsigned int block_id;
    unsigned int exception_id;
    unsigned int magic_num;
    int ret;

    block_id = drv_snapshot_get_blkid(module_id);
    if (block_id == INVALID_RESULT) {
        (void)snapshot_log_err("drv_snapshot_get_blkid_by_name, block_id(%u).\n", block_id);
        return;
    }

    exception_id = drv_snapshot_get_exception_id(module_id, status);
    if (exception_id == INVALID_RESULT) {
        (void)snapshot_log_err("drv_snapshot_get_blkid_by_name, exception_id(%u).\n", exception_id);
        return;
    }

    magic_num = drv_snapshot_get_magic_num(module_id);

    ret = bootdot_init_blk(block_id, magic_num, exception_id, expect_status);
    if (ret != 0) {
        (void)snapshot_log_err("bootdot_init_blk failed, ret(%d), module_id(%u), block_id(%u), "
            "snapshot_status(%u), exception_id(%#x), expect_status(%u).\n",
            ret, module_id, block_id, status, exception_id, expect_status);
        return;
    }
    snapshot_log_info("bootdot_init_blk success, module_id(%u), block_id(%u), snapshot_status(%u),"
        "exception_id(%#x), expect_status(%u).\n", module_id, block_id, status, exception_id,
        expect_status);

    return;
}

void drv_snapshot_bootdot_set(DRV_MODULE_ID module_id, unsigned int current_status)
{
    unsigned int block_id;
    unsigned int magic_num;
    int ret;

    block_id = drv_snapshot_get_blkid(module_id);
    if (block_id == INVALID_RESULT) {
        (void)snapshot_log_err("drv_snapshot_get_blkid_by_name, block_id(%u).\n", block_id);
        return;
    }

    magic_num = drv_snapshot_get_magic_num(module_id);

    ret = bootdot_set_blk(block_id, magic_num, current_status);
    if (ret != 0) {
        (void)snapshot_log_err("bootdot_set_blk failed, ret(%d), module_id(%u), block_id(%u), current_status(%u).\n",
                               ret, module_id, block_id, current_status);
    }

    return;
}

#else
void drv_snapshot_bootdot_init(DRV_MODULE_ID module_id, SNAPSHOT_STATUS status, unsigned int expect_status) {}
void drv_snapshot_bootdot_set(DRV_MODULE_ID module_id, unsigned int current_status) {}
#endif
