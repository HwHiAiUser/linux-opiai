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

#ifndef SDK_HILINK_PUB_H
#define SDK_HILINK_PUB_H

#include <linux/types.h>
#include <stdbool.h>

/* Pre-processor Definitions */
/* Macro ID */
#define MACRO_0 0
#define MACRO_1 1
#define MACRO_MAX 2

/* CS Number */
#define CS0 0x0
#define CS1 0x1
#define CS_MAX 0x2

/* DS Number */
#define DS0 0x0
#define DS1 0x1
#define DS2 0x2
#define DS3 0x3
#define DS_MAX 0x4
#define DS_ALL 0xff

/* Lane ID */
#define LANE_0 0x0
#define LANE_1 0x1
#define LANE_2 0x2
#define LANE_3 0x3
#define LANE_MAX 0x4

typedef enum {
    HILINK_XGE_1_25G = 0x0,
    HILINK_XGE_3_125G,
    HILINK_XGE_BUTT,
} HilinkRate;

u32 HILINK_DsDataRateSwitch(u32 macro, u32 dsIndex, HilinkRate targetRate, u32 dsWidth);
u32 HILINK_LinkDataRateSwitch(u32 macro, u32 sliceMask, HilinkRate targetRate, u32 dsWidth);
int hilink_is_ready(void);

#endif
