/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-10-25
*/

#include <linux/of_address.h>
#include <linux/of_pci.h>
#include <linux/pci-ecam.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/securec.h>

#include "pcie_ras_config.h"
#ifdef CFG_FEATURE_PCIE_DFX
#include "pcie_rc_dfx.h"
#endif
#include "pcie_rc_drv.h"
#include "pcie_rc_ctrl.h"
#include "pcie_rc_qos.h"

#ifdef CFG_FEATURE_SERDES_H25
#include "sdk_hilink_pub.h"
#endif

#ifndef PCIE_RC_UT

STATIC struct atu_cfg_for_pf g_atu_cfg[PCIE_CORE_NUM][ATU_NUM_FOR_PF] = {
    // core0: ATU0 BAR64 256M, base: 0x830000000; ATU1 BAR32 64M-64K, base: 0xB0000000
    {{0x830000000ULL, 0x100 * 0x100000}, {0xB0000000U, 0x03FF0000U}},
    {{0x850000000ULL, 0x100 * 0x100000}, {0xB4000000U, 0x03FF0000U}},
    {{0x870000000ULL, 0x100 * 0x100000}, {0xB8000000U, 0x03FF0000U}},
    {{0x910000000ULL, 0x100 * 0x100000}, {0xBC000000U, 0x03FF0000U}}
};

STATIC struct ecam_space_cfg g_ecam_cfg[PCIE_CORE_NUM] = {
    {0x8, 0x20000000U, 0x0},
    {0x8, 0x40400000U, 0x4},
    {0x8, 0x60800000U, 0x8},
    {0x9, 0x00c00000U, 0xc}
};

STATIC int g_ao_iomux_perst_offset[PCIE_CORE_NUM] = {
    0x10, 0x14, 0x18, 0x1c
};

STATIC pcie_info g_pcie_link_info[PCIE_CORE_NUM] = {
    {true, 0, CFG_WX4, GEN3, GEN3},
    {false, 1, CFG_WX1, GEN3, GEN3},
    {true, 2, CFG_WX2, GEN2, GEN2},
    {true, 3, CFG_WX1, GEN2, GEN2}
};

STATIC struct crg_reset g_crg_reset[PCIE_CORE_NUM] = {
    {IO_SUB_PCIE0_CTRL0_ICG_EN_REG, IO_SUB_PCIE0_CTRL1_ICG_EN_REG, IO_SUB_PCIE0_CTRL_RESET_DREQ_REG},
    {IO_SUB_PCIE1_CTRL0_ICG_EN_REG, IO_SUB_PCIE1_CTRL1_ICG_EN_REG, IO_SUB_PCIE1_CTRL_RESET_DREQ_REG},
    {IO_SUB_PCIE2_CTRL0_ICG_EN_REG, IO_SUB_PCIE2_CTRL1_ICG_EN_REG, IO_SUB_PCIE2_CTRL_RESET_DREQ_REG},
    {IO_SUB_PCIE3_CTRL0_ICG_EN_REG, IO_SUB_PCIE3_CTRL1_ICG_EN_REG, IO_SUB_PCIE3_CTRL_RESET_DREQ_REG},
};

STATIC struct ltssm_cfg ltssm_rc_cfg_def[PCIE_MAX_CONFIG_NUM] = {
    // PCIE_X4_GEN1_LINKUP
    {
        .max_sup_width = CFG_WX4,
        .max_lk_spd = GEN1,
        .target_speed = GEN1,
        .glb_cfg = {
            .port_en = 1,
            .port_mode = PORT_RP,
            .lane_num = GBL_WX4,
            .linkdown_clr_port_en = 0x0,
            .linkdown_rst_en = 0x1,
        },
        .dl_cfg = {
            .init_fc_send_en_valid = 1,
            .init_fc_send_en_val = 0,  // RP config to 0x0,EP config to 0x1
        },
        .equ_cfg = {
            .adj_en = 1,
            .test_en = 1,
            .reg_phy_eq_fb_sel = 0,
            .reg_8g_hilink_mode_en = 0x1d,
            .gen3_eq_rcv_lock_af_phase13_ctle_apt_en = 0,
            .rcv_speed_pull_up_rx_preset_en = 0,
            .gen3_eq_phase23_ctle_apt_en = 1,
            .rcv_lock_hold_rate = 0,
            .reg_switch_eq_mode_mask = 1,
            .gen4_eq_rcv_lock_af_phase13_ctle_apt_en = 0,
            .gen4_eq_phase23_ctle_apt_en = 1,
            .gen3_eq_phase01_ctle_apt_en = 1,
            .gen4_eq_phase01_ctle_apt_en = 1,
            .gen3_eq_phase23_ffe_ctle_adj_en = 0,
            .gen4_eq_phase23_ffe_ctle_adj_en = 0,
            .reg_16g_hilink_mode_en = 0x1d,
            .reg_16g_ds_use_rx_preset_en = 1,

            .reg_8g_eq_fix_lp_tx_preset = 0x9,
            .reg_16g_eq_fix_lp_tx_preset = 0x9,

            .cfg_phase23_hold_timer = 0x2c,
            .cfg_phase0_hold_timer = 0x14,
            .cfg_rcv_lock_hold_timer = 0x2c,
        },
        .mac_cfg = {
            .auto_speed_change_en = 0x1,
            .first_auto_speed_change_en = 0x1,
            .auto_speed_disable_mask = 0x1,
        },
    },
    // PCIE_X2_GEN3_LINKUP
    {
        .max_sup_width = CFG_WX2,
        .max_lk_spd = GEN3,
        .target_speed = GEN3,
        .glb_cfg = {
            .port_en = 1,
            .port_mode = PORT_RP,
            .lane_num = GBL_WX4,
            .linkdown_clr_port_en = 0x0,
            .linkdown_rst_en = 0x1,
        },
        .dl_cfg = {
            .init_fc_send_en_valid = 1,
            .init_fc_send_en_val = 0,  // RP config to 0x0,EP config to 0x1
        },
        .equ_cfg = {
            .adj_en = 1,
            .test_en = 1,
            .reg_phy_eq_fb_sel = 0,
            .reg_8g_hilink_mode_en = 0x1d,
            .gen3_eq_rcv_lock_af_phase13_ctle_apt_en = 0,
            .rcv_speed_pull_up_rx_preset_en = 0,
            .gen3_eq_phase23_ctle_apt_en = 1,
            .rcv_lock_hold_rate = 0,
            .reg_switch_eq_mode_mask = 1,
            .gen4_eq_rcv_lock_af_phase13_ctle_apt_en = 0,
            .gen4_eq_phase23_ctle_apt_en = 1,
            .gen3_eq_phase01_ctle_apt_en = 1,
            .gen4_eq_phase01_ctle_apt_en = 1,
            .gen3_eq_phase23_ffe_ctle_adj_en = 0,
            .gen4_eq_phase23_ffe_ctle_adj_en = 0,
            .reg_16g_hilink_mode_en = 0x1d,
            .reg_16g_ds_use_rx_preset_en = 1,

            .reg_8g_eq_fix_lp_tx_preset = 0x4,
            .reg_16g_eq_fix_lp_tx_preset = 0x4,

            .cfg_phase23_hold_timer = 0x2c,
            .cfg_phase0_hold_timer = 0x14,
            .cfg_rcv_lock_hold_timer = 0x2c,
        },
        .mac_cfg = {
            .auto_speed_change_en = 0x1,
            .first_auto_speed_change_en = 0x1,
            .auto_speed_disable_mask = 0x1,
        },
    }
};

// access PCIE AP and Global
STATIC void drv_pcierc_ap_write(u32 core_id, u32 offset, u32 reg, u32 value)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + offset + reg;
    writel(value, addr);
}

STATIC u32 drv_pcierc_ap_read(u32 core_id, u32 offset, u32 reg)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + offset + reg;
    return readl(addr);
}

