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

#include <linux/securec.h>
#include <linux/of.h>
#include "pcie_rc_common.h"
#include "pcie_rc_qos.h"

struct qos_reg g_qos_ctrl[PCIE_MAX_DEV][PCIE_RC_CORE_NUM];

STATIC void pcie_rc_set_reg(void __iomem *reg_addr, u32 mask, u32 offset, u32 val)
{
    u32 reg_val = readl(reg_addr);
    reg_val &= ~(mask << offset);
    reg_val |= (val & mask) << offset;
    writel(reg_val, reg_addr);
}

STATIC u32 pcie_rc_get_reg(const void __iomem *reg_addr, u32 mask, u32 offset)
{
    u32 reg_val = readl(reg_addr);
    return (reg_val >> offset) & mask;
}

STATIC void pcie_rc_set_qos(void __iomem *iob_base, u32 qos)
{
    void __iomem *reg_base = iob_base + PCIE_AML_QOS_CTRL;
    pcie_rc_set_reg(reg_base, PCIE_AML_QOS_MASK, PCIE_AML_QOS_WR_OFFSET, qos);
    pcie_rc_set_reg(reg_base, PCIE_AML_QOS_MASK, PCIE_AML_QOS_RD_OFFSET, qos);
}

STATIC void pcie_rc_set_pmg(void __iomem *iob_base, u32 pmg)
{
    void __iomem *reg_base = iob_base + PCIE_WR_AML_AXUSER_CTRL;
    pcie_rc_set_reg(reg_base, PCIE_AXUSER_CTRL_PMG_MASK, PCIE_AXUSER_CTRL_PMG_OFFSET, pmg);

    reg_base = iob_base + PCIE_RD_AML_AXUSER_CTRL;
    pcie_rc_set_reg(reg_base, PCIE_AXUSER_CTRL_PMG_MASK, PCIE_AXUSER_CTRL_PMG_OFFSET, pmg);
}

STATIC void pcie_rc_set_mpam_id(void __iomem *iob_base, u32 mpam_id)
{
    void __iomem *reg_base = iob_base + PCIE_WR_AML_AXUSER_CTRL;
    pcie_rc_set_reg(reg_base, PCIE_AXUSER_CTRL_MPAM_MASK, PCIE_AXUSER_CTRL_MPAM_OFFSET, mpam_id);

    reg_base = iob_base + PCIE_RD_AML_AXUSER_CTRL;
    pcie_rc_set_reg(reg_base, PCIE_AXUSER_CTRL_MPAM_MASK, PCIE_AXUSER_CTRL_MPAM_OFFSET, mpam_id);
}

STATIC u32 agentdrv_get_qos(void __iomem *iob_base)
{
    void __iomem *reg_base = iob_base + PCIE_AML_QOS_CTRL;
    return pcie_rc_get_reg(reg_base, PCIE_AML_QOS_MASK, PCIE_AML_QOS_WR_OFFSET);
}

STATIC u32 agentdrv_get_pmg(void __iomem *iob_base)
{
    void __iomem *reg_base = iob_base + PCIE_WR_AML_AXUSER_CTRL;
    return pcie_rc_get_reg(reg_base, PCIE_AXUSER_CTRL_PMG_MASK, PCIE_AXUSER_CTRL_PMG_OFFSET);
}

STATIC u32 agentdrv_get_mpam_id(void __iomem *iob_base)
{
    void __iomem *reg_base = iob_base + PCIE_WR_AML_AXUSER_CTRL;
    return pcie_rc_get_reg(reg_base, PCIE_AXUSER_CTRL_MPAM_MASK, PCIE_AXUSER_CTRL_MPAM_OFFSET);
}

