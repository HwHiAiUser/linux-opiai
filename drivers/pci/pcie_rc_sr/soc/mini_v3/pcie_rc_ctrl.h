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

#ifndef PCIE_RC_CTRL_H
#define PCIE_RC_CTRL_H

#ifndef PCIE_RC_UT

#include "pcie_rc_common.h"

#define ONE_BYTE_WIDTH 8
#define TWO_BYTE_WIDTH 16

#define BIT_OPERATE_MOVE_32BIT 32

#define ONE_REG_LEN 4UL

/* core0:x4/x2/x1 core1:x1 core2:x2 core3:x1 */
#define PCIE_CORE_NUM 4
#define PCIE_REG_OFFSET 0x100000U
#define PCIE_REG_SIZE 0x100000

#define GEN1 0x1
#define GEN2 0x2
#define GEN3 0x3
#define GEN4 0x4

#define GBL_WX16 0x4
#define GBL_WX8  0x3
#define GBL_WX4  0x2
#define GBL_WX2  0x1
#define GBL_WX1  0x0

#define CFG_WX32 0x20
#define CFG_WX16 0x10
#define CFG_WX8  0x8
#define CFG_WX4  0x4
#define CFG_WX2  0x2
#define CFG_WX1  0x1

#define PCIE_X4_GEN1_LINKUP   0
#define PCIE_X2_GEN3_LINKUP   1
#define PCIE_MAX_CONFIG_NUM   2

#define FS_DEFINE_PCIE_PORT0  0
#define FS_DEFINE_PCIE_PORT2  2
#define FS_DEFINE_PCIE_PORT4  4
#define FS_DEFINE_PCIE_PORT6  6
#define FS_DEFINE_PCIE_PORT8  8
#define FS_DEFINE_PCIE_PORT10 10
#define FS_DEFINE_PCIE_PORT12 12
#define FS_DEFINE_PCIE_PORT14 14

#define PCIE_PORT_WIDTH_INDEX_0 0
#define PCIE_PORT_WIDTH_INDEX_1 1
#define PCIE_PORT_WIDTH_INDEX_2 2
#define PCIE_PORT_WIDTH_INDEX_3 3

#define PORT_EP 0
#define PORT_RP 1

#define PORTID_2_CORENUM(p)  ((p) >> 0x3U)               /* Pcie Core -> 0, 1, 2 */
#define PORTID_2_PORTNUM(p)  (((p) % 0x8U) << 1U)        /* Port 0 ~ 7, 8 ~ 15, 16 ~ 19, translate to
                                                           FS define Port 2/4/6/8/10/12/14 */

#define DELAY_MIN_MSECOND 1
#define DELAY_MAX_MSECOND 2

#define ENABLE  1
#define DISABLE 0

#define ATU_NUM_FOR_PF 2
#define ATU_INDEX_0    0
#define ATU_INDEX_1    1

#define PERST_LOW  0
#define PERST_HIGH 1

#define LINK_UP 0x1
#define LTSSM_STATUS_L0 0x10

#define PCIE_DELAY_TIME_US 1
/********************************************************************************************/
/*                  AO IOMUX_REG base                                                       */
/********************************************************************************************/
#define AO_IOMUX_REG_BASE                    0xC4000000U
#define AO_IOMUX_REG_SIZE                    0x10000UL

/********************************************************************************************/
/*                  AO GPIO0_REG base                                                       */
/********************************************************************************************/
#define AO_GPIO0_REG_BASE                    0xC4040000U
#define AO_GPIO0_REG_SIZE                    0x10000UL
#define GPIO_SW_OUT_REG_OFFSET               0x0
#define GPIO_SW_OEN_REG_OFFSET               0x4

/********************************************************************************************/
/*                  AO GPIO5_REG base                                                       */
/********************************************************************************************/
#define GPIO_HIGH 1U
#define GPIO_LOW  0U
#define GPIO5_REG_BASE 0x300160000ULL
#define GPIO5_REG_SIZE 0x10000
#define GPIO_SW_OUT_REG_OFFSET  0x0
#define GPIO_SW_OEN_REG_OFFSET  0x4
#define GPIO_SW_IN_REG_OFFSET   0x50
#define GPIO_PIN13 13U

/********************************************************************************************/
/*                  ITS reg base                                                            */
/********************************************************************************************/
#define ITS_REG_BASE                         0xD2000000U
#define GITS_TRANSLATER_L_OFFSET             0x10040U
#define GITS_FUNC_EN_REG_OFFSET              0x20080U

/********************************************************************************************/
/*                  PCIE reg base                                                           */
/********************************************************************************************/
#define PCIE_BASE_ADDR                       0xA0200000U
#define AP_IOB_TX_REG_BASE                   0x0
#define AP_IOB_RX_REG_BASE                   0x4000
#define PCIE_AP_GLOBAL_REG_BASE              0x8000
#define PCIE_TL_REG_BASE                     0x20000U
#define PCIE_TL_CORE_REG_BASE                0x30000U
#define PCIE_EPF_CFGSPACE_BASE               0x40000U
#define PCIE_PCS_REG_BASE                    0x5A000U
#define PCIE_CORE_REG_BASE                   0x5E000U
#define PCIE_DL_REG_BASE                     0x60000U  /* DL_REG_G3X4 */
#define PCIE_MAC_REG_BASE                    0x70000U