// access PCIE Core Global
STATIC void drv_pcierc_core_write(u32 core_id, u32 reg, u32 value)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_CORE_REG_BASE + reg;
    writel(value, addr);
}

STATIC u32 drv_pcierc_core_read(u32 core_id, u32 reg)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_CORE_REG_BASE + reg;
    return readl(addr);
}

// access PCIE DL
STATIC void drv_pcierc_dl_write(u32 core_id, u32 port, u32 reg, u32 value)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_DL_REG_BASE + ((u64)port * 0x1000UL) + reg;
    writel(value, addr);
}

STATIC u32 drv_pcierc_dl_read(u32 core_id, u32 port, u32 reg)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_DL_REG_BASE + ((u64)port * 0x1000UL) + reg;
    return readl(addr);
}

// access PCIE TL
STATIC void drv_pcierc_tl_write(u32 core_id, u32 port, u32 reg, u32 value)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_TL_REG_BASE + ((u64)port * 0x1000UL) + reg;
    writel(value, addr);
}

STATIC u32 drv_pcierc_tl_read(u32 core_id, u32 port, u32 reg)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_TL_REG_BASE + ((u64)port * 0x1000UL) + reg;
    return readl(addr);
}

STATIC void drv_pcierc_tl_core_pf_write(u32 core_id, u32 pf_num, u32 reg, u32 value)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_TL_CORE_REG_BASE + 0x1000 + (0x200U * pf_num) + reg;
    writel(value, addr);
}

STATIC void drv_pcierc_cfg_write(u32 core_id, u32 port, u32 reg, u32 value)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_EPF_CFGSPACE_BASE + ((u64)port * 0x1000UL) + reg;
    writel(value, addr);
}

STATIC u32 drv_pcierc_cfg_read(u32 core_id, u32 port, u32 reg)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_EPF_CFGSPACE_BASE + ((u64)port * 0x1000UL) + reg;
    return readl(addr);
}

// access PCIE MAC
void drv_pcierc_mac_write(u32 core_id, u32 port, u32 reg, u32 value)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_MAC_REG_BASE + ((u64)port * 0x1000UL) + reg;
    writel(value, addr);
}

u32 drv_pcierc_mac_read(u32 core_id, u32 port, u32 reg)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_MAC_REG_BASE + ((u64)port * 0x1000UL) + reg;
    return readl(addr);
}

STATIC void drv_pcierc_pcs_write(u32 core_id, u32 port, u32 reg, u32 value)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_PCS_REG_BASE + ((u64)port * 0x1000UL) + reg;
    writel(value, addr);
}

STATIC u32 drv_pcierc_pcs_read(u32 core_id, u32 port, u32 reg)
{
    void __iomem *addr = pcie_rc_get_apb_base(core_id) + PCIE_PCS_REG_BASE + ((u64)port * 0x1000UL) + reg;
    return readl(addr);
}

STATIC void drv_pcierc_interrupt_cfg(u32 core_id)
{
    u32 tmp;
    // ******close pcie RAS interrupt enable***********
    // PCIE_CE_ENA.pcie_port_ce_ena bit15:0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_CE_ENA);
    tmp &= ~0xFFFFU;
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_CE_ENA, tmp);
    // AP_GLOBAL_REG.PCIE_UNF_ENA.pcie_port_unf_ena bit15:0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_UNF_ENA);
    tmp &= ~0xFFFFU;
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_UNF_ENA, tmp);
    // AP_GLOBAL_REG.PCIE_UF_ENA.pcie_port_uf_ena bit 15:0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_UF_ENA);
    tmp &= ~0xFFFFU;
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_UF_ENA, tmp);

    // *******close controller local interrupt enable**********
    // AP_GLOBAL_REG.PCIE_AP_CE_ENA.pcie_ap_ce_ena bit9:0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_AP_CE_ENA);
    tmp &= ~0x1FFU;
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_AP_CE_ENA, tmp);
    // AP_GLOBAL_REG.PCIE_AP_UNF_ENA.pcie_ap_unf_ena bit9:0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_AP_UNF_ENA);
    tmp &= ~0x1FFU;
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_AP_UNF_ENA, tmp);
    // AP_GLOBAL_REG.PCIE_AP_UF_ENA.pcie_ap_uf_ena bit9:0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_AP_UF_ENA);
    tmp &= ~0x1FFU;
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_AP_UF_ENA, tmp);
    // AP_GLOBAL_REG.PCIE_AP_NI_ENA.pcie_ap_ni_ena bit9:0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_AP_NI_ENA);
    tmp &= ~0x1FFU;
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_AP_NI_ENA, tmp);
    // AP_GLOBAL_REG.PCIE_CORE_CE_ENA.pcie_core_ce_ena bit 0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_CORE_CE_ENA);
    tmp &= (u32)~BIT(0);
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_CORE_CE_ENA, tmp);
    // AP_GLOBAL_REG.PCIE_CORE_UNF_ENA.pcie_core_unf_ena bit0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_CORE_UNF_ENA);
    tmp &= (u32)~BIT(0);
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_CORE_UNF_ENA, tmp);
    // AP_GLOBAL_REG.PCIE_CORE_UF_ENA.pcie_core_uf_ena bit0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_CORE_UF_ENA);
    tmp &= (u32)~BIT(0);
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_CORE_UF_ENA, tmp);
    // AP_GLOBAL_REG.PCIE_CORE_NI_ENA.pcie_core_ni_ena bit0 to 0x0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_CORE_NI_ENA);
    tmp &= (u32)~BIT(0);
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_CORE_NI_ENA, tmp);

    // *******close MSI and INTx interrupt Mask**********
    // AP_GLOBAL_REG.PCIE_MSI_MASK.pcie_msi_mask bit31:0 to 0xFFFFFFF
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_MSI_MASK);
    tmp |= 0xFFFFFFFFU;
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PCIE_MSI_MASK, tmp);
    // AP_GLOBAL_REG.PORT_INTX_ASSERT_MASK.port_intx_assert_mask Bit51:0 to 0xFFFF
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PORT_INTX_ASSERT_MASK);
    tmp |= 0xFFFFU;
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PORT_INTX_ASSERT_MASK, tmp);
    // AP_GLOBAL_REG.PORT_INTX_DEASSERT_MASK.port_intx_deassert_mask bit15:0 to 0xFFFF
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, PORT_INTX_DEASSERT_MASK);
    tmp |= 0xFFFFU;
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, PORT_INTX_DEASSERT_MASK, tmp);
}

