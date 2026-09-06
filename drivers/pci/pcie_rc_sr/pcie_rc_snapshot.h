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

#ifndef PCIE_RC_SNAPSHOT_H
#define PCIE_RC_SNAPSHOT_H

#include <linux/types.h>
#include "drv_snapshot.h"

typedef enum {
    PCIERC_STARTUP_BEGIN               = 0,
    PCIERC_STARTUP_GET_CORE_ID_FAIL    = 1,
    PCIERC_STARTUP_APB_INIT_FAIL       = 2,
    PCIERC_STARTUP_MATCH_NODE_FAIL     = 3,
    PCIERC_STARTUP_COMMON_PROBE_FAIL   = 4,
    PCIERC_STARTUP_CREAT_THREAD_FAIL   = 5,
    PCIERC_STARTUP_WAIT_LINK_UP        = 6,
    PCIERC_STARTUP_EXPECT              = 9,
    PCIERC_SUSPEND_BEGIN               = 10,
    PCIERC_SUSPEND_GET_CORE_ID_FAIL    = 11,
    PCIERC_SUSPEND_LINK_UP_FAIL        = 12,
    PCIERC_SUSPEND_GET_APB_BASE_FAIL   = 13,
    PCIERC_SUSPEND_EXPECT              = 19,
    PCIERC_RESUME_BEGIN                = 20,
    PCIERC_RESUME_GET_CORE_ID_FAIL     = 21,
    PCIERC_RESUME_APB_INIT_FAIL        = 22,
    PCIERC_RESUME_QOS_CONFIG_FAIL      = 23,
    PCIERC_RESUME_DEVICE_FAIL          = 24,
    PCIERC_RESUME_WAIT_LINK_UP         = 25,
    PCIERC_RESUME_EXPECT               = 29,
    PCIERC_REMOVE_BEGIN                = 30,
    PCIERC_REMOVE_UNINIT_CTRL_FAIL     = 31,
    PCIERC_REMOVE_EXPECT               = 39
} PCIERC_SNAPSHOT_TYPE;

#endif
