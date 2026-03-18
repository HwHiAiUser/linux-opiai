/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * HiSilicon Zodiac subctrl helpers.
 */

#ifndef _MMC_MISC_H
#define _MMC_MISC_H

#include <linux/device.h>

struct sdhci_zodiac_host;

int hisi_subctrl_emmc_clkgate_enable(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_sd_clkgate_enable(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_emmc_clkgate_disable(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_sd_clkgate_disable(struct device *mmc_dev, u32 host_id);
bool hisi_subctrl_emmc_clkgate_is_enabled(struct device *mmc_dev, u32 host_id);
bool hisi_subctrl_sd_clkgate_is_enabled(struct device *mmc_dev, u32 host_id);

int hisi_subctrl_emmc_phy_clkgate_enable(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_emmc_phy_clkgate_disable(struct device *mmc_dev, u32 host_id);
bool hisi_subctrl_emmc_phy_clkgate_is_enabled(struct device *mmc_dev,
					      u32 host_id);
int hisi_subctrl_emmc_phy_reset_assert(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_emmc_phy_reset_deassert(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_emmc_phy_get_reset_status(struct device *mmc_dev, u32 host_id);

int hisi_subctrl_emmc_reset_assert(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_sd_reset_assert(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_emmc_reset_deassert(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_sd_reset_deassert(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_emmc_get_reset_status(struct device *mmc_dev, u32 host_id);
int hisi_subctrl_sd_get_reset_status(struct device *mmc_dev, u32 host_id);

int hisi_subctrl_emmc_set_clk_cfg(struct device *mmc_dev, u32 host_id,
				  u32 sel, u32 div);
int hisi_subctrl_sd_set_clk_cfg(struct device *mmc_dev, u32 host_id,
				u32 sel, u32 div);
int hisi_subctrl_emmc_set_sample_clk(struct device *mmc_dev, u32 host_id,
				     u32 sel);
int hisi_subctrl_emmc_set_power_en(struct device *mmc_dev, u32 host_id, u32 en);
int hisi_subctrl_sd_set_power_en(struct device *mmc_dev, u32 host_id, u32 en);
int hisi_subctrl_sd_volt_switch_1v8(struct device *mmc_dev, u32 host_id,
				    u32 set);

int sdhci_zodiac_get_clk_rst_info(struct sdhci_zodiac_host *zodiac_host);
int hisi_mmc_subctrl_init(u32 host_mode, u32 host_id, struct device *dev);

#endif