STATIC int pcie_rc_dev_qos_init(int dev_id)
{
    int core_id;
    void __iomem *reg_rdallow_base = NULL;
    void __iomem *reg_wrallow_base = NULL;
    void __iomem *reg_otsd_base = NULL;

    reg_rdallow_base = ioremap(PCIE_RD_ALLOW_REG_BASE, PCIE_ALLOW_REG_SIZE);
    if (reg_rdallow_base == NULL) {
        return -ENOMEM;
    }

    reg_wrallow_base = ioremap(PCIE_WR_ALLOW_REG_BASE, PCIE_ALLOW_REG_SIZE);
    if (reg_wrallow_base == NULL) {
        goto dev_release_rdallow;
    }

    reg_otsd_base = ioremap(PCIE_OTSD_REG_BASE, PCIE_OTSD_REG_SIZE);
    if (reg_otsd_base == NULL) {
        goto dev_release_wrallow;
    }

    for (core_id = 0; core_id < PCIE_RC_CORE_NUM; core_id++) {
        g_qos_ctrl[dev_id][core_id].reg_qos_base = ioremap(PCIE_AP_IOB_RX_REG_BASE + (core_id * PCIE_RC_CORE_OFFSET),
            PCIE_AP_IOB_RX_REG_SIZE);
        if (g_qos_ctrl[dev_id][core_id].reg_qos_base == NULL) {
            goto dev_release;
        }
        g_qos_ctrl[dev_id][core_id].reg_rdallow_base = reg_rdallow_base +
            (((u32)core_id >> 1) * PCIE_RW_ALLOW_REG_OFFSET);
        g_qos_ctrl[dev_id][core_id].reg_wrallow_base = reg_wrallow_base +
            (((u32)core_id >> 1) * PCIE_RW_ALLOW_REG_OFFSET);
        g_qos_ctrl[dev_id][core_id].reg_otsd_base = reg_otsd_base + (((u32)core_id >> 1) * PCIE_OTSD_REG_OFFSET);
    }

    return 0;

dev_release:
    for (core_id = core_id - 1; core_id >= 0; core_id--) {
        iounmap(g_qos_ctrl[dev_id][core_id].reg_qos_base);
        g_qos_ctrl[dev_id][core_id].reg_qos_base = NULL;
        g_qos_ctrl[dev_id][core_id].reg_rdallow_base = NULL;
        g_qos_ctrl[dev_id][core_id].reg_wrallow_base = NULL;
        g_qos_ctrl[dev_id][core_id].reg_otsd_base = NULL;
    }

    iounmap(reg_otsd_base);

dev_release_wrallow:
    iounmap(reg_wrallow_base);

dev_release_rdallow:
    iounmap(reg_rdallow_base);

    return -ENOMEM;
}

STATIC void pcie_rc_dev_qos_uninit(int dev_id)
{
    int core_id;
    void __iomem *reg_rdallow_base = g_qos_ctrl[dev_id][0].reg_rdallow_base;
    void __iomem *reg_wrallow_base = g_qos_ctrl[dev_id][0].reg_wrallow_base;
    void __iomem *reg_otsd_base = g_qos_ctrl[dev_id][0].reg_otsd_base;

    for (core_id = 0; core_id < PCIE_RC_CORE_NUM; core_id++) {
        if (g_qos_ctrl[dev_id][core_id].reg_qos_base != NULL) {
            iounmap(g_qos_ctrl[dev_id][core_id].reg_qos_base);
            g_qos_ctrl[dev_id][core_id].reg_qos_base = NULL;
            g_qos_ctrl[dev_id][core_id].reg_rdallow_base = NULL;
            g_qos_ctrl[dev_id][core_id].reg_wrallow_base = NULL;
            g_qos_ctrl[dev_id][core_id].reg_otsd_base = NULL;
        }
    }

    if (reg_rdallow_base != NULL) {
        iounmap(reg_rdallow_base);
    }

    if (reg_wrallow_base != NULL) {
        iounmap(reg_wrallow_base);
    }

    if (reg_otsd_base != NULL) {
        iounmap(reg_otsd_base);
    }
}

int pcie_rc_qos_init(void)
{
    int dev_id, ret;

    ret = memset_s((void *)g_qos_ctrl, sizeof(g_qos_ctrl), 0, sizeof(g_qos_ctrl));
    if (ret != 0) {
        drv_pcie_rc_err("g_qos_ctrl memset_s failed. (ret=%d)\n", ret);
        return ret;
    }

    for (dev_id = 0; dev_id < PCIE_MAX_DEV; dev_id++) {
        ret = pcie_rc_dev_qos_init(dev_id);
        if (ret != 0) {
            drv_pcie_rc_err("Pcie rc qos init failed. (dev_id=%d; ret=%d)\n", dev_id, ret);
            goto release;
        }
    }

    return 0;

release:
    for (dev_id = dev_id - 1; dev_id >= 0; dev_id--) {
        pcie_rc_dev_qos_uninit(dev_id);
    }

    return -ENOMEM;
}

void pcie_rc_qos_uninit(void)
{
    int dev_id;

    for (dev_id = 0; dev_id < PCIE_MAX_DEV; dev_id++) {
        pcie_rc_dev_qos_uninit(dev_id);
    }
}