/********************************************************************************************/
/*                   PCIE CORE GLB CLKRST                                                   */
/********************************************************************************************/
#define PORT_RESET_REG               0x0
#define PHY_REST_REG                 0x4
#define PCS_SOFT_REST_REG            0xC
#define PCIE_CORE_TL_COM_SFT_RST     0x14
#define PCIE_CORE_PHY_COM_SFT_RST    0x18

/********************************************************************************************/
/*                   PCIE CORE GLB CTRL REG                                                 */
/********************************************************************************************/
#define PORT_EN_REG             0x400 /* port exist */
#define GLB_PCIEC_MODE_SEL      0x434 /* port mode */
#define PORT07_LINK_MODE        0x510 /* port 0~7 link mode */
#define PORT815_LINK_MODE       0x514 /* port8~15 link mode */
#define CORE_INT_NI_MSK_0       0x534 /* CORE interrupt mask ni */

#define LINK_DOWN_RST_EN        0x75C /* reset en */
#define LINK_DOWN_CLR_PORT_EN   0x760 /* clear port en */

/********************************************************************************************/
/*                   PCIE EP Config space                                                   */
/********************************************************************************************/
#define PCIe_HIPCIEC_EPF_CFGSPACE_PCIHDR_ID_REG          0x0   /* This register specify the register of config space. */
#define PCIe_HIPCIEC_EPF_CFGSPACE_PCIHDR_CMDSTS_REG      0x4   /* This register specify the register of config space. */
#define PCIHDR_CLSREV                                    0x08
#define PCIHDR_BAR0                                      0x10
#define PCIHDR_BAR2                                      0x18
#define PCIHDR_BAR4                                      0x20
#define PCIe_HIPCIEC_EPF_CFGSPACE_PCIHDR_SUBSYS          0x2C
#define PCIE_CAP_HEADER                                  0x40
#define DEVICE_CAPBILITY                                 0x44
#define DEVICE_CTRL_STATUS                               0x48
#define PCIe_HIPCIEC_EPF_CFGSPACE_LINK_CAPBILITY_REG     0x4C   /* Link capability register */
#define PCIe_HIPCIEC_EPF_CFGSPACE_LINK_CTRL_STATUS_REG   0x50   /* Link control register */
#define DEVICE_CAPABILITY2                               0x64
#define PCIe_HIPCIEC_EPF_CFGSPACE_LINK_CTRL_STATUS2_REG  0x70   /* Link control and status 2 */
#define MSI_DATA                                         0x8C
#define MSIX_CAP_HEADER                                  0xA0   /* MSIX control register */
#define MSIX_TABLE_CTRL                                  0xA4
#define MSIX_PBA_CTRL                                    0xA8
#define UNCR_ERR_MASK                                    0x108
#define UNCR_ERR_SEVERITY                                0x10C
#define COR_ERR_MASK                                     0x114
#define ADVACD_CAP_CTRL                                  0x118
#define ARI_CTRL                                         0x154
#define PCIe_HIPCIEC_EPF_CFGSPACE_SRIOV_CAP_REG          0x204  /* SRROIOV capability */
#define SRIOV_CTRL                                       0x208  /* Virtual function enable */
#define INIT_VF_NUMBER                                   0x20C  /* Initial VF and Totla VF */
#define FUNC_DEP_VF_NUM                                  0x210
#define VF_RID_SETTING                                   0x214
#define VF_DEVICE_ID                                     0x218

/********************************************************************************************/
/*                   PCIE MAC Reg                                                           */
/********************************************************************************************/
#define PCIe_HIPCIEC_MAC_REG_MAC_REG_DC_BALANCE_DISABLE_REG  0x18  /* DC balance funciton disanle */
#define PCIe_HIPCIEC_MAC_REG_MAC_REG_EQ_DISABLE_REG          0x1C  /* TX EQ function disable */
#define PCIe_HIPCIEC_MAC_REG_MAC_REG_EQ_PHASE23_TIMEOUT_VAL  0x30  /* MAC_REG_EQ_PHASE23_TIMEOUT_VAL */

#define PCIe_HIPCIEC_MAC_REG_MAC_REG_LTSSM_TRACER_INPUT_REG  0x50  /* LTSSM tracer input */
#define PCIe_LTSSM_STATUS_OFFSET                             24

#define PCIe_HIPCIEC_MAC_REG_MAC_REG_EQ_FIX_LP_TX_PRESET_REG 0x9C  /* Tx EQ pahse fix link partner preset */
#define PCIe_HIPCIEC_MAC_REG_MAC_REG_ADJ_HILINK_MODE_EN_REG  0xA0  /* adjust hilink SERDES function mode enable */
#define PCIe_HIPCIEC_MAC_REG_MAC_WAIT_LANE_NUM_TIMER_REG     0x2C4 /* mac_wait_lane_num_timer */
#define PCIe_HIPCIEC_MAC_REG_MAC_LANE_NUM_WAIT_TIMER_REG     0x2C8 /* mac_lane_num_wait_timer */

