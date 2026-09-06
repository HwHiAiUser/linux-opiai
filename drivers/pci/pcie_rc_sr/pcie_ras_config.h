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

#ifndef PCIE_RAS_CONFIG_H
#define PCIE_RAS_CONFIG_H

#ifndef PCIE_RC_UT
#ifndef PCIE_EP_UT
#include <linux/io.h>

void pcie_local_ras_config(void __iomem * PcieLocalBase, u32 portnum);
void pcie_mask_fault_config(void __iomem *pcie_local_base);
void pcie_rc_aer_config(void __iomem *pcie_local_base, u32 portnum);

// AP_GLOBAL_REG
#define INIT_PCIE_CORE_NI_STATUS             0x180ULL
#define INIT_PCIE_CORE_CE_STATUS             0x184ULL
#define INIT_PCIE_CORE_UNF_STATUS            0x188ULL
#define INIT_PCIE_CORE_UF_STATUS             0x18CULL
#define INIT_PCIE_AP_NI_STATUS               0x120ULL
#define INIT_PCIE_AP_CE_STATUS               0x124ULL
#define INIT_PCIE_AP_UNF_STATUS              0x128ULL
#define INIT_PCIE_AP_UF_STATUS               0x12CULL

#define INIT_PCIE_AP_CE_ENA                  0x104ULL
#define INIT_PCIE_AP_UNF_ENA                 0x108ULL
#define INIT_PCIE_AP_UF_ENA                  0x10CULL
#define INIT_PCIE_AP_NI_ENA                  0x100ULL
#define INIT_PCIE_CORE_CE_ENA                0x164ULL
#define INIT_PCIE_CORE_UNF_ENA               0x168ULL
#define INIT_PCIE_CORE_UF_ENA                0x16CULL
#define INIT_PCIE_CORE_NI_ENA                0x160ULL

#define INIT_PCIE_AP_CE_MASK                 0x114ULL
#define INIT_PCIE_AP_UNF_MASK                0x118ULL
#define INIT_PCIE_AP_UF_MASK                 0x11CULL
#define INIT_PCIE_AP_NI_MASK                 0x110ULL
#define INIT_PCIE_CORE_CE_MASK               0x174ULL
#define INIT_PCIE_CORE_UNF_MASK              0x178ULL
#define INIT_PCIE_CORE_UF_MASK               0x17CULL
#define INIT_PCIE_CORE_NI_MASK               0x170ULL

// TOP REG
#define APB_TIMEOUT_INT_SET              0x0ULL
#define APB_TIMEOUT_INT_MASK             0x4ULL
#define APB_TIMEOUT_INT_STATUS           0x8ULL
#define APB_TIMEOUT_INT_RO               0xCULL

#define INIT_TL_CORE_INT_STATUS0              (0x30000 + 0x814)
#define INIT_TL_CORE_INT_MASK0                (0x30000 + 0x818)
#define INIT_TL_CORE_INT_SET0                 (0x30000 + 0x81C)
#define INIT_TL_CORE_INT_RO0                  (0x30000 + 0x820)

#define SYSTEM_ERR_OVERRIDE_CONTROL           0x004E4ULL
#define INIT_TL_REG_OFFSET(PortNum)           (0x20000 + 0x1000 * portnum)
#define INIT_TL_REG_OFFSET_CS(PortNum)        (0x20000 + 0x1000 * portnum)
#define INIT_TL_INT_STATUS0                   0x56CULL
#define INIT_TL_INT_MASK0                     0x574ULL
#define INIT_TL_INT_SET0                      0x578ULL
#define INIT_TL_INT_RO0                       0x57CULL
#define INIT_TL_INT_STATUS1                   0x570ULL
#define INIT_TL_INT_MASK1                     0x580ULL
#define INIT_TL_INT_SET1                      0x584ULL
#define INIT_TL_INT_RO1                       0x588ULL

#define INIT_DL_INT_STATUS                    0x88ULL
#define INIT_DL_INT_MASK                      0x8CULL
#define INIT_DL_INT_RO                        0x90ULL
#define INIT_DL_INT_SET                       0x94ULL

#define INIT_MAC_REG_MAC_INT_RO               0x90ULL
#define INIT_MAC_REG_MAC_INT_STATUS           0x54ULL
#define INIT_MAC_REG_MAC_INT_SET              0x94ULL
#define INIT_MAC_REG_MAC_INT_MSK              0x58ULL
#define MAC_INT_TYPE_SEL                      0x3A8ULL

