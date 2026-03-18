// SPDX-License-Identifier: GPL-2.0-only
/*
 * HiSilicon Zodiac eMMC PHY helpers.
 */

#include <linux/bitfield.h>
#include <linux/delay.h>

#include "emmc_phy.h"
#include "sdhci-pltfm.h"
#include "sdhci_zodiac_mmc.h"

#define PHY_PHYINITCTRL			0x4
#define INIT_EN				0x1
#define DLYMEAS_EN			BIT(2)
#define ZCAL_EN				BIT(3)
#define BIST_CLK			BIT(17)
#define PHY_PHYINITSTAT			0x8
#define PHY_IMPCTRL			0x24
#define IMPCTRL_ZCOM_RSP_DLY_MASK	GENMASK(5, 0)
#define PHY_IMPSTATUS			0x28
#define PHY_DATENA_DLY			0x23c
#define PHY_PHYCTRL2			0x248
#define PHY_DLYMEAS_UPDATE		BIT(14)
#define PHY_DLYMEAS_1T_SEL		BIT(21)
#define PHY_DLY_CTL			0x250
#define DLY_CODE_1T_MASK		GENMASK(10, 0)
#define PHY_DLY_CTL1			0x254
#define DLY_3_CODE_MASK			GENMASK(9, 0)
#define DLY_3_CODE_SHIFT		8
#define DLY_2_CODE_MASK			GENMASK(10, 0)
#define DLY_2_CODE_SHIFT		8
#define DLY_1_CODE_MASK			GENMASK(9, 0)
#define DLY_1_CODE_SHIFT		21
#define INV_TX_CLK			BIT(31)
#define PHY_DLY_CTL2			0x258
#define PHY_IOCTL_PUPD			0x260
#define PUPD_EN_DATA			0xff
#define PUPD_EN_STROBE			BIT(8)
#define PUPD_EN_CMD			BIT(9)
#define PULLUP_DATA			(0xff << 16)
#define PULLUP_CMD			(0x200 << 16)
#define PHY_IOCTL_RONSEL_2		0x268
#define EMMC_RONSEL_2			0x7ff
#define PHY_IOCTL_IOE			0x26c
#define DA_EMMC_E			BIT(10)
#define DA_EMMC_IE			(0x7ff << 16)
#define PHY_IOCTL_INMODE		0x28c
#define RG_EMMC_INMODE			0x7ff
#define WAIT_DONE_MAX_TIMES		30

static inline void sdhci_phy_writel(struct sdhci_host *host, u32 val, u32 reg)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	writel(val, zodiac_host->phy.io_base + reg);
}

static inline u32 sdhci_phy_readl(struct sdhci_host *host, u32 reg)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	return readl(zodiac_host->phy.io_base + reg);
}

void sdhci_zodiac_dump_phy_regs(struct sdhci_host *host)
{
	pr_err("Start dump emmc phy register\n");
	pr_err("==================== emmc phy ====================");
	pr_err("PHY init ctrl: 0x%08x | init stat: 0x%08x\n",
	       sdhci_phy_readl(host, PHY_PHYINITCTRL),
	       sdhci_phy_readl(host, PHY_PHYINITSTAT));
	pr_err("PHY imp ctrl: 0x%08x | imp stat: 0x%08x\n",
	       sdhci_phy_readl(host, PHY_IMPCTRL),
	       sdhci_phy_readl(host, PHY_IMPSTATUS));
	pr_err("PHY dly ctrl: 0x%08x\n", sdhci_phy_readl(host, PHY_DLY_CTL));
	pr_err("PHY dly ctrl1: 0x%08x | dly ctrl2: 0x%08x\n",
	       sdhci_phy_readl(host, PHY_DLY_CTL1),
	       sdhci_phy_readl(host, PHY_DLY_CTL2));
	pr_err("PHY drv2: 0x%08x\n", sdhci_phy_readl(host, PHY_IOCTL_RONSEL_2));
}

