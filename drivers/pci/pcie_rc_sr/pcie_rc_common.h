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

#ifndef DRV_PCIE_RC_COMMON_H
#define DRV_PCIE_RC_COMMON_H

#ifdef STATIC_SKIP
#define STATIC
#else
#define STATIC static
#endif

#ifndef PCIE_RC_UT

#include "drv_log.h"
#include "kernel_version_adapt.h"
#include "pcie_rc_snapshot.h"

#define module_name_pcie_rc "drv_pcie_rc"

#define drv_pcie_rc_err(fmt, ...) do { \
    drv_err(module_name_pcie_rc, "<%s:%d:%d> " fmt, \
    current->comm, current->tgid, current->pid, ##__VA_ARGS__); \
} while (0)
#define drv_pcie_rc_warn(fmt, ...) do { \
    drv_warn(module_name_pcie_rc, "<%s:%d:%d> " fmt, \
    current->comm, current->tgid, current->pid, ##__VA_ARGS__); \
} while (0)
#define drv_pcie_rc_info(fmt, ...) do { \
    drv_info(module_name_pcie_rc, "<%s:%d:%d> " fmt, \
    current->comm, current->tgid, current->pid, ##__VA_ARGS__); \
} while (0)
#define drv_pcie_rc_debug(fmt, ...) do { \
    drv_debug(module_name_pcie_rc, "<%s:%d:%d> " fmt, \
    current->comm, current->tgid, current->pid, ##__VA_ARGS__); \
} while (0)

#else

#define drv_pcie_rc_err printf
#define drv_pcie_rc_info printf

#endif // PCIE_RC_UT
#endif // DRV_PCIE_RC_COMMON_H