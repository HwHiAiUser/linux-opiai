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
* Create: 2023-8-10
*/

#include <linux/of_address.h>
#include <linux/securec.h>

#include "pcie_ras_config.h"

#if (defined (PCIE_RC_UT) || defined (PCIE_EP_UT))
int drv_pcie_ras_ut(void)
{
    return 0;
}
#else
#define BIT0 0x00000001U
#define BIT3 0x00000008U
#define BIT5 0x00000020U
#define BIT8     0x0100U
#define BIT13    0x2000U
#define BIT17    0x00020000U
#define BIT26    0x04000000U

#define PCIE_RAS_WARITEL(a, b) writel(b, a)

static void pcie_core_config(void __iomem * pcie_local_base, void __iomem * tl_reg_base,
                             void __iomem * dl_reg_base, void __iomem * mac_reg_base)
{
    PCIE_RAS_WARITEL(tl_reg_base + INIT_TL_INT_SET0, 0U);
    PCIE_RAS_WARITEL(tl_reg_base + INIT_TL_INT_STATUS0, 0xFFFFFFFFU);
    // TL MASK  Decode mask according to the fault handling recommendation table
    PCIE_RAS_WARITEL(tl_reg_base + INIT_TL_INT_MASK0, readl(tl_reg_base + INIT_TL_INT_MASK0) & (~0x11D8U));
    // DL STS
    PCIE_RAS_WARITEL(dl_reg_base + INIT_DL_INT_SET, 0U);
    PCIE_RAS_WARITEL(dl_reg_base + INIT_DL_INT_STATUS, 0xFFFFFFFFU);
    // DL MSK  Decode mask according to the fault handling recommendation table
    PCIE_RAS_WARITEL(dl_reg_base + INIT_DL_INT_MASK, readl(dl_reg_base + INIT_DL_INT_MASK) & (~0x7FFU));
    // MAC STS
    PCIE_RAS_WARITEL(mac_reg_base + INIT_MAC_REG_MAC_INT_SET, 0U);
    PCIE_RAS_WARITEL(mac_reg_base + INIT_MAC_REG_MAC_INT_STATUS, 0xFFFFFFFFU);
    // MAC MSK  Decode mask according to the fault handling recommendation table
    PCIE_RAS_WARITEL(mac_reg_base + INIT_MAC_REG_MAC_INT_MSK,
        readl(mac_reg_base + INIT_MAC_REG_MAC_INT_MSK) & (~0x803602U));
}

static void pcie_core_reg_config(void __iomem * core_reg_base)
{
    // Core SET
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_CE_SET_0, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_CE_SET_1, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_CE_SET_2, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_NFE_SET_0, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_NFE_SET_1, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_NFE_SET_2, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_FE_SET_0, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_FE_SET_1, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_FE_SET_2, 0x0);
    // Core STS
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_CE_STATUS_0, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_CE_STATUS_1, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_CE_STATUS_2, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_NFE_STATUS_0, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_NFE_STATUS_1, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_NFE_STATUS_2, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_FE_STATUS_0, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_FE_STATUS_1, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_FE_STATUS_2, 0x0);
    // Core MSK
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_CE_MSK_0, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_CE_MSK_1, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_CE_MSK_2, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_NFE_MSK_0, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_NFE_MSK_1, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_NFE_MSK_2, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_FE_MSK_0, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_FE_MSK_1, 0x0);
    PCIE_RAS_WARITEL(core_reg_base + INIT_CORE_INT_FE_MSK_2, 0x0);
}

static void pcie_ap_global_config(void __iomem * ap_global_reg)
{
    u32 mapping;
    // Software enable the AP interrupt enable registers of the local interrupt source
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CORE_CE_STATUS, 0x1U);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CORE_UNF_STATUS, 0x1U);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CORE_UF_STATUS, 0x1U);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_AP_CE_STATUS, 0xFFFFFFFFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_AP_UNF_STATUS, 0xFFFFFFFFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_AP_UF_STATUS, 0xFFFFFFFFU);

    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_AP_CE_ENA, 0x3FFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_AP_UNF_ENA, 0x3FFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_AP_UF_ENA, 0x3FFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CORE_CE_ENA, 0xFFFFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CORE_UNF_ENA, 0xFFFFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CORE_UF_ENA, 0xFFFFU);
    // Software configures the AP interrupt mask registers to be invalid.
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_AP_CE_MASK, 0x0U);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_AP_UNF_MASK, 0x0U);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_AP_UF_MASK, 0x0U);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CORE_CE_MASK, 0x0U);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CORE_UNF_MASK, 0x0U);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CORE_UF_MASK, 0x0U);

    // AP_GLOBAL_REG enable
    mapping = readl(ap_global_reg + INIT_PCIE_ERR_MAPPING);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_ERR_MAPPING, (u64)(mapping | (u32)(BIT8 | BIT13 | BIT17)));
}