STATIC void pcie_rc_set_qos_config_to_core(int dev_id, int core_id, const struct qos_master_config_type *cfg)
{
    void __iomem *iob_base = g_qos_ctrl[dev_id][core_id].reg_qos_base;

    pcie_rc_set_qos(iob_base, cfg->qos);
    pcie_rc_set_pmg(iob_base, cfg->pmg);
    pcie_rc_set_mpam_id(iob_base, cfg->mpamid);

    return;
}

STATIC void pcie_rc_save_qos_config(int dev_id, int core_id, const struct qos_master_config_type *cfg)
{
    g_qos_ctrl[dev_id][core_id].qos_config_saved = true;
    g_qos_ctrl[dev_id][core_id].qos_config.qos = cfg->qos;
    g_qos_ctrl[dev_id][core_id].qos_config.pmg = cfg->pmg;
    g_qos_ctrl[dev_id][core_id].qos_config.mpamid = cfg->mpamid;

    return;
}

STATIC int pcie_rc_qos_check_devid(int dev_id)
{
    if ((dev_id >= PCIE_MAX_DEV) || (dev_id < 0)) {
        drv_pcie_rc_err("Invalid dev_id. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    return 0;
}

STATIC int pcie_rc_set_qos_config(int dev_id, const struct qos_master_config_type *cfg)
{
    int count = 0;
    int core_id;

    if (pcie_rc_qos_check_devid(dev_id) != 0) {
        drv_pcie_rc_err("Invalid dev_id. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    if (pcie_qos_check_set_qos(dev_id, cfg) != 0) {
        drv_pcie_rc_err("Invalid qos_master_config. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    for (core_id = 0; core_id < PCIE_RC_CORE_NUM; core_id++) {
        if (((1ULL << (u32)core_id) & cfg->bitmap[0]) != 0) {
            pcie_rc_set_qos_config_to_core(dev_id, core_id, cfg);
            pcie_rc_save_qos_config(dev_id, core_id, cfg); /* mdc support recover qos after self heal */
            count++;
        }
    }

    if (count <= 0) {
        drv_pcie_rc_err("Set qos config failed, no match bitmap. (dev_id=%d; bitmap=0x%llx)\n", dev_id, cfg->bitmap[0]);
        return -EINVAL;
    }

    return 0;
}

STATIC int pcie_rc_get_qos_config_from_core(int dev_id, int core_id, struct qos_master_config_type *cfg)
{
    void __iomem *iob_base = g_qos_ctrl[dev_id][core_id].reg_qos_base;

    cfg->qos = agentdrv_get_qos(iob_base);
    cfg->pmg = agentdrv_get_pmg(iob_base);
    cfg->mpamid = agentdrv_get_mpam_id(iob_base);

    return 0;
}

STATIC int pcie_rc_get_core_id(int dev_id, u64 cfg_bitmap, int *core_id)
{
    u32 core_count;

    if (cfg_bitmap == 0) {
        *core_id = 0;
        return 0;
    }

    for (core_count = 0; core_count < PCIE_RC_CORE_NUM; core_count++) {
        if (((1ULL << core_count) & cfg_bitmap) != 0) {
            break;
        }
    }

    if (core_count >= PCIE_RC_CORE_NUM) {
        drv_pcie_rc_err("Get qos config failed, no match bitmap. (dev_id=%d; cfg_bitmap=0x%llx)\n", dev_id, cfg_bitmap);
        return -EINVAL;
    }

    *core_id = (int)core_count;

    return 0;
}

STATIC int pcie_rc_get_qos_config(int dev_id, struct qos_master_config_type *cfg)
{
    int core_id;

    if (pcie_rc_qos_check_devid(dev_id) != 0) {
        drv_pcie_rc_err("Invalid dev_id. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    if (cfg == NULL) {
        drv_pcie_rc_err("qos_master_config_type is NULL. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    if (pcie_rc_get_core_id(dev_id, cfg->bitmap[0], &core_id) != 0) {
        drv_pcie_rc_err("No match bitmap, when get qos. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    (void)pcie_rc_get_qos_config_from_core(dev_id, core_id, cfg);

    return 0;
}

STATIC void pcie_rc_set_allow_config_to_core(int dev_id, int core_id, const struct qos_allow_config_type *cfg)
{
    void __iomem *iob_rdbase = NULL;
    void __iomem *iob_wrbase = NULL;
    u32 reg_val = 0;

    /* allow mode = 0 disable read and write */
    if (cfg->qos_allow_mode == PCIE_ALLOW_MODE_DISABLE) {
        iob_rdbase = g_qos_ctrl[dev_id][core_id].reg_rdallow_base;
        pcie_rc_set_reg(iob_rdbase, PCIE_RW_ALLOW_SEL_MASK, PCIE_RW_ALLOW_SEL_OFFSET, PCIE_RW_ALLOW_SEL_DISABLE);

        iob_wrbase = g_qos_ctrl[dev_id][core_id].reg_wrallow_base;
        pcie_rc_set_reg(iob_wrbase, PCIE_RW_ALLOW_SEL_MASK, PCIE_RW_ALLOW_SEL_OFFSET, PCIE_RW_ALLOW_SEL_DISABLE);
        return;
    }

    reg_val |= (cfg->qos_allow_lvl[PCIE_QOS_LVL1] & PCIE_RW_ALLOW_LVL_MASK) << PCIE_RW_ALLOW_LVL1_OFFSET;
    reg_val |= (cfg->qos_allow_lvl[PCIE_QOS_LVL2] & PCIE_RW_ALLOW_LVL_MASK) << PCIE_RW_ALLOW_LVL2_OFFSET;
    reg_val |= (cfg->qos_allow_lvl[PCIE_QOS_LVL3] & PCIE_RW_ALLOW_LVL_MASK) << PCIE_RW_ALLOW_LVL3_OFFSET;
    reg_val |= (PCIE_RW_ALLOW_SEL_ENABLE & PCIE_RW_ALLOW_SEL_MASK) << PCIE_RW_ALLOW_SEL_OFFSET;

    /* allow_config to core */
    if (cfg->qos_allow_ctrl == PCIE_QOS_ALLOW_CTRL_READ) {
        iob_rdbase = g_qos_ctrl[dev_id][core_id].reg_rdallow_base;
        pcie_rc_set_reg(iob_rdbase, PCIE_QOS_REG_MASK, 0, reg_val);
    } else if (cfg->qos_allow_ctrl == PCIE_QOS_ALLOW_CTRL_WRITE) {
        iob_wrbase = g_qos_ctrl[dev_id][core_id].reg_wrallow_base;
        pcie_rc_set_reg(iob_wrbase, PCIE_QOS_REG_MASK, 0, reg_val);
    } else {
        iob_rdbase = g_qos_ctrl[dev_id][core_id].reg_rdallow_base;
        pcie_rc_set_reg(iob_rdbase, PCIE_QOS_REG_MASK, 0, reg_val);

        iob_wrbase = g_qos_ctrl[dev_id][core_id].reg_wrallow_base;
        pcie_rc_set_reg(iob_wrbase, PCIE_QOS_REG_MASK, 0, reg_val);
    }

    return;
}

STATIC int pcie_rc_set_allow_config(int dev_id, const struct qos_allow_config_type *cfg)
{
    int core_id, ret;
    int count = 0;

    if (pcie_rc_qos_check_devid(dev_id) != 0) {
        drv_pcie_rc_err("Invalid dev_id. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    ret = pcie_qos_check_set_allow(dev_id, cfg);
    if (ret != 0) {
        drv_pcie_rc_err("Invalid qos_allow_config. (dev_id=%d)\n", dev_id);
        return ret;
    }
    for (core_id = 0; core_id < PCIE_RC_CORE_NUM; core_id++) {
        if (((1ULL << (u32)core_id) & cfg->bitmap[0]) != 0) {
            pcie_rc_set_allow_config_to_core(dev_id, core_id, cfg);
            count++;
        }
    }

    if (count <= 0) {
        drv_pcie_rc_err("Set qos_allow config failed, no match bitmap. (dev_id=%d; bitmap=0x%llx)\n",
            dev_id, cfg->bitmap[0]);
        return -EINVAL;
    }
    return 0;
}

STATIC void pcie_rc_get_allow_config_from_core(int dev_id, int core_id, struct qos_allow_config_type *cfg)
{
    void __iomem *iob_base = NULL;
    u32 reg_val = 0;

    if (cfg->qos_allow_ctrl == PCIE_QOS_ALLOW_CTRL_READ) {
        iob_base = g_qos_ctrl[dev_id][core_id].reg_rdallow_base;
    } else if (cfg->qos_allow_ctrl == PCIE_QOS_ALLOW_CTRL_WRITE) {
        iob_base = g_qos_ctrl[dev_id][core_id].reg_wrallow_base;
    }

    reg_val = pcie_rc_get_reg(iob_base, PCIE_QOS_REG_MASK, 0);
    cfg->qos_allow_lvl[PCIE_QOS_LVL1] = (reg_val >> PCIE_RW_ALLOW_LVL1_OFFSET) & PCIE_RW_ALLOW_LVL_MASK;
    cfg->qos_allow_lvl[PCIE_QOS_LVL2] = (reg_val >> PCIE_RW_ALLOW_LVL2_OFFSET) & PCIE_RW_ALLOW_LVL_MASK;
    cfg->qos_allow_lvl[PCIE_QOS_LVL3] = (reg_val >> PCIE_RW_ALLOW_LVL3_OFFSET) & PCIE_RW_ALLOW_LVL_MASK;

    return;
}

STATIC int pcie_rc_get_allow_config(int dev_id, struct qos_allow_config_type *cfg)
{
    int core_id;

    if (pcie_rc_qos_check_devid(dev_id) != 0) {
        drv_pcie_rc_err("Invalid dev_id. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }
    if (pcie_qos_check_get_allow(dev_id, cfg) != 0) {
        drv_pcie_rc_err("Invalid qos_allow_config, when get. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    if (pcie_rc_get_core_id(dev_id, cfg->bitmap[0], &core_id) != 0) {
        drv_pcie_rc_err("No match bitmap. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    pcie_rc_get_allow_config_from_core(dev_id, core_id, cfg);
    return 0;
}

STATIC void pcie_rc_set_otsd_config_to_core(int dev_id, int core_id, const struct qos_otsd_config_type *cfg)
{
    u32 reg_val = 0;
    void __iomem *iob_base = g_qos_ctrl[dev_id][core_id].reg_otsd_base;

    if (cfg->otsd_mode == PCIE_OTSD_MODE_DISABLE) {
        pcie_rc_set_reg(iob_base, PCIE_RW_OTSD_EN_MASK, PCIE_WR_OTSD_EN_OFFSET, ~(PCIE_RW_OTSD_EN));
        pcie_rc_set_reg(iob_base, PCIE_RW_OTSD_EN_MASK, PCIE_RD_OTSD_EN_OFFSET, ~(PCIE_RW_OTSD_EN));
        return;
    }

    if (cfg->otsd_mode == PCIE_OTSD_MODE_RDWR) {
        reg_val |= (cfg->otsd_lvl[PCIE_OTSD_RD] & PCIE_RW_OTSD_LVL_MASK) << PCIE_RD_OTSD_LVL_OFFSET;
        reg_val |= (PCIE_RW_OTSD_EN & PCIE_RW_OTSD_EN_MASK) << PCIE_RD_OTSD_EN_OFFSET;
        reg_val |= (cfg->otsd_lvl[PCIE_OTSD_WR] & PCIE_RW_OTSD_LVL_MASK) << PCIE_WR_OTSD_LVL_OFFSET;
        reg_val |= (PCIE_RW_OTSD_EN & PCIE_RW_OTSD_EN_MASK) << PCIE_WR_OTSD_EN_OFFSET;
        pcie_rc_set_reg(iob_base, PCIE_QOS_REG_MASK, 0, reg_val);
    } else {
        reg_val |= (cfg->otsd_lvl[PCIE_OTSD_RD] & PCIE_RW_OTSD_LVL_MASK) << PCIE_RD_OTSD_LVL_OFFSET;
        reg_val |= (PCIE_RW_OTSD_EN & PCIE_RW_OTSD_EN_MASK) << PCIE_RD_OTSD_EN_OFFSET;
        reg_val |= (cfg->otsd_lvl[PCIE_OTSD_RD] & PCIE_RW_OTSD_LVL_MASK) << PCIE_WR_OTSD_LVL_OFFSET;
        reg_val |= (PCIE_RW_OTSD_EN & PCIE_RW_OTSD_EN_MASK) << PCIE_WR_OTSD_EN_OFFSET;
        pcie_rc_set_reg(iob_base, PCIE_QOS_REG_MASK, 0, reg_val);
    }
}

STATIC int pcie_rc_set_otsd_config(int dev_id, const struct qos_otsd_config_type *cfg)
{
    int count = 0;
    int core_id;

    if (pcie_rc_qos_check_devid(dev_id) != 0) {
        drv_pcie_rc_err("Invalid dev_id. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }
    if (pcie_qos_check_set_otsd(dev_id, cfg) != 0) {
        drv_pcie_rc_err("Invalid qos_otsd_config. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    for (core_id = 0; core_id < PCIE_RC_CORE_NUM; core_id++) {
        if (((1ULL << (u32)core_id) & cfg->bitmap[0]) != 0) {
            pcie_rc_set_otsd_config_to_core(dev_id, core_id, cfg);
            count++;
        }
    }

    if (count <= 0) {
        drv_pcie_rc_err("Set qos_otsd config failed, no match bitmap. (dev_id=%d; bitmap=0x%llx)\n",
            dev_id, cfg->bitmap[0]);
        return -EINVAL;
    }

    return 0;
}

STATIC void pcie_rc_get_otsd_config_from_core(int dev_id, int core_id, struct qos_otsd_config_type *cfg)
{
    void __iomem *iob_base = g_qos_ctrl[dev_id][core_id].reg_otsd_base;
    u32 reg_val = 0;

    reg_val = pcie_rc_get_reg(iob_base, PCIE_QOS_REG_MASK, 0);
    cfg->otsd_lvl[PCIE_OTSD_RD] = (reg_val >> PCIE_RD_OTSD_LVL_OFFSET) & PCIE_RW_OTSD_LVL_MASK;
    cfg->otsd_lvl[PCIE_OTSD_WR] = (reg_val >> PCIE_WR_OTSD_LVL_OFFSET) & PCIE_RW_OTSD_LVL_MASK;

    return;
}

STATIC int pcie_rc_get_otsd_config(int dev_id, struct qos_otsd_config_type *cfg)
{
    int core_id;

    if ((dev_id >= PCIE_MAX_DEV) || (dev_id < 0) || (cfg == NULL)) {
        drv_pcie_rc_err("Invalid param. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    if (pcie_rc_get_core_id(dev_id, cfg->bitmap[0], &core_id) != 0) {
        drv_pcie_rc_err("No match bitmap. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    pcie_rc_get_otsd_config_from_core(dev_id, core_id, cfg);

    return 0;
}

STATIC void pcie_rc_suspend_qos_config_from_core(int dev_id, int core_id)
{
    (void)pcie_rc_get_qos_config_from_core(dev_id, core_id, &g_qos_ctrl[dev_id][core_id].qos_config);
    return;
}

STATIC void pcie_rc_suspend_qos_allow_from_core(int dev_id, int core_id)
{
    u32 reg_val;
    void __iomem *iob_base;

    pcie_rc_get_allow_config_from_core(dev_id, core_id, &g_qos_ctrl[dev_id][core_id].rdallow_config);

    /* read reg and set allow_mode of q_qos_ctrl */
    iob_base = g_qos_ctrl[dev_id][core_id].reg_rdallow_base;
    reg_val = pcie_rc_get_reg(iob_base, PCIE_RW_ALLOW_SEL_MASK, PCIE_RW_ALLOW_SEL_OFFSET);
    g_qos_ctrl[dev_id][core_id].rdallow_config.qos_allow_mode = reg_val;

    pcie_rc_get_allow_config_from_core(dev_id, core_id, &g_qos_ctrl[dev_id][core_id].wrallow_config);
    iob_base = g_qos_ctrl[dev_id][core_id].reg_wrallow_base;
    reg_val = pcie_rc_get_reg(iob_base, PCIE_RW_ALLOW_SEL_MASK, PCIE_RW_ALLOW_SEL_OFFSET);
    g_qos_ctrl[dev_id][core_id].rdallow_config.qos_allow_mode = reg_val;

    return;
}

STATIC void pcie_rc_suspend_qos_otsd_from_core(int dev_id, int core_id)
{
    u32 reg_val;
    void __iomem *iob_base;

    pcie_rc_get_otsd_config_from_core(dev_id, core_id, &g_qos_ctrl[dev_id][core_id].ost_config);

    /* read reg and set otsd_mode of q_qos_ctrl */
    iob_base = g_qos_ctrl[dev_id][core_id].reg_otsd_base;
    reg_val = pcie_rc_get_reg(iob_base, PCIE_RW_OTSD_EN_MASK, PCIE_WR_OTSD_EN_OFFSET);
    g_qos_ctrl[dev_id][core_id].ost_config.otsd_mode = reg_val;

    return;
}

STATIC void pcie_rc_dev_suspend_prepare(int dev_id, u32 core_id)
{
    g_qos_ctrl[dev_id][core_id].qos_config.bitmap[0] = 1u << core_id;

    g_qos_ctrl[dev_id][core_id].ost_config.bitmap[0] = 1u << core_id;

    g_qos_ctrl[dev_id][core_id].rdallow_config.bitmap[0] = 1u << core_id;
    g_qos_ctrl[dev_id][core_id].rdallow_config.qos_allow_ctrl = PCIE_QOS_ALLOW_CTRL_READ;

    g_qos_ctrl[dev_id][core_id].wrallow_config.bitmap[0] = 1u << core_id;
    g_qos_ctrl[dev_id][core_id].wrallow_config.qos_allow_ctrl = PCIE_QOS_ALLOW_CTRL_WRITE;
}

int pcie_rc_suspend_qos_config(u32 core_id, struct platform_device *pdev)
{
    int dev_id = 0;

    if (core_id >= PCIE_RC_CORE_NUM) {
        drv_pcie_rc_err("Invalid core_id, when suspend. (core_id=%u)\n", core_id);
        return -EINVAL;
    }

    pcie_rc_dev_suspend_prepare(dev_id, core_id);
    if (g_qos_ctrl[dev_id][core_id].reg_qos_base == NULL) {
        drv_pcie_rc_err("reg_otsd_base is NULL, when suspend. (core_id=%u)\n", core_id);
    } else {
        pcie_rc_suspend_qos_config_from_core(dev_id, (int)core_id);
    }

    if ((g_qos_ctrl[dev_id][core_id].reg_rdallow_base == NULL) ||
        (g_qos_ctrl[dev_id][core_id].reg_wrallow_base == NULL)) {
        drv_pcie_rc_err("reg_rdallow_base or reg_wrallow_base is NULL, when suspend. (core_id=%u)\n", core_id);
    } else {
        pcie_rc_suspend_qos_allow_from_core(dev_id, (int)core_id);
    }

    if (g_qos_ctrl[dev_id][core_id].reg_otsd_base == NULL) {
        drv_pcie_rc_err("reg_rdallow_base or reg_wrallow_base is NULL, when suspend. (core_id=%u)\n", core_id);
    } else {
        pcie_rc_suspend_qos_otsd_from_core(dev_id, (int)core_id);
    }

    return 0;
}

STATIC void pcie_rc_resume_allow_config_to_core(int dev_id, int core_id, const struct qos_allow_config_type *cfg)
{
    void __iomem *iob_base = NULL;

    u32 reg_val = 0;

    reg_val |= (cfg->qos_allow_lvl[PCIE_QOS_LVL1] & PCIE_RW_ALLOW_LVL_MASK) << PCIE_RW_ALLOW_LVL1_OFFSET;
    reg_val |= (cfg->qos_allow_lvl[PCIE_QOS_LVL2] & PCIE_RW_ALLOW_LVL_MASK) << PCIE_RW_ALLOW_LVL2_OFFSET;
    reg_val |= (cfg->qos_allow_lvl[PCIE_QOS_LVL3] & PCIE_RW_ALLOW_LVL_MASK) << PCIE_RW_ALLOW_LVL3_OFFSET;
    reg_val |= (cfg->qos_allow_mode & PCIE_RW_ALLOW_SEL_MASK) << PCIE_RW_ALLOW_SEL_OFFSET;

    if (cfg->qos_allow_ctrl == PCIE_QOS_ALLOW_CTRL_READ) {
        iob_base = g_qos_ctrl[dev_id][core_id].reg_rdallow_base;
    } else {
        iob_base = g_qos_ctrl[dev_id][core_id].reg_wrallow_base;
    }
    pcie_rc_set_reg(iob_base, PCIE_QOS_REG_MASK, 0, reg_val);
    return;
}

STATIC void pcie_rc_resume_otsd_config_to_core(int dev_id, int core_id, const struct qos_otsd_config_type *cfg)
{
    void __iomem *iob_base = NULL;
    u32 reg_val = 0;

    iob_base = g_qos_ctrl[dev_id][core_id].reg_otsd_base;
    reg_val |= (cfg->otsd_lvl[PCIE_OTSD_RD] & PCIE_RW_OTSD_LVL_MASK) << PCIE_RD_OTSD_LVL_OFFSET;
    reg_val |= (cfg->otsd_mode & PCIE_RW_OTSD_EN_MASK) << PCIE_RD_OTSD_EN_OFFSET;
    reg_val |= (cfg->otsd_lvl[PCIE_OTSD_WR] & PCIE_RW_OTSD_LVL_MASK) << PCIE_WR_OTSD_LVL_OFFSET;
    reg_val |= (cfg->otsd_mode & PCIE_RW_OTSD_EN_MASK) << PCIE_WR_OTSD_EN_OFFSET;
    pcie_rc_set_reg(iob_base, PCIE_QOS_REG_MASK, 0, reg_val);
    return;
}

int pcie_rc_resume_qos_config(u32 core_id, struct platform_device *pdev)
{
    int dev_id = 0;

    drv_pcie_rc_info("Start pcie_rc_resume_qos_config. (core_id=%u)\n", core_id);

    if (core_id >= PCIE_RC_CORE_NUM) {
        drv_pcie_rc_err("Invalid core_id, when resume. (core_id=%u)\n", core_id);
        return -EINVAL;
    }

    if (g_qos_ctrl[dev_id][core_id].reg_qos_base == NULL) {
        drv_pcie_rc_err("reg_qos_base is NULL, when resume. (core_id=%u)\n", core_id);
    } else {
        pcie_rc_set_qos_config_to_core(dev_id, (int)core_id, &g_qos_ctrl[dev_id][core_id].qos_config);
    }

    if ((g_qos_ctrl[dev_id][core_id].reg_rdallow_base == NULL) ||
        (g_qos_ctrl[dev_id][core_id].reg_wrallow_base == NULL)) {
        drv_pcie_rc_err("reg_rdallow_base or reg_wrallow_base is NULL, when resume. (core_id=%u)\n", core_id);
    } else {
        pcie_rc_resume_allow_config_to_core(dev_id, (int)core_id, &g_qos_ctrl[dev_id][core_id].rdallow_config);
        pcie_rc_resume_allow_config_to_core(dev_id, (int)core_id, &g_qos_ctrl[dev_id][core_id].wrallow_config);
    }

    if (g_qos_ctrl[dev_id][core_id].reg_otsd_base == NULL) {
        drv_pcie_rc_err("reg_otsd_base is NULL, when resume. (core_id=%u)\n", core_id);
    } else {
        pcie_rc_resume_otsd_config_to_core(dev_id, (int)core_id, &g_qos_ctrl[dev_id][core_id].ost_config);
    }

    drv_pcie_rc_info("pcie_rc_resume_qos_config success. (core_id=%u)\n", core_id);
    return 0;
}

void drv_pcierc_recover_qos_cfg(int dev_id, u32 core_id)
{
    if (core_id >= PCIE_RC_CORE_NUM || dev_id >= PCIE_MAX_DEV) {
        drv_pcie_rc_err("Invalid core_id or dev_id, when recover. (dev_id=%d, core_id=%u)\n", dev_id, core_id);
        return;
    }

    if (g_qos_ctrl[dev_id][core_id].qos_config_saved != true) {
        drv_pcie_rc_info("qos configuration is not saved. (dev_id=%d, core_id=%u)\n", dev_id, core_id);
        return;
    }

    if (g_qos_ctrl[dev_id][core_id].reg_qos_base == NULL) {
        drv_pcie_rc_err("reg_qos_base is NULL, when recover. (dev_id=%d, core_id=%u)\n", dev_id, core_id);
    } else {
        pcie_rc_set_qos_config_to_core(dev_id, (int)core_id, &g_qos_ctrl[dev_id][core_id].qos_config);
    }
    drv_pcie_rc_info("recover qos configuration success. (dev_id=%d, core_id=%u)\n", dev_id, core_id);
}

void hal_kernel_agentdrv_get_qos_func(struct qos_master_node *qos_master)
{
    if (qos_master == NULL) {
        drv_pcie_rc_err("Pcie qos register failed, qos_master_node is NULL.\n");
        return;
    }

    qos_master->set = pcie_rc_set_qos_config;
    qos_master->get = pcie_rc_get_qos_config;
    qos_master->set_allow = pcie_rc_set_allow_config;
    qos_master->get_allow = pcie_rc_get_allow_config;
    qos_master->set_otsd = pcie_rc_set_otsd_config;
    qos_master->get_otsd = pcie_rc_get_otsd_config;
}
EXPORT_SYMBOL(hal_kernel_agentdrv_get_qos_func);

#else

int pcie_rc_qos_init(void)
{
    return 0;
}

#endif /* end of PCIE_RC_UT */