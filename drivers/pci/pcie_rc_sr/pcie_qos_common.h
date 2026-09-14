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
#include "ascend_kernel_hal.h"

#ifndef DRV_PCIE_QOS_COMMON_H
#define DRV_PCIE_QOS_COMMON_H

#ifndef PCIE_RC_UT

#define SCHE_REG_BASE 0xA0100000U

#define PCIE_QOS_MAX 7u
#define PCIE_MPAMID_MAX 31u

#define SCHE_RD_ALLOW_REG_OFFSET 0x398
#define PCIE_RD_ALLOW_REG_BASE (SCHE_REG_BASE + SCHE_RD_ALLOW_REG_OFFSET)
#define SCHE_WR_ALLOW_REG_OFFSET 0x3B8
#define PCIE_WR_ALLOW_REG_BASE (SCHE_REG_BASE+ SCHE_WR_ALLOW_REG_OFFSET)
#define PCIE_ALLOW_REG_SIZE 0x8
#define PCIE_RW_ALLOW_REG_OFFSET 4u
#define PCIE_RW_ALLOW_LVL1_OFFSET 0u
#define PCIE_RW_ALLOW_LVL2_OFFSET 8u
#define PCIE_RW_ALLOW_LVL3_OFFSET 16u
#define PCIE_RW_ALLOW_SEL_OFFSET 25u
#define PCIE_RW_ALLOW_LVL_MASK 0xFF
#define PCIE_RW_ALLOW_SEL_MASK 0x3u

#define PCIE_RW_ALLOW_SEL_ENABLE 0x2u
#define PCIE_RW_ALLOW_SEL_DISABLE 0x0
#define PCIE_RW_ALLOW_LVL_MIN 1u
#define PCIE_RW_ALLOW_LVL_MAX 255u
#define PCIE_QOS_ALLOW_CTRL_RW 0x0
#define PCIE_QOS_ALLOW_CTRL_READ 0x1
#define PCIE_QOS_ALLOW_CTRL_WRITE 0x2

#define SCHE_OTSD_REG_OFFSET 0x2E4
#define PCIE_OTSD_REG_BASE (SCHE_REG_BASE + SCHE_OTSD_REG_OFFSET)
#define PCIE_OTSD_REG_OFFSET 4u
#define PCIE_OTSD_REG_SIZE 0x8
#define PCIE_RD_OTSD_LVL_OFFSET 9u
#define PCIE_RD_OTSD_EN_OFFSET 17u
#define PCIE_WR_OTSD_LVL_OFFSET 0u
#define PCIE_WR_OTSD_EN_OFFSET 8u
#define PCIE_RW_OTSD_LVL_MASK 0xFFU
#define PCIE_RW_OTSD_EN_MASK 0x1u
#define PCIE_RW_OTSD_EN 0x1u
#define PCIE_RW_OTSD_LVL_MIN 1u
#define PCIE_RW_OTSD_LVL_MAX 255u

#define PCIE_QOS_LVL1 0u
#define PCIE_QOS_LVL2 1u
#define PCIE_QOS_LVL3 2u
#define PCIE_OTSD_RD 0u
#define PCIE_OTSD_WR 1u
#define PCIE_ALLOW_MODE_DISABLE 0u
#define PCIE_ALLOW_MODE_RESPONSE 2u
#define PCIE_OTSD_MODE_DISABLE 0u
#define PCIE_OTSD_MODE_RDWR_MERGE 1u
#define PCIE_OTSD_MODE_RDWR 2u
#define PCIE_QOS_REG_MASK 0xFFFFFFFFU

struct qos_reg {
    bool qos_config_saved;
    bool qos_rdallow_saved;
    bool qos_wrallow_saved;
    bool qos_otsd_saved;
    void __iomem *reg_qos_base;
    void __iomem *reg_rdallow_base;
    void __iomem *reg_wrallow_base;
    void __iomem *reg_otsd_base;
    struct qos_master_config_type qos_config;
    struct qos_otsd_config_type ost_config;
    struct qos_allow_config_type rdallow_config;
    struct qos_allow_config_type wrallow_config;
};

int pcie_qos_check_set_qos(int dev_id, const struct qos_master_config_type *cfg);
int pcie_qos_check_set_allow(int dev_id, const struct qos_allow_config_type *cfg);
int pcie_qos_check_get_allow(int dev_id, const struct qos_allow_config_type *cfg);
int pcie_qos_check_set_otsd(int dev_id, const struct qos_otsd_config_type *cfg);

#endif // PCIE_RC_UT
#endif // DRV_PCIE_QOS_COMMON_H