#define INIT_CORE_INT_NI_SET_0                0x530ULL
#define INIT_CORE_INT_NI_MSK_0                0x534ULL
#define INIT_CORE_INT_NI_STATUS_0             0x538ULL
#define INIT_CORE_INT_NI_RO_0                 0x53CULL
#define INIT_CORE_INT_NI_SET_1                0x540ULL
#define INIT_CORE_INT_NI_MSK_1                0x544ULL
#define INIT_CORE_INT_NI_STATUS_1             0x548ULL
#define INIT_CORE_INT_NI_RO_1                 0x54CULL

#define INIT_CORE_INT_CE_SET_0                0x550ULL
#define INIT_CORE_INT_CE_MSK_0                0x554ULL
#define INIT_CORE_INT_CE_STATUS_0             0x558ULL
#define INIT_CORE_INT_CE_RO_0                 0x55CULL
#define INIT_CORE_INT_CE_SET_1                0x560ULL
#define INIT_CORE_INT_CE_MSK_1                0x564ULL
#define INIT_CORE_INT_CE_STATUS_1             0x568ULL
#define INIT_CORE_INT_CE_RO_1                 0x56CULL
#define INIT_CORE_INT_NFE_SET_0               0x570ULL
#define INIT_CORE_INT_NFE_MSK_0               0x574ULL
#define INIT_CORE_INT_NFE_STATUS_0            0x578ULL
#define INIT_CORE_INT_NFE_RO_0                0x57CULL
#define INIT_CORE_INT_NFE_SET_1               0x580ULL
#define INIT_CORE_INT_NFE_MSK_1               0x584ULL
#define INIT_CORE_INT_NFE_STATUS_1            0x588ULL
#define INIT_CORE_INT_NFE_RO_1                0x58CULL
#define INIT_CORE_INT_FE_SET_0                0x590ULL
#define INIT_CORE_INT_FE_MSK_0                0x594ULL
#define INIT_CORE_INT_FE_STATUS_0             0x598ULL
#define INIT_CORE_INT_FE_RO_0                 0x59CULL
#define INIT_CORE_INT_FE_SET_1                0x5A0ULL
#define INIT_CORE_INT_FE_MSK_1                0x5A4ULL
#define INIT_CORE_INT_FE_STATUS_1             0x5A8ULL
#define INIT_CORE_INT_FE_RO_1                 0x5ACULL

#define INIT_CORE_INT_NI_SET_2                     0x5B0
#define INIT_CORE_INT_NI_MSK_2                     0x5B4
#define INIT_CORE_INT_NI_STATUS_2                  0x5B8
#define INIT_CORE_INT_CE_SET_2                     0x5C0
#define INIT_CORE_INT_CE_MSK_2                     0x5C4
#define INIT_CORE_INT_CE_STATUS_2                  0x5C8
#define INIT_CORE_INT_NFE_SET_2                    0x5D0
#define INIT_CORE_INT_NFE_MSK_2                    0x5D4
#define INIT_CORE_INT_NFE_STATUS_2                 0x5D8
#define INIT_CORE_INT_FE_SET_2                     0x5E0
#define INIT_CORE_INT_FE_MSK_2                     0x5E4
#define INIT_CORE_INT_FE_STATUS_2                  0x5E8
// hilink
#define HILINK_INT_SET                             0x51C
#define HILINK_INT_MASK                            0x520
#define HILINK_INT_RO                              0x524
#define HILINK_INT_STATUS                          0x528

// DL
#define INIT_DL_CXL_INT_STATUS                          0xC24
#define INIT_DL_CXL_INT_MASK                            0xC28
#define INIT_DL_CXL_INT_RO                              0xC2C
#define INIT_DL_CXL_INT_SET                             0xC30