#define PCIe_HIPCIEC_MAC_REG_MAC_LANE_NUM_ACC_TIMER_REG      0x2CC /* mac_lane_num_acc_timer */
#define PCIe_HIPCIEC_MAC_REG_MAC_ENTER_LPBK_DISABLE_REG      0x2EC /* mac_enter_lpbk_disable */
#define PCIe_HIPCIEC_MAC_REG_MAC_RX_ERR_CHECK_EN_REG         0x304 /* mac_rx_error_check_en */
#define PCIe_HIPCIEC_MAC_REG_MAC_CFG_RCV_HOLD_EN_REG         0x310 /* MAC_CFG_RCV_HOLD_EN */

#define PCIe_HIPCIEC_MAC_REG_MAC_FRAMING_ERR_CTRL_REG        0x3A0 /* MAC_FRAMING_ERR_CTRL */

#define PCIe_HIPCIEC_MAC_REG_MAC_REG_USE_FIX_CORE_CLK_EN     0x4EC /* MAC_REG_USE_FIX_CORE_CLK_EN */
#define PCIe_HIPCIEC_MAC_REG_MAC_REG_PIPE_MODE_SEL           0x508
#define PCIe_HIPCIEC_MAC_REG_MAC_TXELEIDLE_TM                0x530 /* MAC_TXELEIDLE_TM */
#define PCIe_HIPCIEC_MAC_REG_MAC_TARGET_LINK_WIDTH           0x534

#define PCIe_HIPCIEC_MAC_REG_MAC_REG_PRESET2                 0x208
#define PCIe_MAC_PRESET2_FFE_CONFIG                          0x51700 /* FFE config (0,23,5) */

/********************************************************************************************/
/*                   PCIE DL Reg                                                            */
/********************************************************************************************/
#define PCIe_HIPCIEC_DL_REG_ACK_LATENCY_CYCLE_G1_REG 0x0C  /* ACK_LATENCY_CYCLE_G1 */
#define PCIe_HIPCIEC_DL_REG_ACK_LATENCY_CYCLE_G2_REG 0x10  /* ACK_LATENCY_CYCLE_G2 */
#define TX_FC_UPDATE_P_CYCLE_G1                      0x30
#define TX_FC_UPDATE_P_CYCLE_G2                      0x34
#define TX_FC_UPDATE_P_CYCLE_G3                      0x38
#define TX_FC_UPDATE_P_CYCLE_G4                      0x3C
#define PCIe_DL_REG_G3X4_DL_INT_STATUS_REG           0x88  /* DL_INT_STATUS */
#define PCIe_HIPCIEC_DL_REG_INIT_FC_SEND_EN_REG      0xDC  /* init fc send enable */
#define PCIe_HIPCIEC_DL_REG_DFX_SEND_EN_REG          0xE8  /* tx send enable */
#define TX_FC_UPDATE_NP_CYCLE_G1                     0x128
#define TX_FC_UPDATE_NP_CYCLE_G2                     0x12C
#define TX_FC_UPDATE_NP_CYCLE_G3                     0x130
#define TX_FC_UPDATE_NP_CYCLE_G4                     0x134
#define TX_FC_UPDATE_CPL_CYCLE_G1                    0x138
#define TX_FC_UPDATE_CPL_CYCLE_G2                    0x13C
#define TX_FC_UPDATE_CPL_CYCLE_G3                    0x140
#define TX_FC_UPDATE_CPL_CYCLE_G4                    0x144

#define PCIe_DL_UP_INT_STATUS_OFFSET                 18U   /* DL_INT_STATUS.dl_up_int_status bit[18] */

/********************************************************************************************/
/*                   PCIE TL Reg                                                            */
/********************************************************************************************/
#define PCIe_HIPCIEC_TL_REG_TL_TX_CTRL_REG                 0x0    /* Transaction Layer source arbiter control */
#define PCIe_HIPCIEC_TL_REG_TL_TX_DFX_CTRL_REG             0xA00  /* TL_TX_DFX_CTRL */
#define PCIe_HIPCIEC_TL_REG_TL_RX_POSTED_CREDIT_REG        0xB1C  /* TL RX CREDIT POSTED TLP INITIAL SIZE */
#define PCIe_HIPCIEC_TL_REG_TL_RX_NON_POSTED_CREDIT_REG    0xB20  /* TL RX CREDIT NON_POSTED TLP INITIAL SIZE */
#define PCIe_HIPCIEC_TL_REG_TL_RX_CPL_CREDIT_REG           0xB24  /* TL RX CREDIT CPL TLP INITIAL SIZE */
#define PCIe_HIPCIEC_TL_REG_TL_RX_CDT_INI_UP_REG           0xB28  /* TL RX CREDIT reconfigure enable */
#define PCIe_HIPCIEC_TL_REG_TL_RX_POSTED_CREDIT_DF_REG     0xB88  /* TL RX CREDIT POSTED TLP INITIAL SIZE */
#define PCIe_HIPCIEC_TL_REG_TL_RX_NON_POSTED_CREDIT_DF_REG 0xB8C  /* TL RX CREDIT NON_POSTED TLP INITIAL SIZE */
#define PCIe_HIPCIEC_TL_REG_TL_RX_CPL_CREDIT_DF_REG        0xB90  /* TL RX CREDIT CPL TLP INITIAL SIZE */
#define PCIe_HIPCIEC_TL_REG_TL_RX_CDT_INI_UP_DF_REG        0xB94  /* TL RX CREDIT reconfigure enable */