STATIC void drv_pcierc_port_reset(u32 core_id, u32 port, u32 para_index)
{
    u32 tmp;
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_IDLE);
    // AP_GLOBAL_REG.AP_IDLE.ap_idle is 0x1 ,begin to reset
    if (tmp == 0x1) {
        drv_pcierc_interrupt_cfg(core_id);
        // *****Close port EN
        // CORE_GLOBAL_CTRL_REG.PORT_EN.port_en bit 15:0 to 0x0
        tmp = drv_pcierc_core_read(core_id, PORT_EN_REG);
        tmp &= ~(u32)(0x1U << PORTID_2_PORTNUM(port));
        drv_pcierc_core_write(core_id, PORT_EN_REG, tmp);
        // ***reset AP AXI
        // AP_GLOBAL_REG.AP_AXI_SYN_RST.ap_axi_syn_rst bit0 to 0x1
        tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_AXI_SYN_RST);
        tmp |= (u32)BIT(0);
        drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_AXI_SYN_RST, tmp);
        // ***reset AP register
        // AP_GLOBAL_REG.AP_APB_SYN_RST.ap_apb_syn_rst  bit0 to 0x1
        tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_APB_SYN_RST);
        tmp |= (u32)BIT(0);
        drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_APB_SYN_RST, tmp);

        // ******soft rest all port logic******
        // CORE_GLOBAL_CLKRST_CTRL_REG.PORT_RESET.port_sft_rst bit15:0 to 0xFFFF
        tmp = drv_pcierc_core_read(core_id, PORT_RESET_REG);
        tmp |= 0xFFFFU;
        drv_pcierc_core_write(core_id, PORT_RESET_REG, tmp);
        // ******soft reset CORE common logic******
        // CORE_GLOBAL_CLKRST_CTRL_REG.PCIE_CORE_TL_COM_SFT_RST.core_tl_com_sft_rst bit0 to 0x1
        tmp = drv_pcierc_core_read(core_id, PCIE_CORE_TL_COM_SFT_RST);
        tmp |= (u32)BIT(0);
        drv_pcierc_core_write(core_id, PCIE_CORE_TL_COM_SFT_RST, tmp);
        // CORE_GLOBAL_CLKRST_CTRL_REG.PCIE_CORE_PHY_COM_SFT_RST.core_phy_com_sft_rst bit0 to 0x1
        tmp = drv_pcierc_core_read(core_id, PCIE_CORE_PHY_COM_SFT_RST);
        tmp |= (u32)BIT(0);
        drv_pcierc_core_write(core_id, PCIE_CORE_PHY_COM_SFT_RST, tmp);
        // ******reset cfgspace sticky register******
        // CORE_GLOBAL_CLKRST_CTRL_REG.PHY_RESET.port_sticky_sft_rst bit15:0 to 0xFFFF
        tmp = drv_pcierc_core_read(core_id, PHY_REST_REG);
        tmp |= 0xFFFFU;
        drv_pcierc_core_write(core_id, PHY_REST_REG, tmp);
        // ******reset PCS logic******
        // CORE_GLOBAL_CLKRST_CTRL_REG.PCS_SOFT_REST bit31:0 to 0xFFFFFFFF
        drv_pcierc_core_write(core_id, PCS_SOFT_REST_REG, 0xFFFFFFFFU);
    }
}

STATIC void drv_pcierc_port_dereset(u32 core_id, u32 port, u32 para_index)
{
    u32 tmp;
    u32 i;

    // core_phy_com_sft_rst dereset
    tmp = drv_pcierc_core_read(core_id, PCIE_CORE_PHY_COM_SFT_RST);
    tmp &= (u32)~BIT(0);
    drv_pcierc_core_write(core_id, PCIE_CORE_PHY_COM_SFT_RST, tmp);

    // pcs sft dereset
    tmp = drv_pcierc_core_read(core_id, PCS_SOFT_REST_REG);
    // PCS_SOFT_REST.pcs_com_sft_rst dreset bit31~bit24 to 0x0
    tmp &= 0x00ffffffu;
    // PCS_SOFT_REST.pcs_sft_rst dreset bit23~bit16 to 0x0
    tmp &= 0xff00ffffu;
    drv_pcierc_core_write(core_id, PCS_SOFT_REST_REG, tmp);

    // pcs lane dereset
    tmp = drv_pcierc_core_read(core_id, PCS_SOFT_REST_REG);
    for (i = 0; i < ltssm_rc_cfg_def[para_index].max_sup_width; i++) {
        tmp &= ~(u32)(1U << (PORTID_2_PORTNUM(port) + i));
    }
    drv_pcierc_core_write(core_id, PCS_SOFT_REST_REG, tmp);

    // tl rst dreset bit0
    drv_pcierc_core_write(core_id, PCIE_CORE_TL_COM_SFT_RST, 0x0);

    // logic dereset
    tmp = drv_pcierc_core_read(core_id, PORT_RESET_REG);
    tmp &= ~(u32)(1U << PORTID_2_PORTNUM(port));
    tmp &= ~(u32)((u32)(1U << PORTID_2_PORTNUM(port)) << TWO_BYTE_WIDTH);
    drv_pcierc_core_write(core_id, PORT_RESET_REG, tmp);

    // sticky dereset
    tmp = drv_pcierc_core_read(core_id, PHY_REST_REG);
    tmp &= ~(u32)(1U << PORTID_2_PORTNUM(port));
    drv_pcierc_core_write(core_id, PHY_REST_REG, tmp);

    usleep_range(DELAY_MIN_MSECOND, DELAY_MAX_MSECOND);

    // AP AXI and AP APB dreset
    // AP_AXI_SYN_RST.ap_axi_syn_rst dreset bit0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_AXI_SYN_RST);
    tmp &= (u32)~BIT(0);
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_AXI_SYN_RST, tmp);

    // AP_APB_SYN_RST.ap_apb_syn_rst dreset bit0
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_APB_SYN_RST);
    tmp &= (u32)~BIT(0);
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_APB_SYN_RST, tmp);
}

STATIC void drv_pcierc_port_global_cfg(u32 core_id, u32 port, u32 para_index)
{
    u32 tmp;

    // set port mode
    // config prot work mode(GLB_PCIEC_MODE_SEL.pciec_mode_sel(Bit15~Bit0) ) RC
    tmp = drv_pcierc_core_read(core_id, GLB_PCIEC_MODE_SEL);

    if (ltssm_rc_cfg_def[para_index].glb_cfg.port_mode == PORT_RP) {
        tmp |= (u32)(1U << PORTID_2_PORTNUM(port));
    } else {
        tmp &= ~(u32)(1U << PORTID_2_PORTNUM(port));
    }

    drv_pcierc_core_write(core_id, GLB_PCIEC_MODE_SEL, tmp);
}

STATIC void drv_pcierc_ep_cfg_def_cfg(u32 core_id, u32 port, u32 para_index)
{
    U_LINK_CAPBILITY ulLinkCapbility;
    U_LINK_CTRL_STATUS2 ulLinkCtrlStatus2;

    ulLinkCapbility.u32 = drv_pcierc_cfg_read(core_id, port, PCIe_HIPCIEC_EPF_CFGSPACE_LINK_CAPBILITY_REG);
    ulLinkCapbility.bits.max_link_speed = ltssm_rc_cfg_def[para_index].max_lk_spd;     // set max link speed
    ulLinkCapbility.bits.max_link_width = ltssm_rc_cfg_def[para_index].max_sup_width;  // set max link width
    drv_pcierc_cfg_write(core_id, port, PCIe_HIPCIEC_EPF_CFGSPACE_LINK_CAPBILITY_REG, ulLinkCapbility.u32);

    // set target link speed
    ulLinkCtrlStatus2.u32 = drv_pcierc_cfg_read(core_id, port, PCIe_HIPCIEC_EPF_CFGSPACE_LINK_CTRL_STATUS2_REG);
    ulLinkCtrlStatus2.bits.target_link_speed = ltssm_rc_cfg_def[para_index].target_speed;
    drv_pcierc_cfg_write(core_id, port, PCIe_HIPCIEC_EPF_CFGSPACE_LINK_CTRL_STATUS2_REG, ulLinkCtrlStatus2.u32);
}

