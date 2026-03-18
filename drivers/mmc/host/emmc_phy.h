/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * HiSilicon Zodiac eMMC PHY helpers.
 */

#ifndef _EMMC_PHY_H
#define _EMMC_PHY_H

struct mmc_ios;
struct sdhci_host;

void sdhci_zodiac_dump_phy_regs(struct sdhci_host *host);
int sdhci_combo_phy_init(struct sdhci_host *host);
int sdhci_combo_phy_delay_measurement(struct sdhci_host *host,
				      struct mmc_ios *ios);

#endif
