/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * HiSilicon Zodiac SDHCI host definitions.
 */

#ifndef _SDHCI_ZODIAC_MMC_H
#define _SDHCI_ZODIAC_MMC_H

#include <linux/mmc/host.h>

#define HISI_MMC_MAX_TIMING_MODE (MMC_TIMING_MMC_HS400 + 1)

struct sdhci_timing_cfg {
	u32 oe_mask_en;
	u32 read_auto_cmd12_start;
	u32 read_wait_start;
	u32 cmd_timeout_cnt;
	u32 cmd_resp_cnt;
	u32 stop_sdclk_rd_start;
	u32 crc_st_det_dly_mode;
	u32 crc_st_det_dly;
	u32 crc_st_chk_dly;
};

struct sdhci_clk_cfg {
	u32 clk_src;
	u32 clk_dly_sample;
	u32 clk_dly_drv;
	u32 clk_div;
	u32 sample_mode;
	u32 max_clk;
	struct sdhci_timing_cfg timing_cfg;
};

struct zodiac_region {
	void __iomem *io_base;
	u32 io_size;
};

struct zodiac_emmc_delay_config {
	u32 ratio;
	int margin;
};

enum zodiac_emmc_delay1_mode {
	ZODIAC_EMMC_DDR52 = 0,
	ZODIAC_EMMC_HS200,
	ZODIAC_EMMC_HS400,
	ZODIAC_EMMC_MODE_MAX,
};

enum zodiac_host_mode {
	ZODIAC_EMMC_MODE = 0,
	ZODIAC_SD_MODE,
	ZODIAC_SDIO_MODE,
};

struct sdhci_zodiac_host {
	struct device *dev;
	struct sdhci_host *host;
	struct zodiac_region phy;
	struct sdhci_clk_cfg clk_cfg[HISI_MMC_MAX_TIMING_MODE];
	struct zodiac_emmc_delay_config delay1_cfg[ZODIAC_EMMC_MODE_MAX];
	u32 host_mode;
	u32 host_id;
	u8 power_mode;
};

#endif