STATIC void drv_pcierc_equalization_cfg(u32 core_id, u32 port, u32 para_index)
{
    U_MAC_REG_DC_BALANCE_DISABLE uDcBalanceDisable;
    U_MAC_REG_EQ_DISABLE uEqDisable;
    U_MAC_REG_ADJ_HILINK_MODE_EN uAdjHilinkModeEn;
    U_MAC_REG_EQ_FIX_LP_TX_PRESET uEqFixLpTxPreset;

    if ((ltssm_rc_cfg_def[para_index].equ_cfg.adj_en) == 0x0) {
        uDcBalanceDisable.u32 = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_DC_BALANCE_DISABLE_REG);
        uDcBalanceDisable.bits.reg_dc_balance_disable = 1;
        drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_DC_BALANCE_DISABLE_REG, uDcBalanceDisable.u32);

        uEqDisable.u32 = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_EQ_DISABLE_REG);
        uEqDisable.bits.reg_ds_eq_p23_disable = 1;
        uEqDisable.bits.reg_equalization_disable = 1;
        drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_EQ_DISABLE_REG, uEqDisable.u32);
    }

    uAdjHilinkModeEn.u32 = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_ADJ_HILINK_MODE_EN_REG);
    uAdjHilinkModeEn.bits.reg_8g_hilink_mode_en = ltssm_rc_cfg_def[para_index].equ_cfg.reg_8g_hilink_mode_en;
    uAdjHilinkModeEn.bits.gen3_eq_rcv_lock_af_phase13_ctle_apt_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.gen3_eq_rcv_lock_af_phase13_ctle_apt_en;
    uAdjHilinkModeEn.bits.rcv_speed_pull_up_rx_preset_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.rcv_speed_pull_up_rx_preset_en;
    uAdjHilinkModeEn.bits.gen3_eq_phase23_ctle_apt_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.gen3_eq_phase23_ctle_apt_en;
    uAdjHilinkModeEn.bits.rcv_lock_hold_rate = ltssm_rc_cfg_def[para_index].equ_cfg.rcv_lock_hold_rate;
    uAdjHilinkModeEn.bits.reg_switch_eq_mode_mask = ltssm_rc_cfg_def[para_index].equ_cfg.reg_switch_eq_mode_mask;
    uAdjHilinkModeEn.bits.gen4_eq_rcv_lock_af_phase13_ctle_apt_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.gen4_eq_rcv_lock_af_phase13_ctle_apt_en;
    uAdjHilinkModeEn.bits.gen4_eq_phase23_ctle_apt_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.gen4_eq_phase23_ctle_apt_en;
    uAdjHilinkModeEn.bits.gen3_eq_phase01_ctle_apt_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.gen3_eq_phase01_ctle_apt_en;
    uAdjHilinkModeEn.bits.gen4_eq_phase01_ctle_apt_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.gen4_eq_phase01_ctle_apt_en;
    uAdjHilinkModeEn.bits.gen3_eq_phase23_ffe_ctle_adj_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.gen3_eq_phase23_ffe_ctle_adj_en;
    uAdjHilinkModeEn.bits.gen4_eq_phase23_ffe_ctle_adj_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.gen4_eq_phase23_ffe_ctle_adj_en;
    uAdjHilinkModeEn.bits.reg_16g_hilink_mode_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.reg_16g_hilink_mode_en;
    uAdjHilinkModeEn.bits.reg_16g_ds_use_rx_preset_en =
        ltssm_rc_cfg_def[para_index].equ_cfg.reg_16g_ds_use_rx_preset_en;
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_ADJ_HILINK_MODE_EN_REG, uAdjHilinkModeEn.u32);

    uEqFixLpTxPreset.u32 = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_EQ_FIX_LP_TX_PRESET_REG);
    uEqFixLpTxPreset.bits.reg_8g_eq_fix_lp_tx_preset =
        ltssm_rc_cfg_def[para_index].equ_cfg.reg_8g_eq_fix_lp_tx_preset;
    uEqFixLpTxPreset.bits.reg_16g_eq_fix_lp_tx_preset =
        ltssm_rc_cfg_def[para_index].equ_cfg.reg_16g_eq_fix_lp_tx_preset;
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_EQ_FIX_LP_TX_PRESET_REG, uEqFixLpTxPreset.u32);
}

STATIC void drv_pcierc_mac_lpbk_disable(u32 core_id, u32 port, u32 para_index)
{
    U_MAC_ENTER_LPBK_DISABLE uMacRnterLpbkDisable;

    uMacRnterLpbkDisable.u32 = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_ENTER_LPBK_DISABLE_REG);
    uMacRnterLpbkDisable.bits.auto_speed_change_en = ltssm_rc_cfg_def[para_index].mac_cfg.auto_speed_change_en;
    uMacRnterLpbkDisable.bits.first_auto_speed_change_en =
        ltssm_rc_cfg_def[para_index].mac_cfg.first_auto_speed_change_en;
    uMacRnterLpbkDisable.bits.auto_speed_disable_mask = ltssm_rc_cfg_def[para_index].mac_cfg.auto_speed_disable_mask;

    uMacRnterLpbkDisable.bits.disable_enter_hotreset = ltssm_rc_cfg_def[para_index].mac_cfg.disable_enter_hotreset;
    uMacRnterLpbkDisable.bits.disable_enter_loopback = ltssm_rc_cfg_def[para_index].mac_cfg.disable_enter_loopback;
    uMacRnterLpbkDisable.bits.disable_enter_disable = ltssm_rc_cfg_def[para_index].mac_cfg.disable_enter_disable;
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_ENTER_LPBK_DISABLE_REG, uMacRnterLpbkDisable.u32);
}

STATIC void drv_pcierc_mac_err_retrain_enable(u32 core_id, u32 port, u32 para_index)
{
    U_MAC_FRAMING_ERR_CTRL uMacFramingErrCtrl;
    u32 lane_num = ltssm_rc_cfg_def[para_index].mac_cfg.lane_num;
    u32 i;
    u32 val;

    uMacFramingErrCtrl.u32 = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_FRAMING_ERR_CTRL_REG);
    uMacFramingErrCtrl.bits.reg_framing_err_retrain_en = 1;
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_FRAMING_ERR_CTRL_REG, uMacFramingErrCtrl.u32);

    val = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_RX_ERR_CHECK_EN_REG);
    for (i = 0; i < lane_num; i++) {
        val |= (u32)(PORTID_2_PORTNUM(port) + i);
    }

    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_RX_ERR_CHECK_EN_REG, val);
}

STATIC void drv_pcierc_port_pcs_config(u32 core_id, u32 port, u32 para_index)
{
    u32 lane, value;

    // select use message-bus
    for (lane = 0; lane < ltssm_rc_cfg_def[para_index].max_sup_width; lane++) {
        drv_pcierc_pcs_write(core_id, port, PCS_LANE_REG_BASE + PCS_MSG_PIN_EN + (PCS_LANE_OFFSET * lane), 0x0);
    }

    // PCS_GLB_REG.POWER_CHANGE_1US.cfg_txelecidle_dly_num(bit27~bit24) to 0x1
    value = drv_pcierc_pcs_read(core_id, port, PCS_POWER_CHANGE_1US);
    value &= PCS_TXE_DLY_NUM_MASK;
    value |= (u32)BIT(24);
    drv_pcierc_pcs_write(core_id, port, PCS_POWER_CHANGE_1US, value);

    // PCS_LANE_REG.PCS_MISC_CTRL_PARA_0.cfg_g3_high_txidle_delay_en (bit11) to 0x1
    for (lane = 0; lane < ltssm_rc_cfg_def[para_index].max_sup_width; lane++) {
        value = drv_pcierc_pcs_read(core_id, port, PCS_LANE_REG_BASE + PCS_MISC_CTRL_PARA_0 + PCS_LANE_OFFSET * lane);
        value |= (u32)BIT(11);
        drv_pcierc_pcs_write(core_id, port, PCS_LANE_REG_BASE + PCS_MISC_CTRL_PARA_0 + PCS_LANE_OFFSET * lane, value);
    }
}

STATIC void drv_pcierc_port_mac_cfg(u32 core_id, u32 port, u32 para_index)
{
    u32 val;
    drv_pcierc_mac_lpbk_disable(core_id, port, para_index);
    /* set framing err retrain enable */
    drv_pcierc_mac_err_retrain_enable(core_id, port, para_index);

    // MAC_REG.MAC_CFG_RCV_HOLD_EN.reg_rcv_lock_hold_en bit0 to 0x1
    val = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_CFG_RCV_HOLD_EN_REG);
    val |= (u32)BIT(0);
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_CFG_RCV_HOLD_EN_REG, val);

    // RC mode MAC_REG_EQ_PHASE23_TIMEOUT_VAL. reg_eq_phase3_timeout_val bit15~8 to 0x24
    val = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_EQ_PHASE23_TIMEOUT_VAL);
    val &= 0xffff00ffU;
    val |= 0x2400U;
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_EQ_PHASE23_TIMEOUT_VAL, val);

    // MAC_TXELEIDLE_TX. reg_rcv_cfg_txildle_en to 0x0(bit24)
    val = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_TXELEIDLE_TM);
    val &= (u32)~BIT(24);
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_TXELEIDLE_TM, val);
    // MAC_REG. MAC_ENTER_LPBK_DISABLE.reg_pipe_data_width_sel_gen3 to 0x2'b10(bit21~bit20)
    val = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_ENTER_LPBK_DISABLE_REG);
    val &= ~0x00300000U;
    val |= (u32)BIT(21);
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_ENTER_LPBK_DISABLE_REG, val);

    // set pipe mode to message bus
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_PIPE_MODE_SEL, 0x0);

    // change ffe(pre, main, post) of preset2 to (0,23,5)
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_PRESET2, PCIe_MAC_PRESET2_FFE_CONFIG);

    val = drv_pcierc_mac_read(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_TARGET_LINK_WIDTH);
    val |= 0x4U;
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_TARGET_LINK_WIDTH, val);
}