/************ TL_CORE_PF_REG****************/
#define TL_CFG_PF_FUNC_SEL        0x00
#define PFN_BAR0_MASK             0x10
#define PFN_BAR1_MASK             0x14
#define PFN_BAR2_MASK             0x18
#define PFN_BAR3_MASK             0x1C
#define PFN_BAR4_MASK             0x20
#define PFN_BAR5_MASK             0x24
#define PFn_BAR_ENABLE            0x2C

/********************************************************************************************/
/*                   PCIE AP_GLOBAL_REG                                                     */
/********************************************************************************************/
#define PCIE_CE_ENA               0x0008
#define PCIE_UNF_ENA              0x0010
#define PCIE_UF_ENA               0x0018

#define PCIE_MSI_MASK             0x00F4
#define PORT_INTX_ASSERT_MASK     0x01B0
#define PORT_INTX_DEASSERT_MASK   0x01B4

#define PCIE_AP_NI_ENA            0x0100
#define PCIE_AP_CE_ENA            0x0104
#define PCIE_AP_UNF_ENA           0x0108
#define PCIE_AP_UF_ENA            0x010c
#define PCIE_CORE_NI_ENA          0x0160
#define PCIE_CORE_CE_ENA          0x0164
#define PCIE_CORE_UNF_ENA         0x0168
#define PCIE_CORE_UF_ENA          0x016c

#define AP_PORT_EN_REG            0x0800
#define AP_APB_SYN_RST            0x0810
#define AP_AXI_SYN_RST            0x0814
#define AP_IDLE                   0x0C08

/********************************************************************************************/
/*                   PCIE AP_IOB_RX_COM_REG Reg                                             */
/********************************************************************************************/
#define IOB_RX_AML_SNOOP          0x1AAC
#define IOB_RX_AML_QOS_CTRL       0x1810U

/********************************************************************************************/
/*                   PCIE AP_IOB_TX_REG Reg                                                 */
/********************************************************************************************/
#define IOB_TX_ECAM_CONTROL0      0x600
#define IOB_TX_ECAM_CONTROL1      0x604
#define IOB_ECAM_BASE_ADDR_L      0x608
#define IOB_ECAM_BASE_ADDR_H      0x60C
#define IOB_TX_AXIS_CONTROL1      0xC04

/********************************************************************************************/
/*                    IO_SUBCTRL Reg                                                        */
/********************************************************************************************/
#define IO_SUB_REG_BASE                   0xA0120000U
#define IO_SUB_REG_SIZE                   0x10000UL
#define IO_SUB_PCIE_BRG_ICG_EN_REG        0x340  /* PCIE_BRG clock enable */
#define IO_SUB_PCIE0_CTRL0_ICG_EN_REG     0x348  /* PCIE0_CTRL0 clock enable */
#define IO_SUB_PCIE0_CTRL1_ICG_EN_REG     0x350  /* PCIE0_CTRL1 clock enable */
#define IO_SUB_PCIE1_CTRL0_ICG_EN_REG     0x358  /* PCIE1_CTRL0 clock enable */
#define IO_SUB_PCIE1_CTRL1_ICG_EN_REG     0x360  /* PCIE1_CTRL1 clock enable */
#define IO_SUB_PCIE2_CTRL0_ICG_EN_REG     0x368  /* PCIE2_CTRL0 clock enable */
#define IO_SUB_PCIE2_CTRL1_ICG_EN_REG     0x370  /* PCIE2_CTRL1 clock enable */
#define IO_SUB_PCIE3_CTRL0_ICG_EN_REG     0x378  /* PCIE3_CTRL0 clock enable */
#define IO_SUB_PCIE3_CTRL1_ICG_EN_REG     0x380  /* PCIE3_CTRL1 clock enable */
#define IO_SUB_PCIE0_CTRL_RESET_DREQ_REG  0x464
#define IO_SUB_PCIE1_CTRL_RESET_DREQ_REG  0x46C
#define IO_SUB_PCIE2_CTRL_RESET_DREQ_REG  0x474
#define IO_SUB_PCIE3_CTRL_RESET_DREQ_REG  0x47C

