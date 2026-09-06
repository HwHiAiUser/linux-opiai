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

#ifndef PCIE_RC_DFX_REG_H
#define PCIE_RC_DFX_REG_H

#include <linux/types.h>

#ifdef CFG_FEATURE_PCIE_DFX

#include "pcie_rc_drv.h"
#include "pcie_rc_ctrl.h"

#define pcie_dfx_mac_write drv_pcierc_mac_write
#define pcie_dfx_mac_read drv_pcierc_mac_read

void pcie_ltssm_tracer(u32 chip_id, u32 port);

#else
static inline void pcie_ltssm_tracer(u32 chip_id, u32 port) { }
#endif

#endif