STATIC void drv_pcierc_port_dl_cfg(u32 core_id, u32 port, u32 para_index)
{
    if (ltssm_rc_cfg_def[para_index].dl_cfg.init_fc_send_en_valid != 0x0) {
        drv_pcierc_dl_write(core_id,
            port,
            PCIe_HIPCIEC_DL_REG_INIT_FC_SEND_EN_REG,
            ltssm_rc_cfg_def[para_index].dl_cfg.init_fc_send_en_val);
    }
}

STATIC void drv_pcierc_port_tl_cfg(u32 core_id, u32 port)
{
    u32 val;
    // TL Credit CFG
    // EPF_CFGSPACE. PCIHDR_ID.vendor_id(19e5) and EPF_CFGSPACE. PCIHDR_ID.device_id(d107)
    drv_pcierc_cfg_write(core_id, port, PCIe_HIPCIEC_EPF_CFGSPACE_PCIHDR_ID_REG, 0xd10719e5u);

    // RP_CFGSPACE.LINK_CTRL_STATUS.aspm_ctrl[1]  bit1:0 to 0x1
    val = drv_pcierc_cfg_read(core_id, port, PCIe_HIPCIEC_TL_REG_TL_TX_CTRL_REG);
    val |= 0x1U;
    drv_pcierc_cfg_write(core_id, port, PCIe_HIPCIEC_TL_REG_TL_TX_CTRL_REG, val);

    // TL_REG.TL_TX_DFX_CTRL.tl_tx_range_order to 0x0(bit17)
    val = drv_pcierc_tl_read(core_id, port, PCIe_HIPCIEC_TL_REG_TL_TX_DFX_CTRL_REG);
    val &= (u32)~BIT(17);
    drv_pcierc_tl_write(core_id, port, PCIe_HIPCIEC_TL_REG_TL_TX_DFX_CTRL_REG, val);
}

STATIC void drv_pcierc_port_ap_cfg(u32 core_id, u32 port, u32 para_index)
{
    u32 val;

    // ECAM space
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, IOB_ECAM_BASE_ADDR_L, g_ecam_cfg[core_id].base_addr_l);
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, IOB_ECAM_BASE_ADDR_H, g_ecam_cfg[core_id].base_addr_h);
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, IOB_TX_ECAM_CONTROL0, g_ecam_cfg[core_id].start_bus);
    // ECAM enable (bit4 to 0x1)
    val = drv_pcierc_ap_read(core_id, AP_IOB_TX_REG_BASE, IOB_TX_ECAM_CONTROL1);
    val |= (u32)BIT(4);
    // 16 bus bit (n=bit3:0; bus_num=2^n)
    val &= 0xFFFFFFF0U;
    val |= (u32)BIT(2);
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, IOB_TX_ECAM_CONTROL1, val);

    // config AXIS_CRSs tatus:AP_IOB_TX_REG.IOB_TX_AXIS_CONTROL1.iob_tx_axis_control_1[15:14] to 0x2
    val = drv_pcierc_ap_read(core_id, AP_IOB_TX_REG_BASE, IOB_TX_AXIS_CONTROL1);
    val &= 0xFFFF3FFFU;
    // bit 15:14 to 0x2
    val |= (u32)BIT(15);
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, IOB_TX_AXIS_CONTROL1, val);

    // axi timeout threshold
    val = drv_pcierc_ap_read(core_id, AP_IOB_RX_REG_BASE, IOB_RX_AML_QOS_CTRL);
    val |= 0x3FF00U;
    drv_pcierc_ap_write(core_id, AP_IOB_RX_REG_BASE, IOB_RX_AML_QOS_CTRL, val);
}

STATIC void drv_pcierc_port_init(u32 core_id, u32 port, u32 para_index)
{
    u32 tmp;

    drv_pcierc_port_reset(core_id, port, para_index);
    drv_pcierc_port_dereset(core_id, port, para_index);
    drv_pcierc_port_global_cfg(core_id, port, para_index);
    drv_pcierc_ep_cfg_def_cfg(core_id, port, para_index);
    drv_pcierc_equalization_cfg(core_id, port, para_index);
    drv_pcierc_port_pcs_config(core_id, port, para_index);
    drv_pcierc_port_mac_cfg(core_id, port, para_index);
    drv_pcierc_port_dl_cfg(core_id, port, para_index);
    drv_pcierc_port_tl_cfg(core_id, port);
    drv_pcierc_port_ap_cfg(core_id, port, para_index);

    // trace config
    drv_pcierc_mac_write(core_id, port, PCIe_HIPCIEC_MAC_REG_MAC_REG_LTSSM_TRACER_INPUT_REG, 0x1);

    // clear port-en
    tmp = drv_pcierc_core_read(core_id, LINK_DOWN_CLR_PORT_EN);

    if (ltssm_rc_cfg_def[para_index].glb_cfg.linkdown_clr_port_en == 0x1) {
        tmp |= (u32)(1U << (PORTID_2_PORTNUM(port)));
    } else {
        tmp &= ~(u32)(1U << (PORTID_2_PORTNUM(port)));
    }

    drv_pcierc_core_write(core_id, LINK_DOWN_CLR_PORT_EN, 0);
    tmp = drv_pcierc_core_read(core_id, LINK_DOWN_CLR_PORT_EN);
    drv_pcie_rc_info("Get pcie linkdown_clear_port_enable value.(port_enable_value=0x%x).\n", tmp);
    // reset port en
    tmp = drv_pcierc_core_read(core_id, LINK_DOWN_RST_EN);

    if (ltssm_rc_cfg_def[para_index].glb_cfg.linkdown_rst_en == 0x1) {
        tmp |= (u32)(1U << (PORTID_2_PORTNUM(port)));
    } else {
        tmp &= ~(u32)(1U << (PORTID_2_PORTNUM(port)));
    }

    drv_pcierc_core_write(core_id, LINK_DOWN_RST_EN, tmp);
}

STATIC void drv_pcierc_tx_atu_cfg(u32 core_id, u32 atu_id, u64 size, u64 src_addr, u32 mode)
{
    u64 tgt_addr = src_addr;

    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, (0x20U * atu_id) + 0x0U, 0x0);
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, (0x20U * atu_id) + 0x8U,
                        (u32)(size >> BIT_OPERATE_MOVE_32BIT));
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, (0x20U * atu_id) + 0xCU,
                        (u32)(size & 0xFFFFFFFFU));          // size
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, (0x20U * atu_id) + 0x14U,
                        (u32)(src_addr >> BIT_OPERATE_MOVE_32BIT)); // src addrh
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, (0x20U * atu_id) + 0x10U,
                        (u32)(src_addr & 0xFFFFFFFFU));       // source addrl
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, (0x20U * atu_id) + 0x1CU,
                        (u32)(tgt_addr >> BIT_OPERATE_MOVE_32BIT)); // tag addrh
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, (0x20U * atu_id) + 0x18U,
                        (u32)(tgt_addr & 0xFFFFFFFFU));       // target addrl
    drv_pcierc_ap_write(core_id, AP_IOB_TX_REG_BASE, (0x20U * atu_id) + 0x0U, mode);
}