/********************************************************************************************/
/*                          PCIE PCS Reg                                                    */
/********************************************************************************************/
#define PCS_LANE_REG_BASE               0x1000U
#define PCS_MSG_PIN_EN                  0x800U
#define PCS_MISC_CTRL_PARA_0            0x80CU
#define PCS_POWER_CHANGE_1US            0x004
#define PCS_TXE_DLY_NUM_MASK            0xF0FFFFFFU /* POWER_CHANGE_1US.cfg_txelecidle_dly_num(bit27~bit24) */
#define LANE_EBUF_PARA                  0x50U
#define PCS_LANE_OFFSET                 0x80U

/********************************************************************************************/
/*                          PCIE IP RESET                                                   */
/********************************************************************************************/
#define AP_IDLE_REG_ADDR 0xC08
#define AP_IDLE_STATUS_WAIT_TIME 10    /* 10ms */
#define AP_STATUS_IDLE 0x1U
#define AP_STATUS_IDLE_WAIT_MAX  10    /* total 100ms */
#define PCIE_CE_ENA_ADDR 0x8
#define PCIE_UNF_ENA_ADDR 0x10
#define PCIE_UF_ENA_ADDR 0x18

#define PCIE_AP_CE_ENA_ADDR 0x104
#define PCIE_AP_UNF_ENA_ADDR 0x108
#define PCIE_AP_UF_ENA_ADDR 0x10C
#define PCIE_AP_NI_ENA_ADDR 0x100
#define PCIE_CORE_CE_ENA_ADDR 0x164
#define PCIE_CORE_UNF_ENA_ADDR 0x168
#define PCIE_CORE_UF_ENA_ADDR 0x16C
#define PCIE_CORE_NI_ENA_ADDR 0x160

#define PCIE_MSI_MASK_ADDR 0xF4
#define PORT_INTX_ASSERT_MASK_ADDR 0x1b0
#define PORT_INTX_DEASSERT_MASK_ADDR 0x1b4

#define PORT_EN_ADDR 0x400

#define AP_AXI_SYN_RST_ADDR 0x814
#define AP_APB_SYN_RST_ADDR 0x810

#define PORT_RESET_ADDR 0x0

#define PCIE_CORE_TL_COM_SFT_RST_ADDR 0x14
#define PCIE_CORE_PHY_COM_SFT_RST_ADDR 0x18

#define PHY_RESET_ADDR 0x4
#define PCS_SOFT_REST_ADDR 0xC

#define PCS_SOFT_REST_ADDR 0xC

#define PCIE_MODULE_RESET_WAIT_TIME 20    /* 20ms */

#define PCIE_SELF_HEAL_MAX_TIMES 2

#define PCIE_LINK_UP_WAIT_TIME                    10000    /* 10ms */
#define PCIE_LINK_UP_WAIT_TIME_RANGE              10
#define PCIE_LINK_UP_WAIT_MAX_TIMES               200   /* total 2s */
struct port_global_cfg {
    u32 port_en;    // port enable, 1:enable, 0:disable
    u32 port_mode;  // RC OR EP
    u32 lane_num;
    u32 linkdown_clr_port_en;
    u32 linkdown_rst_en;
};

struct crg_reset {
    u32 ctrl0_en_clock;
    u32 ctrl1_en_clock;
    u32 ctrl_reset;
};

struct equalization_cfg {
    u32 adj_en;
    u32 timer;

    u32 test_en;
    u32 reg_phy_eq_fb_sel;
    u32 reg_8g_hilink_mode_en;
    u32 gen3_eq_rcv_lock_af_phase13_ctle_apt_en;
    u32 rcv_speed_pull_up_rx_preset_en;
    u32 gen3_eq_phase23_ctle_apt_en;
    u32 rcv_lock_hold_rate;
    u32 reg_switch_eq_mode_mask;
    u32 gen4_eq_rcv_lock_af_phase13_ctle_apt_en;
    u32 gen4_eq_phase23_ctle_apt_en;
    u32 gen3_eq_phase01_ctle_apt_en;
    u32 gen4_eq_phase01_ctle_apt_en;
    u32 gen3_eq_phase23_ffe_ctle_adj_en;
    u32 gen4_eq_phase23_ffe_ctle_adj_en;
    u32 reg_16g_hilink_mode_en;
    u32 reg_16g_ds_use_rx_preset_en;
    u32 reg_8g_eq_fix_lp_tx_preset;
    u32 reg_16g_eq_fix_lp_tx_preset;
    u32 reg_8g_eq_fix_lp_tx_use_preset;
    u32 reg_16g_eq_fix_lp_tx_use_preset;
    /* dfx-->set timer */
    u32 cfg_phase23_hold_timer;
    u32 cfg_rcv_lock_hold_timer;
    u32 cfg_phase0_hold_timer;
    u32 mac_preset_table0;
    u32 reg_preset_num;
    u32 mac_preset_table1;
    u32 cfg_merit_step;
};

struct port_mac_cfg {
    u32 auto_speed_change_en;
    u32 first_auto_speed_change_en;
    u32 auto_speed_disable_mask;