static void pcie_iob_tx_config(void __iomem * pcie_local_base, void __iomem * ap_iob_rx_reg)
{
    // AP AP_IOB_TX_REG SET
    PCIE_RAS_WARITEL(pcie_local_base + INIT_IOB_TX_INT_SET1, 0x0U);
    PCIE_RAS_WARITEL(pcie_local_base + INIT_IOB_TX_INT_SET2, 0x0U);
    PCIE_RAS_WARITEL(pcie_local_base + INIT_IOB_TX_INT_SET3, 0x0U);
    // AP AP_IOB_TX_REG STS
    PCIE_RAS_WARITEL(pcie_local_base + INIT_IOB_TX_INT_STATUS1, 0xFFFFFFFFU);
    PCIE_RAS_WARITEL(pcie_local_base + INIT_IOB_TX_INT_STATUS2, 0xFFFFFFFFU);
    PCIE_RAS_WARITEL(pcie_local_base + INIT_IOB_TX_INT_STATUS3, 0xFFFFFFFFU);
    // AP AP_IOB_TX_REG MSK
    PCIE_RAS_WARITEL(pcie_local_base + INIT_IOB_TX_INT_MASK2,
        readl(pcie_local_base + INIT_IOB_TX_INT_MASK2) & (~0xFFFFFFFFU));
    PCIE_RAS_WARITEL(pcie_local_base + INIT_IOB_TX_INT_MASK3,
        readl(pcie_local_base + INIT_IOB_TX_INT_MASK3) & (~0xC0FFFFU));
    // AP ctrl_timeout_thres
    PCIE_RAS_WARITEL(ap_iob_rx_reg + IOB_RX_AMB_GLOBAL_CTRL,
        (readl(ap_iob_rx_reg + IOB_RX_AMB_GLOBAL_CTRL) & (~0xF0U)) | (0xF0U));
    // AP AP_IOB_RX_REG SET
    PCIE_RAS_WARITEL(ap_iob_rx_reg + INIT_IOB_RX_INT_SET, 0x0U);
    // AP AP_IOB_RX_REG STS
    PCIE_RAS_WARITEL(ap_iob_rx_reg + INIT_IOB_RX_INT_STATUS, 0xFFFFFFFFU);
    // AP AP_IOB_RX_REG MSK
    PCIE_RAS_WARITEL(ap_iob_rx_reg + INIT_IOB_RX_INT_MASK,
        readl(ap_iob_rx_reg + INIT_IOB_RX_INT_MASK) & (u64)(~0x1EU));
}

static void pcie_ap_int_config(void __iomem * ap_int_reg)
{
    // AP_INT_REG
    PCIE_RAS_WARITEL(ap_int_reg + (RAS_INT_LEVEL(0)), readl(ap_int_reg + (RAS_INT_LEVEL(0))) | 0x1CU);
    PCIE_RAS_WARITEL(ap_int_reg + (ABNORMAL_INT_STS(0)), 0xFFU);
    PCIE_RAS_WARITEL(ap_int_reg + (ABNORMAL_INT_MASK(0)), readl(ap_int_reg + (ABNORMAL_INT_MASK(0))) & (~0x7U));
    PCIE_RAS_WARITEL(ap_int_reg + (ECC_ERR_INJECT(0)), 0U);
    PCIE_RAS_WARITEL(ap_int_reg + (ECC_ERR_INT_SRC_STS(0)), 0xFFU);
    PCIE_RAS_WARITEL(ap_int_reg + (ECC_ERR_MASK(0)), readl(ap_int_reg + ((ECC_ERR_MASK(0)))) & (~0xAAU));
}

static void pcie_ap_dma_config(void __iomem * ap_dma_reg)
{
    // AP_DMA_REG
    PCIE_RAS_WARITEL(ap_dma_reg + DMA_INT_MODE_SEL, 0x0);
}