STATIC void drv_pcierc_atu_cfg_for_pf(u32 core_id, u32 port, int mode)
{
    u32 tmp;
    u32 atu_index = 0;
    u32 pf_num = 0;

    // config bar0~bar5 mask to 0x0 aviod wasting space
    drv_pcierc_tl_core_pf_write(core_id, pf_num, PFN_BAR0_MASK, 0x0);
    drv_pcierc_tl_core_pf_write(core_id, pf_num, PFN_BAR1_MASK, 0x0);
    drv_pcierc_tl_core_pf_write(core_id, pf_num, PFN_BAR2_MASK, 0x0);
    drv_pcierc_tl_core_pf_write(core_id, pf_num, PFN_BAR3_MASK, 0x0);
    drv_pcierc_tl_core_pf_write(core_id, pf_num, PFN_BAR4_MASK, 0x0);
    drv_pcierc_tl_core_pf_write(core_id, pf_num, PFN_BAR5_MASK, 0x0);

    // ATU0
    drv_pcierc_tx_atu_cfg(core_id,
        atu_index++,
        g_atu_cfg[core_id][ATU_INDEX_0].size,
        g_atu_cfg[core_id][ATU_INDEX_0].offset,
        (u32)mode);
    // ATU1
    drv_pcierc_tx_atu_cfg(core_id,
        atu_index++,
        g_atu_cfg[core_id][ATU_INDEX_1].size,
        g_atu_cfg[core_id][ATU_INDEX_1].offset,
        (u32)mode);

    // device id
    tmp = drv_pcierc_cfg_read(core_id, port, PCIe_HIPCIEC_EPF_CFGSPACE_PCIHDR_ID_REG);
    tmp &= ~0xFFFF0000U;
    tmp |= 0xd1070000U;
    drv_pcierc_cfg_write(core_id, port, PCIe_HIPCIEC_EPF_CFGSPACE_PCIHDR_ID_REG, tmp);
}

STATIC void drv_pcierc_msi_int_cfg(u32 core_id, u32 port)
{
    u32 val;
    // confg ITS addr (0xD2000000)
    // IOB_RX_MSI_INT_ADDR_HIGH.iob_rx_msi_int_addr_high to 0x0
    drv_pcierc_ap_write(core_id, AP_IOB_RX_REG_BASE, 0x1044, 0x0);
    // IOB_RX_MSI_INT_ADDR_LOW.iob_rx_msi_int_addr_low to 0xD2010040
    drv_pcierc_ap_write(core_id, AP_IOB_RX_REG_BASE, 0x1048, ITS_REG_BASE + GITS_TRANSLATER_L_OFFSET);

    // config IOB_RX_MSI_INT_CTRL.iob_rx_msi_int_enable to 0x1
    val = drv_pcierc_ap_read(core_id, AP_IOB_RX_REG_BASE, 0x1040);
    val |= (u32)BIT(0);
    drv_pcierc_ap_write(core_id, AP_IOB_RX_REG_BASE, 0x1040, val);
}

STATIC bool drv_pcierc_get_smmu_enable_cfg(void)
{
    // read sysconfig.bin to get SMMU enable status
    return true;
}

STATIC void drv_pcierc_enable_smmu(u32 core_id)
{
    u32 val;
    void __iomem *its_reg_base = NULL;

    // find which PCIe Core work on RC Mode
    if (drv_pcierc_get_smmu_enable_cfg()) {
        val = drv_pcierc_core_read(core_id, GLB_PCIEC_MODE_SEL);
        if ((val & BIT(0)) == PORT_RP) {
            // step1:'iob_rx_aml_cmd_type for host MSI/MSI-X int RC mode
            val = drv_pcierc_ap_read(core_id, AP_IOB_RX_REG_BASE, IOB_RX_AML_SNOOP);
            val |= (u32)BIT(16) | (u32)BIT(17) | (u32)BIT(18);
            drv_pcierc_ap_write(core_id, AP_IOB_RX_REG_BASE, IOB_RX_AML_SNOOP, val);

            // step2:iob_rx_msi_int_reqid_en
            val = drv_pcierc_ap_read(core_id, AP_IOB_RX_REG_BASE, (u32)IOB_RX_MSI_INT_CTRL);
            val |= (u32)BIT(9) | (u32)BIT(1);
            drv_pcierc_ap_write(core_id, AP_IOB_RX_REG_BASE, (u32)IOB_RX_MSI_INT_CTRL, val);
        }
        // step3:bit7：PCIE virtual address transform enable，default 0x0(1‘b0),reg GITS_FUNC_EN bit7 to 0x1;
        its_reg_base = ioremap(ITS_REG_BASE + GITS_FUNC_EN_REG_OFFSET, ONE_REG_LEN);
        if (its_reg_base == NULL) {
            drv_pcie_rc_err("Function ioremap its reg failed.\n");
            return;
        }
        val = readl(its_reg_base);
        val |= (u32)BIT(7);
        writel(val, its_reg_base);
        iounmap(its_reg_base);
    }
}

STATIC void drv_pcierc_pf_init(u32 core_id, u32 port)
{
    drv_pcie_rc_info("PCIE BAR CONFIG......START\n");
    drv_pcierc_atu_cfg_for_pf(core_id, port, ENABLE);
    drv_pcie_rc_info("PCIE BAR CONFIG......END\n");

    drv_pcierc_msi_int_cfg(core_id, port);
    drv_pcierc_enable_smmu(core_id);
}

STATIC void drv_pcierc_crg_dereset(u32 core_id)
{
    void __iomem *io_subctrl_reg = NULL;

    io_subctrl_reg = ioremap(IO_SUB_REG_BASE, IO_SUB_REG_SIZE);
    if (io_subctrl_reg == NULL) {
        drv_pcie_rc_err("Function ioremap IO_SUBCTRL reg failed.\n");
        return;
    }
    // enable pcie master and slave clock
    writel(0x3, io_subctrl_reg + IO_SUB_PCIE_BRG_ICG_EN_REG);
    // enable pcie ctrl0/ctrl1 clock
    writel(0xFFFFFFFFU, io_subctrl_reg + g_crg_reset[core_id].ctrl0_en_clock);
    writel(0xFF, io_subctrl_reg + g_crg_reset[core_id].ctrl1_en_clock);
    // dereset pcie ctrl
    writel(0x7FFFFF, io_subctrl_reg + g_crg_reset[core_id].ctrl_reset);

    iounmap(io_subctrl_reg);
}

STATIC void drv_pcierc_enable_port_en(u32 core_id, u32 port_id)
{
    u32 tmp;

    // config IOB_RX_AML_AXUSER_CTRL2.iob_rx_aml_so_wr(bit31 to 0x1) for TLP report int order
    tmp = drv_pcierc_ap_read(core_id, AP_IOB_RX_REG_BASE, 0x1AA4);
    tmp |= (u32)BIT(31);
    drv_pcierc_ap_write(core_id, AP_IOB_RX_REG_BASE, 0x1AA4, tmp);

    udelay(PCIE_DELAY_TIME_US);
    // port EN
    tmp = drv_pcierc_core_read(core_id, PORT_EN_REG);
    tmp |= (u32)(0x1U << PORTID_2_PORTNUM(port_id));
    drv_pcierc_core_write(core_id, PORT_EN_REG, tmp);

    // Ap port EN
    tmp = drv_pcierc_ap_read(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_PORT_EN_REG);
    tmp |= (u32)(0x1U << PORTID_2_PORTNUM(port_id));
    drv_pcierc_ap_write(core_id, PCIE_AP_GLOBAL_REG_BASE, AP_PORT_EN_REG, tmp);
}

STATIC inline u32 drv_pcierc_get_dlinit_linkup_status(u32 core_id, u32 port_id)
{
    return (drv_pcierc_dl_read(core_id, port_id, PCIe_DL_REG_G3X4_DL_INT_STATUS_REG) >>
        PCIe_DL_UP_INT_STATUS_OFFSET) & 0x1U;
}

