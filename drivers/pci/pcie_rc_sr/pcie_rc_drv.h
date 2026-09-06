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

#ifndef PCIE_RC_DRV_H
#define PCIE_RC_DRV_H

#ifndef PCIE_RC_UT

#include <linux/platform_device.h>
#include "pcie_rc_common.h"

#define BUS_SHIFT 20
#define RC_TYPE_MDC 1
#define RC_TYPE_NORMAL 0
#define READ_STATUS_DELAY_TIME  100
#define PCIe_DL_LINK_UP_WAIT_TIME                    10    /* 10ms */
#define PCIe_DL_LINK_UP_WAIT_LIMIT                   10    /* total 100ms */
#define PCIe_DL_LINK_UP_WAIT_MAX                     50    /* total 500ms */

struct pcie_core_ctrl {
    void __iomem *apb_base;
    u32 core_id;
};

int pcie_rc_get_type(void);
u32 pcie_rc_get_active_dev_num(void);
void pcie_rc_ctrl_lock(void);
void pcie_rc_ctrl_unlock(void);
int drv_pcierc_apb_init(u32 core_id);
void drv_pcierc_apb_uninit(u32 core_id);
void __iomem *pcie_rc_get_apb_base(u32 core_id);

#endif // PCIE_RC_UT
#endif // PCIE_RC_DRV_H