// AP_IOB_TX_REG
#define INIT_IOB_TX_CHI_P2P_UNMATCH_ADDR_L    0x848ULL
#define INIT_IOB_TX_CHI_P2P_UNMATCH_ADDR_H    0x84CULL
#define INIT_IOB_TX_INT_STATUS0               0x820ULL
#define INIT_IOB_TX_INT_STATUS1               0x824ULL
#define INIT_IOB_TX_INT_STATUS2               0x828ULL
#define INIT_IOB_TX_INT_STATUS3               0x82CULL
#define INIT_IOB_TX_INT_STATUS4               0x830ULL
#define INIT_IOB_TX_INT_STATUS5               0x834ULL
#define INIT_IOB_TX_INT_RO0                   0x840ULL
#define INIT_IOB_TX_INT_RO1                   0x844ULL
#define INIT_IOB_TX_INT_RO2                   0x848ULL
#define INIT_IOB_TX_INT_RO3                   0x84CULL
#define INIT_IOB_TX_INT_RO4                   0x850ULL
#define INIT_IOB_TX_INT_RO5                   0x854ULL
#define INIT_IOB_TX_INT_SET0                  0x860ULL
#define INIT_IOB_TX_INT_SET1                  0x864ULL
#define INIT_IOB_TX_INT_SET2                  0x868ULL
#define INIT_IOB_TX_INT_SET3                  0x86CULL
#define INIT_IOB_TX_INT_SET4                  0x870ULL
#define INIT_IOB_TX_INT_SET5                  0x874ULL
#define INIT_IOB_TX_INT_SEVERITY0             0x880ULL
#define INIT_IOB_TX_INT_SEVERITY1             0x884ULL
#define INIT_IOB_TX_INT_SEVERITY2             0x888ULL
#define INIT_IOB_TX_INT_SEVERITY3             0x88CULL
#define INIT_IOB_TX_INT_SEVERITY4             0x890ULL
#define INIT_IOB_TX_INT_SEVERITY5             0x894ULL
#define INIT_IOB_TX_INT_MASK0                 0x800ULL
#define INIT_IOB_TX_INT_MASK1                 0x804ULL
#define INIT_IOB_TX_INT_MASK2                 0x808ULL
#define INIT_IOB_TX_INT_MASK3                 0x80CULL
#define INIT_IOB_TX_INT_MASK4                 0x810ULL
#define INIT_IOB_TX_INT_MASK5                 0x814ULL

// IOB RX INT
#define INIT_IOB_RX_INT_MASK                  0x1700ULL
#define INIT_IOB_RX_INT_STATUS                0x1704ULL
#define INIT_IOB_RX_INT_RO                    0x1708ULL
#define INIT_IOB_RX_INT_SET                   0x170CULL
#define INIT_IOB_RX_INT_SEVERITY              0x1710ULL
#define IOB_RX_AMB_GLOBAL_CTRL                0x1C00ULL
#define IOB_RX_MSI_INT_CTRL                   0x1040ULL

// AP_INT_REG
#define RAS_INT_LEVEL(ep_num)            (0x10000ULL * (ep_num) + 0xA8ULL)
#define ABNORMAL_INT_STS(ep_num)         (0x10000ULL * (ep_num) + 0x500ULL)
#define ABNORMAL_INT_MASK(ep_num)        (0x10000ULL * (ep_num) + 0x504ULL)
#define ABNORMAL_INT_SET(ep_num)         (0x10000ULL * (ep_num) + 0x508ULL)
#define ECC_ERR_INT_STS(ep_num)          (0x10000ULL * (ep_num) + 0x60cULL)
#define ECC_ERR_INT_SRC_STS(ep_num)      (0x10000ULL * (ep_num) + 0x604ULL)
#define ECC_ERR_MASK(ep_num)             (0x10000ULL * (ep_num) + 0x608ULL)
#define ECC_ERR_INJECT(ep_num)           (0x10000ULL * (ep_num) + 0x610ULL)

// PCIE_GLOBAL_REG
#define INIT_PCIE_ERR_MAPPING                 0x00ULL
#define INIT_PCIE_AP_MG_REG                   0x8000ULL
#define INIT_PCIE_CE_STATUS                   0x20ULL
#define INIT_PCIE_UNF_STATUS                  0x28ULL
#define INIT_PCIE_UF_STATUS                   0x30ULL
#define INIT_PCIE_CE_ENA                      0x08ULL
#define INIT_PCIE_UNF_ENA                     0x10ULL
#define INIT_PCIE_UF_ENA                      0x18ULL
#define INIT_PCIE_CE_MASK                     0xCULL
#define INIT_PCIE_UNF_MASK                    0x14ULL
#define INIT_PCIE_UF_MASK                     0x1CULL

// AP_DMA_REG
#define DMA_INT_MODE_SEL                      0x18CULL

// PCS
#define LANE_INTR_MASK                        0x48

// CFGSPACE
#define UNCORRECT_ERR_STATUS 0x104ULL
#define UNCORRECT_ERR_MASK 0x108ULL
#define CORRECT_ERR_STATUS 0x110ULL
#define CORRECT_ERR_MASK 0x114ULL

#endif
#endif
#endif