STATIC inline u32 drv_pcierc_get_ltssm_status(u32 core_id, u32 port_id)
{
    return (drv_pcierc_mac_read(core_id, port_id, 0x60) >> PCIe_LTSSM_STATUS_OFFSET) & 0x3fU;
}

STATIC bool drv_pcierc_get_enum_device_flag(void)
{
    void __iomem *gpio5_base = NULL;
    u32 value;

    /* resume scene needn't jugde gpio */
    if (pcie_rc_get_active_dev_num() != 0) {
        return true;
    }
    gpio5_base = ioremap(GPIO5_REG_BASE, GPIO5_REG_SIZE);
    if (gpio5_base == NULL) {
        drv_pcie_rc_err("ioremap GPIO5_REG_BASE fail.\n");
        return false;
    }

    value = readl(gpio5_base + GPIO_SW_IN_REG_OFFSET);
    iounmap(gpio5_base);

    if (((value >> GPIO_PIN13) & 0x1U) == GPIO_LOW) {
        return true;
    }
    return false;
}

STATIC int drv_pcierc_controller_reset(u32 core_id)
{
    u32 ap_idle_status;
    u32 count = 0;
    void __iomem *apb_base = NULL;

    apb_base = pcie_rc_get_apb_base(core_id);
    /* stop the interaction between system bus and PCIe IP
    AP_GLOBAL_REG.AP_ IDLE.ap_idle [Should be 0x1] */
    do {
        count++;
        if (count > AP_STATUS_IDLE_WAIT_MAX) {
            drv_pcie_rc_err("wait ap idle status timeout.(core_id=%u)\n", core_id);
            return -EBUSY;
        }
        msleep(AP_IDLE_STATUS_WAIT_TIME);
        ap_idle_status = readl(apb_base + PCIE_AP_GLOBAL_REG_BASE + AP_IDLE_REG_ADDR) & 0x1U;
    } while (ap_idle_status != AP_STATUS_IDLE);

    /* ras interrupt disable */
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_CE_ENA_ADDR);
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_UNF_ENA_ADDR);
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_UF_ENA_ADDR);

    /* local interrupt disable */
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_AP_CE_ENA_ADDR);
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_AP_UNF_ENA_ADDR);
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_AP_UF_ENA_ADDR);
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_AP_NI_ENA_ADDR);
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_CORE_CE_ENA_ADDR);
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_CORE_UNF_ENA_ADDR);
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_CORE_UF_ENA_ADDR);
    writel(0x0, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_CORE_NI_ENA_ADDR);

    /* msi and intx interrupt disable */
    writel(0xFFFFFFFFU, apb_base + PCIE_AP_GLOBAL_REG_BASE + PCIE_MSI_MASK_ADDR);
    writel(0xFFFF, apb_base + PCIE_AP_GLOBAL_REG_BASE + PORT_INTX_ASSERT_MASK_ADDR);
    writel(0xFFFF, apb_base + PCIE_AP_GLOBAL_REG_BASE + PORT_INTX_DEASSERT_MASK_ADDR);

    /* close port_en */
    writel(0x0, apb_base + PCIE_CORE_REG_BASE + PORT_EN_ADDR);

    /* reset ap axi */
    writel(0x1, apb_base + PCIE_AP_GLOBAL_REG_BASE + AP_AXI_SYN_RST_ADDR);
    /* reset ap apb */
    writel(0x1, apb_base + PCIE_AP_GLOBAL_REG_BASE + AP_APB_SYN_RST_ADDR);

    /* reset port logic */
    writel(0xFFFF, apb_base + PCIE_CORE_REG_BASE + PORT_RESET_ADDR);

    /* config the soft reset of the common logic in core */
    writel(0x1, apb_base + PCIE_CORE_REG_BASE + PCIE_CORE_TL_COM_SFT_RST_ADDR);
    writel(0x1, apb_base + PCIE_CORE_REG_BASE + PCIE_CORE_PHY_COM_SFT_RST_ADDR);

    /* config the port_sticky_sft_rst to reset the cfgspace sticky register */
    writel(0xFFFF, apb_base + PCIE_CORE_REG_BASE + PHY_RESET_ADDR);

    /* config the soft reset of the PCS */
    writel(0xFFFFFFFFU, apb_base + PCIE_CORE_REG_BASE + PCS_SOFT_REST_ADDR);
    return 0;
}

STATIC void drv_pcierc_controller_self_heal(u32 core_id)
{
    int ret;
    drv_pcie_rc_info("reset controller self heal start.(core_id=%u)\n", core_id);
    ret = drv_pcierc_controller_reset(core_id);
    if (ret != 0) {
        drv_pcie_rc_err("reset controller failed.(core_id=%u)\n", core_id);
        return;
    }

    msleep(PCIE_MODULE_RESET_WAIT_TIME);
    drv_pcierc_ctrl_init(core_id);
    drv_pcie_rc_info("reset controller self heal end.(core_id=%u)\n", core_id);
    return;
}

STATIC void drv_pcierc_startup_self_heal_handle(bool link_up_flag, u32 core_id, u32 port_id, bool *self_heal_flag)
{
    if (link_up_flag || (pcie_rc_get_active_dev_num() != 0)) {
        return;
    }
#ifdef CFG_FEATURE_PCIE_DFX
    if ((*self_heal_flag) == false) {
        drv_pcie_rc_info("link up timeout, print the ltssm trace and try self-healing\n");
        pcie_ltssm_tracer(core_id, port_id);
    }
#endif
    drv_pcierc_controller_self_heal(core_id);
    *self_heal_flag = true;
    return;
}

void drv_pcierc_resume_self_heal_handle(void)
{
    u32 status;
    u32 ltssm_st;
    u32 count = 0;
    u32 core_id = 0;
    u32 port_id = 0;
    u32 self_heal_times = 0;

    /* if not link up, to reduce resume time, so self heal immediately. */
    if (drv_pcierc_get_dlinit_linkup_status(core_id, port_id) == LINK_UP) {
        return;
    } else {
#ifdef CFG_FEATURE_PCIE_DFX
        drv_pcie_rc_info("link up timeout, print the ltssm trace and try self-healing\n");
        pcie_ltssm_tracer(core_id, port_id);
#endif
        drv_pcierc_controller_self_heal(core_id);
        self_heal_times++;
    }

    do {
        count++;
        if (self_heal_times >= PCIE_SELF_HEAL_MAX_TIMES) {
            drv_pcie_rc_info("self heal up to two times.(status=%u, ltssm_st=%u)\n", status, ltssm_st);
            return;
        }
        if (count > PCIE_LINK_UP_WAIT_MAX_TIMES) {
            drv_pcierc_controller_self_heal(core_id);
            self_heal_times++;
            count = 0;
        }
        usleep_range(PCIE_LINK_UP_WAIT_TIME, PCIE_LINK_UP_WAIT_TIME + PCIE_LINK_UP_WAIT_TIME_RANGE);
        status = drv_pcierc_get_dlinit_linkup_status(core_id, port_id);
        ltssm_st = drv_pcierc_get_ltssm_status(core_id, port_id);
    } while ((status != LINK_UP) && (ltssm_st != LTSSM_STATUS_L0));
    return;
}