    u32 rc_link_number;
    u32 scrambel_disable_g3;
    u32 scrambel_disable_g12;
    u32 skp_intval_sris;
    u32 skp_intval_srns;
    u32 n_fts;

    u32 debug_en;

    u32 disable_enter_disable;
    u32 disable_enter_hotreset;
    u32 disable_enter_loopback;
    u32 disable_enter_compliance;

    u32 lane_num;
    u32 data_width_16;
};

struct port_dl_link_cfg {
    /* less than 34 us */
    u32 tx_fc_init_cycle;

    u32 init_fc_send_en_valid;
    u32 init_fc_send_en_val;
};

struct port_tl_cfg {
    u32 ph_vc0_cdt;
    u32 pd_vc0_cdt;
    u32 nph_vc0_cdt;
    u32 npd_vc0_cdt;
    u32 cplh_vc0_cdt;
    u32 cpld_vc0_cdt;

    u32 ph_vc0_scale;
    u32 pd_vc0_scale;
    u32 nph_vc0_scale;
    u32 npd_vc0_scale;
    u32 cplh_vc0_scale;
    u32 cpld_vc0_scale;
    u32 ph_vc0_cdt_df;
    u32 pd_vc0_cdt_df;
    u32 nph_vc0_cdt_df;
    u32 npd_vc0_cdt_df;
    u32 cplh_vc0_cdt_df;
    u32 cpld_vc0_cdt_df;
};

struct ltssm_cfg {
    u32 max_sup_width;
    u32 max_lk_spd;
    u32 target_speed;
    struct port_global_cfg glb_cfg;
    struct equalization_cfg equ_cfg;
    struct port_mac_cfg mac_cfg;
    struct port_dl_link_cfg dl_cfg;
    struct port_tl_cfg tl_cfg;
};

struct atu_cfg_for_pf {
    u64 offset;
    u32 size;
};

struct ecam_space_cfg {
    u32 base_addr_h;
    u32 base_addr_l;
    u32 start_bus;
};

typedef struct {
    u32 enable;
    u32 core_id;
    u32 max_link_width;
    u32 max_link_speed;
    u32 target_link_speed;
} pcie_info;

/* Define the union U_LINK_CAPBILITY */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int max_link_speed : 4;          /* [3..0]  */
        unsigned int max_link_width : 6;          /* [9..4]  */
        unsigned int aspm_sup : 2;                /* [11..10]  */
        unsigned int l0_exit_lat : 3;             /* [14..12]  */
        unsigned int l1_exit_lat : 3;             /* [17..15]  */
        unsigned int clock_pm : 1;                /* [18]  */
        unsigned int surprise_dn_err_rpt_cap : 1; /* [19]  */
        unsigned int dl_link_act_rpt_cap : 1;     /* [20]  */
        unsigned int link_band_notice_cap : 1;    /* [21]  */
        unsigned int aspm_opt_compliance : 1;     /* [22]  */
        unsigned int reserved_0 : 1;              /* [23]  */
        unsigned int port_num : 8;                /* [31..24]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_LINK_CAPBILITY;

/* Define the union U_LINK_CTRL_STATUS2 */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int target_link_speed : 4;             /* [3..0]  */
        unsigned int enter_compliance : 1;              /* [4]  */
        unsigned int hw_auto_speed_dis : 1;             /* [5]  */
        unsigned int selectable_de_emphasis : 1;        /* [6]  */
        unsigned int transmit_margin : 3;               /* [9..7]  */
        unsigned int enter_mod_compliance : 1;          /* [10]  */
        unsigned int compliance_sos : 1;                /* [11]  */
        unsigned int compliance_preset_deemp : 4;       /* [15..12]  */
        unsigned int cur_deemp_level : 1;               /* [16]  */
        unsigned int eq_8g_complete : 1;                /* [17]  */
        unsigned int eq_8g_phase1_success : 1;          /* [18]  */
        unsigned int eq_8g_phase2_success : 1;          /* [19]  */
        unsigned int eq_8g_phase3_success : 1;          /* [20]  */
        unsigned int link_8g_eq_req : 1;                /* [21]  */
        unsigned int retimer_presence_detect : 1;       /* [22]  */
        unsigned int retimer2_presence_detect : 1;      /* [23]  */
        unsigned int crosslink_resolution : 2;          /* [25..24]  */
        unsigned int reserved_0 : 2;                    /* [27..26]  */
        unsigned int downstream_component_presence : 3; /* [30..28]  */
        unsigned int drs_msg_recved : 1;                /* [31]  */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_LINK_CTRL_STATUS2;

/* Define the union U_SRIOV_CAP */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int vf_mig_cap : 1;        /* [0]  */
        unsigned int ari_cap_hier_pres : 1; /* [1]  */
        unsigned int vf_10bit_tag_sup : 1;  /* [2]  */
        unsigned int reserved_0 : 18;       /* [20..3]  */
        unsigned int vf_mig_int : 11;       /* [31..21]  */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_SRIOV_CAP;

