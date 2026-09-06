/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
#ifndef PCIE_RC_UT

#include "drv_log.h"
#include "pcie_qos_common.h"

#define module_name_pcie_qos "pcie_qos"

#define pcie_qos_err(fmt, ...) do { \
    drv_err(module_name_pcie_qos, "<%s:%d:%d> " fmt, \
    current->comm, current->tgid, current->pid, ##__VA_ARGS__); \
} while (0)

int pcie_qos_check_set_qos(int dev_id, const struct qos_master_config_type *cfg)
{
    if (cfg == NULL) {
        pcie_qos_err("qos_master_config_type is NULL. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    if (cfg->qos > PCIE_QOS_MAX) {
        pcie_qos_err("Invalid qos. (dev_id=%d; qos=%u)\n", dev_id, cfg->qos);
        return -EINVAL;
    }

    if (cfg->mpamid > PCIE_MPAMID_MAX) {
        pcie_qos_err("Invalid mpam_id. (dev_id=%d; mpam_id=%u)\n", dev_id, cfg->mpamid);
        return -EINVAL;
    }

    return 0;
}

int pcie_qos_check_set_allow(int dev_id, const struct qos_allow_config_type *cfg)
{
    int i;

    if (cfg == NULL) {
        pcie_qos_err("qos_allow_config_type is NULL. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    /* allow mode */
    if ((cfg->qos_allow_mode != PCIE_ALLOW_MODE_RESPONSE) && ((cfg->qos_allow_mode != PCIE_ALLOW_MODE_DISABLE))) {
        pcie_qos_err("Invalid qos_allow_mode. (dev_id=%d; allow_mode=%u)\n", dev_id, cfg->qos_allow_mode);
        return -EINVAL;
    }

    /* verify allow_ctrl */
    if ((cfg->qos_allow_ctrl != PCIE_QOS_ALLOW_CTRL_READ) && (cfg->qos_allow_ctrl != PCIE_QOS_ALLOW_CTRL_WRITE) &&
        (cfg->qos_allow_ctrl != PCIE_QOS_ALLOW_CTRL_RW)) {
        pcie_qos_err("Invalid qos_allow_ctrl. (dev_id=%d; allow_ctrl=%u)\n", dev_id, cfg->qos_allow_ctrl);
        return -EINVAL;
    }

    /* verify allow_lvl */
    for (i = 0; i < MAX_QOS_ALLOW_LEVEL; i++) {
        if ((cfg->qos_allow_lvl[i] < PCIE_RW_ALLOW_LVL_MIN) || (cfg->qos_allow_lvl[i] > PCIE_RW_ALLOW_LVL_MAX)) {
            pcie_qos_err("Invalid qos_allow_lvl. (dev_id=%d; qos_allow_lvl=%u)\n", dev_id, cfg->qos_allow_lvl[i]);
            return -EINVAL;
        }
        if ((i < MAX_QOS_ALLOW_LEVEL -1) && (cfg->qos_allow_lvl[i] <= cfg->qos_allow_lvl[i+1])) {
            pcie_qos_err("Qos_allow_lvl must be in ascending order. (dev_id=%d; qos_allow_lvl=%u; qos_allow_lvl=%u)\n",
                dev_id, cfg->qos_allow_lvl[i], cfg->qos_allow_lvl[i+1]);
            return -EINVAL;
        }
    }
    return 0;
}

int pcie_qos_check_get_allow(int dev_id, const struct qos_allow_config_type *cfg)
{
    if (cfg == NULL) {
        pcie_qos_err("qos_allow_config_type is NULL. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    if ((cfg->qos_allow_ctrl != PCIE_QOS_ALLOW_CTRL_READ) && (cfg->qos_allow_ctrl != PCIE_QOS_ALLOW_CTRL_WRITE)) {
        pcie_qos_err("Invalid qos_allow_ctrl. (dev_id=%d; allow_ctrl=%u)\n", dev_id, cfg->qos_allow_ctrl);
        return -EINVAL;
    }

    return 0;
}

int pcie_qos_check_set_otsd(int dev_id, const struct qos_otsd_config_type *cfg)
{
    int i;

    if (cfg == NULL) {
        pcie_qos_err("qos_otsd_config_type is NULL. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    /* verify otsd_mode and otsd_lvl */
    if (cfg->otsd_mode == PCIE_OTSD_MODE_RDWR) {
        for (i = 0; i < MAX_OTSD_LEVEL; i++) {
            if ((cfg->otsd_lvl[i] < PCIE_RW_OTSD_LVL_MIN) || (cfg->otsd_lvl[i] > PCIE_RW_OTSD_LVL_MAX)) {
                pcie_qos_err("Invalid qos_otsd_lvl. (dev_id=%d; qos_otsd_lvl=%u)\n", dev_id, cfg->otsd_lvl[i]);
                return -EINVAL;
            }
        }
        return 0;
    } else if (cfg->otsd_mode == PCIE_OTSD_MODE_RDWR_MERGE) {
        if ((cfg->otsd_lvl[0] < PCIE_RW_OTSD_LVL_MIN) || (cfg->otsd_lvl[0] > PCIE_RW_OTSD_LVL_MAX)) {
                pcie_qos_err("Invalid qos_otsd_lvl. (dev_id=%d; qos_otsd_lvl=%u)\n", dev_id, cfg->otsd_lvl[i]);
                return -EINVAL;
            }
        return 0;
    } else {
        if (cfg->otsd_mode != PCIE_OTSD_MODE_DISABLE) {
            pcie_qos_err("Invalid otsd_mode. (dev_id=%d; otsd_mode=%u)\n", dev_id, cfg->otsd_mode);
            return -EINVAL;
        }
    }

    return 0;
}

#else

int pcie_qos_check_set_otsd(void)
{
    return 0;
}
#endif /* end of PCIE_RC_UT */