void drv_pcierc_wait_link_up(u32 core_id, u32 port_id, bool *self_heal_flag)
{
    u32 status;
    u32 ltssm_st;
    bool enum_device_flag = false;
    bool link_up_flag = false;
    u32 count = 0;
    u32 self_heal_times = 0;

    do {
        count++;
        if ((count > PCIE_LINK_UP_WAIT_MAX_TIMES) && (self_heal_times < PCIE_SELF_HEAL_MAX_TIMES)) {
            drv_pcierc_startup_self_heal_handle(link_up_flag, core_id, port_id, self_heal_flag);
            self_heal_times++;
            count = 0;
        }
        usleep_range(PCIE_LINK_UP_WAIT_TIME, PCIE_LINK_UP_WAIT_TIME + PCIE_LINK_UP_WAIT_TIME_RANGE);
        status = drv_pcierc_get_dlinit_linkup_status(core_id, port_id);
        ltssm_st = drv_pcierc_get_ltssm_status(core_id, port_id);
        enum_device_flag = drv_pcierc_get_enum_device_flag();
        if (((status == LINK_UP) || (ltssm_st == LTSSM_STATUS_L0)) && !link_up_flag) {
            drv_pcie_rc_info("pcie link up finish. (dl_status=0x%x, ltssm_st=0x%x)\n", status, ltssm_st);
            link_up_flag = true;
        }
    } while (((status != LINK_UP) && (ltssm_st != LTSSM_STATUS_L0)) || (!enum_device_flag));

    drv_pcie_rc_info("PCIe DL int link up status is 0x%x.(core_id=%u, enum_dev_flag=%u)\n",
        status, core_id, (u32)enum_device_flag);
    return;
}

STATIC void drv_pcierc_check_dlinit_linkup_status(u32 core_id, u32 port_id)
{
    u32 status;
    u32 cnt = 0;

    do {
        cnt++;
        msleep(PCIe_DL_LINK_UP_WAIT_TIME);
        status = drv_pcierc_get_dlinit_linkup_status(core_id, port_id);
    } while ((status != 0x1) && (cnt < PCIe_DL_LINK_UP_WAIT_LIMIT));

    drv_pcie_rc_info("PCIe DL int link up status is 0x%x.(core_id=%u; wait_cnt=%u)\n", status, core_id, cnt);
}

STATIC void drv_pcierc_set_perst(u32 core_id, u32 level)
{
    void __iomem *ao_iomux_base;
    void __iomem *ao_gpio0_base;
    u32 val;

    ao_iomux_base = ioremap(AO_IOMUX_REG_BASE, AO_IOMUX_REG_SIZE);
    if (ao_iomux_base == NULL) {
        drv_pcie_rc_err("Function ioremap pcie base reg failed.\n");
        return;
    }
    ao_gpio0_base = ioremap(AO_GPIO0_REG_BASE, AO_GPIO0_REG_SIZE);
    if (ao_gpio0_base == NULL) {
        iounmap(ao_iomux_base);
        drv_pcie_rc_err("Function ioremap pcie base reg failed.\n");
        return;
    }

    // config AO IOMUX04/5/6/7 to GPIO
    writel(0x3, ao_iomux_base + g_ao_iomux_perst_offset[core_id]);
    // config output
    writel(0xF, ao_gpio0_base + GPIO_SW_OEN_REG_OFFSET);

    if (level == PERST_LOW) {
        val = readl(ao_gpio0_base + GPIO_SW_OUT_REG_OFFSET);
        val &= ~(1U << core_id);
        writel(val, ao_gpio0_base + GPIO_SW_OUT_REG_OFFSET);
    } else {
        val = readl(ao_gpio0_base + GPIO_SW_OUT_REG_OFFSET);
        val |= (1U << core_id);
        writel(val, ao_gpio0_base + GPIO_SW_OUT_REG_OFFSET);
        val = readl(ao_gpio0_base + GPIO_SW_OUT_REG_OFFSET);
    }

    iounmap(ao_gpio0_base);
    iounmap(ao_iomux_base);

    drv_pcie_rc_info("PCIe RC Config Preset. (core_id=%u; level=%u; val=%u)\n", core_id, level, val);
}

void drv_pcierc_ctrl_init(u32 core_id)
{
    u32 port_id = 0x0;  // Port Id in one chip
    u32 lane;
    u32 para_index;

    pcie_rc_ctrl_lock();

    drv_pcie_rc_info("PCIe RC ctrl init start. (core=%u)\n", core_id);

    // set PCIE Perst High
    if (pcie_rc_get_type() == RC_TYPE_NORMAL) {
        drv_pcierc_set_perst(core_id, PERST_HIGH);
    }

    // CRG de-reset
    drv_pcierc_crg_dereset(core_id);

    // ASIC max link speed gen3
    ltssm_rc_cfg_def[PCIE_X4_GEN1_LINKUP].max_lk_spd = g_pcie_link_info[core_id].max_link_speed;
    // ASIC is X4 lane
    ltssm_rc_cfg_def[PCIE_X4_GEN1_LINKUP].max_sup_width = g_pcie_link_info[core_id].max_link_width;
    // ASIC target link speed gen3
    ltssm_rc_cfg_def[PCIE_X4_GEN1_LINKUP].target_speed = g_pcie_link_info[core_id].target_link_speed;

    if (pcie_rc_get_type() == RC_TYPE_NORMAL) {
        para_index = PCIE_X4_GEN1_LINKUP;
    } else {
        para_index = PCIE_X2_GEN3_LINKUP;
    }

    drv_pcierc_port_init(core_id, port_id, para_index);
    // PF init
    drv_pcierc_pf_init(core_id, port_id);

    // pcs set rx low impedance
    for (lane = 0; lane < ltssm_rc_cfg_def[para_index].max_sup_width; lane++) {
        drv_pcierc_pcs_write(core_id, port_id, PCS_LANE_REG_BASE + LANE_EBUF_PARA + (0x80U * lane), 0x3949EF);
    }
    drv_pcie_rc_info("PCIe RC pcs init finish. (core=%u)\n", core_id);

    pcie_local_ras_config(pcie_rc_get_apb_base(core_id), port_id);
    pcie_rc_aer_config(pcie_rc_get_apb_base(core_id), port_id);

    pcie_rc_ctrl_unlock();

    // Enable Port EN and Ap Port EN
    drv_pcierc_enable_port_en(core_id, port_id);
    // Check DL init link up status (0x1)  DL_INT_STATUS.dl_up_int_status bit18
    drv_pcierc_check_dlinit_linkup_status(core_id, port_id);

    drv_pcie_rc_info("PCIe RC ctrl init end. (core=%u)\n", core_id);

    return;
}

int drv_pcierc_ctrl_uninit(struct platform_device *pdev)
{
    u32 core_id;
    int ret;

    ret = of_property_read_u32(pdev->dev.of_node, "pcie-core-id", &core_id);
    if ((ret < 0) || (core_id >= PCIE_CORE_NUM)) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_SUSPEND_GET_CORE_ID_FAIL);
        drv_pcie_rc_err("Failed to get pcie-core-id. (ret=%d; core_id=%u)\n", ret, core_id);
        return -EINVAL;
    }

    if (pcie_rc_get_apb_base(core_id) == NULL) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_SUSPEND_GET_APB_BASE_FAIL);
        drv_pcie_rc_err("apb_base is null. (core_id=%u)\n", core_id);
        return -EINVAL;
    }

    pcie_rc_ctrl_lock();
    drv_pcie_rc_info("PCIE RC ctrl uninit start. (core=%u)\n", core_id);

    /* disable tx atu */
    drv_pcierc_atu_cfg_for_pf(core_id, 0, DISABLE);
    /* disable interrupt */
    drv_pcierc_interrupt_cfg(core_id);
    // set PCIE Perst low
    if (pcie_rc_get_type() == RC_TYPE_NORMAL) {
        drv_pcierc_set_perst(core_id, PERST_LOW);
    }
    drv_pcierc_apb_uninit(core_id);

    drv_pcie_rc_info("PCIE RC ctrl uninit end. (core=%u)\n", core_id);
    pcie_rc_ctrl_unlock();
    return 0;
}

phys_addr_t drv_pcierc_get_reg_base(u32 core_id)
{
    return (phys_addr_t)(PCIE_BASE_ADDR + (PCIE_REG_OFFSET * core_id));
}
#else

void drv_pcierc_ctrl_init(void)
{
    return;
}

#endif /* end of PCIE_RC_UT */