/* Define the union U_MSIX_CAP_HEADER */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int msix_cap_id : 8;        /* [7..0]   */
        unsigned int msix_next_cap_addr : 8; /* [15..8]  */
        unsigned int msix_table_size : 11;   /* [26..16] */
        unsigned int reserved_0 : 3;         /* [29..27] */
        unsigned int msix_func_mask : 1;     /* [30]  */
        unsigned int msix_enable : 1;        /* [31]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_MSIX_CAP_HEADER;

/* Define the union U_MAC_REG_DC_BALANCE_DISABLE */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int reg_dc_balance_disable : 1;  /* [0]  */
        unsigned int reserved_0             : 31; /* [31..1]  */
    } bits;
    /* Define an unsigned member */
    unsigned int    u32;
} U_MAC_REG_DC_BALANCE_DISABLE;

/* Define the union U_MAC_REG_EQ_DISABLE */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int reg_equalization_disable : 1; /* [0]  */
        unsigned int reg_ds_eq_p23_disable : 1;    /* [1]  */
        unsigned int reserved_0            : 30;   /* [31..2]  */
    } bits;
    /* Define an unsigned member */
    unsigned int    u32;
} U_MAC_REG_EQ_DISABLE;

/* Define the union U_MAC_REG_ADJ_HILINK_MODE_EN */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int reg_8g_hilink_mode_en : 6;                   /* [5..0]  */
        unsigned int gen3_eq_rcv_lock_af_phase13_ctle_apt_en : 1; /* [6]  */
        unsigned int rcv_speed_pull_up_rx_preset_en : 4;          /* [10..7]  */
        unsigned int gen3_eq_phase23_ctle_apt_en : 1;             /* [11]  */
        unsigned int rcv_lock_hold_rate : 4;                      /* [15..12]  */
        unsigned int reg_switch_eq_mode_mask : 1;                 /* [16]  */
        unsigned int gen4_eq_rcv_lock_af_phase13_ctle_apt_en : 1; /* [17]  */
        unsigned int gen4_eq_phase23_ctle_apt_en : 1;             /* [18]  */
        unsigned int gen3_eq_phase01_ctle_apt_en : 1;             /* [19]  */
        unsigned int gen4_eq_phase01_ctle_apt_en : 1;             /* [20]  */
        unsigned int gen3_eq_phase23_ffe_ctle_adj_en : 1;         /* [21]  */
        unsigned int gen4_eq_phase23_ffe_ctle_adj_en : 1;         /* [22]  */
        unsigned int reserved_0 : 1;                              /* [23]  */
        unsigned int reg_16g_hilink_mode_en : 6;                  /* [29..24]  */
        unsigned int reg_16g_ds_use_rx_preset_en : 1;             /* [30]  */
        unsigned int reserved_1 : 1;                              /* [31]  */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_MAC_REG_ADJ_HILINK_MODE_EN;

/* Define the union U_MAC_REG_EQ_FIX_LP_TX_PRESET */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int reg_8g_eq_fix_lp_tx_preset : 4;  /* [3..0]  */
        unsigned int reg_16g_eq_fix_lp_tx_preset : 4; /* [7..4]  */
        unsigned int reserved_0 : 24;                 /* [31..8] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_MAC_REG_EQ_FIX_LP_TX_PRESET;

/* Define the union U_MAC_FRAMING_ERR_CTRL */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int reg_framing_err_retrain_en : 1; /* [0]  */
        unsigned int reg_framing_err_rpt_en : 1;     /* [1]  */
        unsigned int reserved_0 : 6;                 /* [7..2]   */
        unsigned int reg_framing_mask : 8;           /* [15..8]  */
        unsigned int resvered : 16;                  /* [31..16] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_MAC_FRAMING_ERR_CTRL;

/* Define the union U_TL_RX_POSTED_CREDIT */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int tl_ph_vc0_cdt : 12; /* [11..0]  */
        unsigned int reserved_0 : 4;     /* [15..12] */
        unsigned int tl_pd_vc0_cdt : 16; /* [31..16] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_TL_RX_POSTED_CREDIT;

/* Define the union U_TL_RX_NON_POSTED_CREDIT */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int tl_nph_vc0_cdt : 12; /* [11..0]  */
        unsigned int reserved_0 : 4;      /* [15..12] */
        unsigned int tl_npd_vc0_cdt : 16; /* [31..16] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_TL_RX_NON_POSTED_CREDIT;

/* Define the union U_TL_RX_CPL_CREDIT */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int tl_cplh_vc0_cdt : 12; /* [11..0]  */
        unsigned int reserved_0 : 4;       /* [15..12] */
        unsigned int tl_cpld_vc0_cdt : 16; /* [31..16] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_TL_RX_CPL_CREDIT;

/* Define the union U_TL_RX_POSTED_CREDIT_DF */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int tl_ph_vc0_cdt_df : 12; /* [11..0]  */
        unsigned int tl_pd_vc0_scale : 2;   /* [13..12] */
        unsigned int tl_ph_vc0_scale : 2;   /* [15..14] */
        unsigned int tl_pd_vc0_cdt_df : 16; /* [31..16] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_TL_RX_POSTED_CREDIT_DF;

/* Define the union U_TL_RX_NON_POSTED_CREDIT_DF */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int tl_nph_vc0_cdt_df : 12; /* [11..0]  */
        unsigned int tl_npd_vc0_scale : 2;   /* [13..12] */
        unsigned int tl_nph_vc0_scale : 2;   /* [15..14] */
        unsigned int tl_npd_vc0_cdt_df : 16; /* [31..16] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_TL_RX_NON_POSTED_CREDIT_DF;

/* Define the union U_TL_RX_CPL_CREDIT_DF */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int tl_cplh_vc0_cdt_df : 12; /* [11..0]  */
        unsigned int tl_cpld_vc0_scale : 2;   /* [13..12] */
        unsigned int tl_cplh_vc0_scale : 2;   /* [15..14] */
        unsigned int tl_cpld_vc0_cdt_df : 16; /* [31..16] */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_TL_RX_CPL_CREDIT_DF;