static int sdhci_combo_phy_zq_cal(struct sdhci_host *host)
{
	u32 count = 0;
	u32 reg_val;

	reg_val = sdhci_phy_readl(host, PHY_IMPCTRL);
	reg_val &= ~IMPCTRL_ZCOM_RSP_DLY_MASK;
	reg_val |= FIELD_PREP(IMPCTRL_ZCOM_RSP_DLY_MASK, 0x3c);
	sdhci_phy_writel(host, reg_val, PHY_IMPCTRL);

	reg_val = sdhci_phy_readl(host, PHY_PHYINITCTRL);
	reg_val |= ZCAL_EN;
	reg_val &= ~DLYMEAS_EN;
	reg_val |= INIT_EN;
	sdhci_phy_writel(host, reg_val, PHY_PHYINITCTRL);

	do {
		if (count++ > WAIT_DONE_MAX_TIMES) {
			pr_err("phy_zq_cal wait timeout\n");
			pr_err("PHY init stat: 0x%08x\n",
			       sdhci_phy_readl(host, PHY_PHYINITSTAT));
			pr_err("the IMPSTATUS is %x\n",
			       sdhci_phy_readl(host, PHY_IMPSTATUS));
			return -ETIMEDOUT;
		}

		udelay(100);
		reg_val = sdhci_phy_readl(host, PHY_PHYINITCTRL);
	} while (reg_val & INIT_EN);

	return 0;
}

int sdhci_combo_phy_init(struct sdhci_host *host)
{
	u32 reg_val;
	int ret;

	reg_val = sdhci_phy_readl(host, PHY_PHYINITCTRL);
	reg_val |= BIST_CLK;
	sdhci_phy_writel(host, reg_val, PHY_PHYINITCTRL);
	sdhci_phy_writel(host, 0, PHY_DATENA_DLY);

	reg_val = PUPD_EN_DATA | PUPD_EN_STROBE | PUPD_EN_CMD |
		  PULLUP_DATA | PULLUP_CMD;
	sdhci_phy_writel(host, reg_val, PHY_IOCTL_PUPD);

	reg_val = sdhci_phy_readl(host, PHY_IOCTL_RONSEL_2);
	reg_val |= EMMC_RONSEL_2;
	sdhci_phy_writel(host, reg_val, PHY_IOCTL_RONSEL_2);

	reg_val = sdhci_phy_readl(host, PHY_IOCTL_IOE);
	reg_val |= DA_EMMC_E | DA_EMMC_IE;
	sdhci_phy_writel(host, reg_val, PHY_IOCTL_IOE);

	reg_val = sdhci_phy_readl(host, PHY_IOCTL_INMODE);
	reg_val &= ~RG_EMMC_INMODE;
	sdhci_phy_writel(host, reg_val, PHY_IOCTL_INMODE);

	reg_val = sdhci_phy_readl(host, PHY_DLY_CTL1);
	reg_val |= INV_TX_CLK;
	sdhci_phy_writel(host, reg_val, PHY_DLY_CTL1);

	ret = sdhci_combo_phy_zq_cal(host);
	if (ret)
		pr_err("sdhci_combo_phy_zq_cal fail\n");

	return ret;
}

static inline u32 sdhci_get_phy_delay1(u32 ratio, u32 margin, u32 delaymeas_code)
{
	if (!ratio)
		return margin;

	return ((delaymeas_code / ratio) & 0x7) +
	       ((delaymeas_code / ratio) & 0x3f8) + margin;
}

