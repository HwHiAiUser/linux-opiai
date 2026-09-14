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

#ifndef DRV_PCIE_RC_QOS_H
#define DRV_PCIE_RC_QOS_H

#ifndef PCIE_RC_UT

#include <linux/io.h>
#include <linux/platform_device.h>
#include "ascend_kernel_hal.h"
#include "pcie_qos_common.h"

#define PCIE_MAX_DEV 1
#define PCIE_RC_CORE_NUM 4
#define PCIE_CORE_0_REG_BASE 0xA0200000U
#define PCIE_RC_CORE_OFFSET 0x100000

#define PCIE_AP_IOB_RX_REG_OFFSET 0x4000
#define PCIE_AP_IOB_RX_REG_SIZE 0x2000
#define PCIE_AP_IOB_RX_REG_BASE (PCIE_CORE_0_REG_BASE + PCIE_AP_IOB_RX_REG_OFFSET)

#define PCIE_AML_QOS_CTRL 0x1810
#define PCIE_AML_QOS_MASK 0xF
#define PCIE_AML_QOS_WR_OFFSET 0u
#define PCIE_AML_QOS_RD_OFFSET 4u

#define PCIE_WR_AML_AXUSER_CTRL 0x1AB4
#define PCIE_RD_AML_AXUSER_CTRL 0x1ABC
#define PCIE_AXUSER_CTRL_PMG_MASK 0x3
#define PCIE_AXUSER_CTRL_PMG_OFFSET 4u
#define PCIE_AXUSER_CTRL_MPAM_MASK 0xFF
#define PCIE_AXUSER_CTRL_MPAM_OFFSET 22u

int pcie_rc_qos_init(void);
void pcie_rc_qos_uninit(void);
int pcie_rc_resume_qos_config(u32 core_id, struct platform_device *pdev);
int pcie_rc_suspend_qos_config(u32 core_id, struct platform_device *pdev);
void drv_pcierc_recover_qos_cfg(int dev_id, u32 core_id);

#endif // PCIE_RC_UT
#endif // DRV_PCIE_RC_QOS_H