/* Define the union U_TL_RX_CDT_INI_UP_DF */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int tl_cdt_ini_up_df : 1;   /* [0]  */
        unsigned int reserved_0 : 7;         /* [7..1]  */
        unsigned int tl_cpl_cdt_infi_en : 1; /* [8]  */
        unsigned int reserved_1 : 23;        /* [31..9] */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_TL_RX_CDT_INI_UP_DF;

/* Define the union U_TL_RX_CDT_INI_UP */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int tl_cdt_ini_up : 1; /* [0]  */
        unsigned int reserved_0 : 31;   /* [31..1]  */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_TL_RX_CDT_INI_UP;

/* Define the union U_AXIM_GLOBAL_CTRL */
typedef union {
    /* Define the struct bits */
    struct {
        unsigned int ctrl_shutdown_req : 1;         /* [0]  */
        unsigned int ctrl_en_rd_256byte : 1;        /* [1]  */
        unsigned int ctrl_en_wr_256byte : 1;        /* [2]  */
        unsigned int ctrl_partial_write_64byte : 1; /* [3]  */
        unsigned int ctrl_lat_stat_rd_en : 1;       /* [4]  */
        unsigned int ctrl_lat_stat_wr_en : 1;       /* [5]  */
        unsigned int reserved_0 : 2;                /* [7..6]  */
        unsigned int ctrl_axuser_update_en : 1;     /* [8]  */
        unsigned int reserved_1 : 23;               /* [31..9]  */
    } bits;

    /* Define an unsigned member */
    unsigned int u32;
} U_AXIM_GLOBAL_CTRL;

typedef union {
    /* Define the struct bits */
    struct {
        unsigned int disable_enter_loopback : 3;     /* [2..0]  */
        unsigned int disable_enter_disable : 3;      /* [5..3]  */
        unsigned int disable_enter_hotreset : 1;     /* [6]  */
        unsigned int auto_speed_disable_mask : 1;    /* [7]  */
        unsigned int first_auto_speed_change_en : 1; /* [8]  */
        unsigned int auto_speed_change_en : 1;       /* [9]  */
        unsigned int disable_enter_compliance : 1;   /* [10]  */
        unsigned int gen3_low_latency_mode : 1;      /* [11]  */
        unsigned int reg_disable_ctrl_skp : 1;       /* [12]  */
        unsigned int reg_new_gen4_eieos_en : 1;      /* [13]  */
        unsigned int disable_scramber_disable : 1;   /* [14]  */

        unsigned int reg_new_esm_20g_25g_eieos_en : 1;           /* [15]  */
        unsigned int reg_pipe_data_width_sel : 8;                /* [23..16]  */
        unsigned int reg_core_clk_frequency : 4;                 /* [27..24]  */
        unsigned int reg_txdata_valid_mode : 1;                  /* [28]  */
        unsigned int reg_new_gen5_eieos_en : 1;                  /* [29]  */
        unsigned int reg_modify_ts_disable : 1;                  /* [30]  */
        unsigned int reg_req_bypass_disable_assert : 1;          /* [31]  */
    } bits;
    /* Define an unsigned member */
    unsigned int u32;
} U_MAC_ENTER_LPBK_DISABLE;

int drv_pcierc_ctrl_uninit(struct platform_device *pdev);
void drv_pcierc_ctrl_init(u32 core_id);
int pcie_rc_suspend_qos_config(u32 core_id, struct platform_device *pdev);
int pcie_rc_resume_qos_config(u32 core_id, struct platform_device *pdev);
phys_addr_t drv_pcierc_get_reg_base(u32 core_id);
void drv_pcierc_wait_link_up(u32 core_id, u32 port_id, bool *self_heal_flag);
void drv_pcierc_resume_self_heal_handle(void);

void drv_pcierc_mac_write(u32 core_id, u32 port, u32 reg, u32 value);
u32 drv_pcierc_mac_read(u32 core_id, u32 port, u32 reg);
#endif // PCIE_RC_UT
#endif // PCIE_RC_DRV_H