void pcie_local_ras_config(void __iomem *pcie_local_base, u32 portnum)
{
    void __iomem * tl_reg_base = pcie_local_base + 0x20000ULL + (u64)(0x1000U * portnum);
    void __iomem * dl_reg_base = pcie_local_base + 0x60000ULL + (u64)(0x1000U * portnum);
    void __iomem * mac_reg_base = pcie_local_base + 0x70000ULL + (u64)(0x1000U * portnum);
    void __iomem * core_reg_base = pcie_local_base + 0x5E000ULL;
    void __iomem * ap_global_reg = pcie_local_base + 0x8000ULL;
    void __iomem * ap_int_reg = pcie_local_base + 0xC000ULL;
    void __iomem * ap_dma_reg = pcie_local_base + 0x10000ULL;
    void __iomem * ap_iob_rx_reg = pcie_local_base + 0x4000ULL;

    pcie_core_config(pcie_local_base, tl_reg_base, dl_reg_base, mac_reg_base);
    pcie_core_reg_config(core_reg_base);
    pcie_iob_tx_config(pcie_local_base, ap_iob_rx_reg);
    pcie_ap_int_config(ap_int_reg);
    pcie_ap_global_config(ap_global_reg);
    pcie_ap_dma_config(ap_dma_reg);
    pcie_mask_fault_config(pcie_local_base);
}

void pcie_mask_fault_config(void __iomem *pcie_local_base)
{
    const u32 portnum = 0x0U;
    void __iomem * tl_reg_base = pcie_local_base + 0x20000ULL + (u64)(0x1000U * portnum);
    void __iomem * mac_reg_base = pcie_local_base + 0x70000ULL + (u64)(0x1000U * portnum);

    PCIE_RAS_WARITEL(pcie_local_base + 0x4000ULL + INIT_IOB_RX_INT_MASK, 0x1FU);
    PCIE_RAS_WARITEL(tl_reg_base + INIT_TL_INT_MASK0, readl(tl_reg_base + INIT_TL_INT_MASK0) & (~0x5F8U));
    PCIE_RAS_WARITEL(mac_reg_base + INIT_MAC_REG_MAC_INT_MSK,
        readl(mac_reg_base + INIT_MAC_REG_MAC_INT_MSK) | (0x01U << 13U));
    PCIE_RAS_WARITEL(pcie_local_base + INIT_IOB_TX_INT_MASK3,
        readl(pcie_local_base + INIT_IOB_TX_INT_MASK3) & (~(0x01U << 16U)));
    return;
}

void pcie_rc_aer_config(void __iomem *pcie_local_base, u32 portnum)
{
    void __iomem *ap_global_reg;
    void __iomem *ap_iob_rx_reg;
    void __iomem *rp_cfg_space_reg;
    u32 reg_value;

    ap_global_reg = pcie_local_base + 0x8000ULL;
    ap_iob_rx_reg = pcie_local_base + 0x4000ULL;
    rp_cfg_space_reg = pcie_local_base + 0x40000ULL + (u64)(0x1000U * portnum);
    // trigger local interrupt
    reg_value = readl(ap_iob_rx_reg + IOB_RX_MSI_INT_CTRL);
    PCIE_RAS_WARITEL(ap_iob_rx_reg + IOB_RX_MSI_INT_CTRL, reg_value | 0x2U);

    // AP_GLOBAL_REG enable
    reg_value = readl(ap_global_reg + INIT_PCIE_ERR_MAPPING);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_ERR_MAPPING, reg_value | (u32)(BIT0 | BIT3 | BIT5));

    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CE_STATUS, 0xFFFFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_UNF_STATUS, 0xFFFFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_UF_STATUS, 0xFFFFU);

    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CE_ENA, 0xFFFFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_UNF_ENA, 0xFFFFU);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_UF_ENA, 0xFFFFU);

    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_CE_MASK, 0x0U);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_UNF_MASK, 0x0U);
    PCIE_RAS_WARITEL(ap_global_reg + INIT_PCIE_UF_MASK, 0x0U);

    /* enable ce mask and clear status */
    PCIE_RAS_WARITEL(rp_cfg_space_reg + CORRECT_ERR_MASK,
        readl(rp_cfg_space_reg + CORRECT_ERR_MASK) & (~0xF1C1U));
    PCIE_RAS_WARITEL(rp_cfg_space_reg + CORRECT_ERR_STATUS,
        readl(rp_cfg_space_reg + CORRECT_ERR_STATUS));

    /* enable ue mask and clear status */
    PCIE_RAS_WARITEL(rp_cfg_space_reg + UNCORRECT_ERR_MASK,
        readl(rp_cfg_space_reg + UNCORRECT_ERR_MASK) & (~0x7FFF030U));
    PCIE_RAS_WARITEL(rp_cfg_space_reg + UNCORRECT_ERR_STATUS,
        readl(rp_cfg_space_reg + UNCORRECT_ERR_STATUS));

    /* pcie 3.0 not support bit26 */
    PCIE_RAS_WARITEL(rp_cfg_space_reg + UNCORRECT_ERR_MASK,
        readl(rp_cfg_space_reg + UNCORRECT_ERR_MASK) | BIT26);
}
#endif