static void sdhci_config_phy_delay_ctl(struct sdhci_host *host,
				       struct mmc_ios *ios)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	u32 reg_val;
	u32 delaymeas_code;
	u32 delay1 = 0;
	u32 delay2;
	u32 delay3;

	reg_val = sdhci_phy_readl(host, PHY_DLY_CTL);
	delaymeas_code = reg_val & DLY_CODE_1T_MASK;
	reg_val = sdhci_phy_readl(host, PHY_DLY_CTL1);
	reg_val &= ~(DLY_1_CODE_MASK << DLY_1_CODE_SHIFT);

	if (ios->timing == MMC_TIMING_MMC_HS200)
		delay1 = sdhci_get_phy_delay1(
			zodiac_host->delay1_cfg[ZODIAC_EMMC_HS200].ratio,
			zodiac_host->delay1_cfg[ZODIAC_EMMC_HS200].margin,
			delaymeas_code);
	else if (ios->timing == MMC_TIMING_MMC_HS400)
		delay1 = sdhci_get_phy_delay1(
			zodiac_host->delay1_cfg[ZODIAC_EMMC_HS400].ratio,
			zodiac_host->delay1_cfg[ZODIAC_EMMC_HS400].margin,
			delaymeas_code);
	else if (ios->timing == MMC_TIMING_MMC_DDR52)
		delay1 = sdhci_get_phy_delay1(
			zodiac_host->delay1_cfg[ZODIAC_EMMC_DDR52].ratio,
			zodiac_host->delay1_cfg[ZODIAC_EMMC_DDR52].margin,
			delaymeas_code);

	reg_val |= (DLY_1_CODE_MASK & delay1) << DLY_1_CODE_SHIFT;
	sdhci_phy_writel(host, reg_val, PHY_DLY_CTL1);

	if (ios->timing == MMC_TIMING_MMC_DDR52) {
		reg_val = sdhci_phy_readl(host, PHY_DLY_CTL2);
		reg_val &= ~(DLY_2_CODE_MASK << DLY_2_CODE_SHIFT);
		delay2 = delaymeas_code / 2;
		reg_val |= (DLY_2_CODE_MASK & delay2) << DLY_2_CODE_SHIFT;
		sdhci_phy_writel(host, reg_val, PHY_DLY_CTL2);
	} else if (ios->timing == MMC_TIMING_MMC_HS400) {
		reg_val = sdhci_phy_readl(host, PHY_DLY_CTL1);
		reg_val &= ~(DLY_3_CODE_MASK << DLY_3_CODE_SHIFT);
		delay3 = ((delaymeas_code / 4) & 0x7) +
			 ((delaymeas_code / 4) & 0x3f8);
		reg_val |= (DLY_3_CODE_MASK & delay3) << DLY_3_CODE_SHIFT;
		sdhci_phy_writel(host, reg_val, PHY_DLY_CTL1);
	}
}

int sdhci_combo_phy_delay_measurement(struct sdhci_host *host,
				      struct mmc_ios *ios)
{
	u32 count = 0;
	u32 reg_val;

	reg_val = sdhci_phy_readl(host, PHY_PHYCTRL2);
	if (ios->timing == MMC_TIMING_MMC_DDR52)
		reg_val &= ~PHY_DLYMEAS_1T_SEL;
	else
		reg_val |= PHY_DLYMEAS_1T_SEL;
	reg_val |= PHY_DLYMEAS_UPDATE;
	sdhci_phy_writel(host, reg_val, PHY_PHYCTRL2);

	reg_val = sdhci_phy_readl(host, PHY_PHYINITCTRL);
	reg_val &= ~ZCAL_EN;
	reg_val |= INIT_EN | DLYMEAS_EN;
	sdhci_phy_writel(host, reg_val, PHY_PHYINITCTRL);

	do {
		if (count++ > WAIT_DONE_MAX_TIMES) {
			pr_err("wait for delay measurement done timeout\n");
			return -ETIMEDOUT;
		}

		udelay(100);
		reg_val = sdhci_phy_readl(host, PHY_PHYINITCTRL);
	} while (reg_val & INIT_EN);

	sdhci_config_phy_delay_ctl(host, ios);
	reg_val = sdhci_phy_readl(host, PHY_PHYCTRL2);
	reg_val &= ~PHY_DLYMEAS_UPDATE;
	sdhci_phy_writel(host, reg_val, PHY_PHYCTRL2);

	return 0;
}
