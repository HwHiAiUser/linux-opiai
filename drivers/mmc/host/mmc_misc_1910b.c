// SPDX-License-Identifier: GPL-2.0-only
/*
 * HiSilicon Hi1910B MMC subctrl helpers.
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/io.h>

#include "mmc_misc.h"

#define SC_EMMC_PHY_RESET_REQ_REG	0xa60
#define SRST_REQ_EMMC_PHY_TX		BIT(1)
#define SRST_REQ_EMMC_PHY_RX		BIT(0)
#define SC_EMMC_PHY_RESET_DREQ_REG	0xa64
#define SRST_DREQ_EMMC_PHY_APB		BIT(2)
#define SRST_DREQ_EMMC_PHY_TX		BIT(1)
#define SRST_DREQ_EMMC_PHY_RX		BIT(0)
#define SC_EMMC_PHY_RESET_ST_REG	0x5a60
#define SRST_ST_EMMC_PHY_TX		BIT(1)
#define SRST_ST_EMMC_PHY_RX		BIT(0)
#define SC_SD_RESET_REQ_REG		0xa68
#define SRST_REQ_SD			BIT(1)
#define SRST_REQ_SD_APB			BIT(0)
#define SC_SD_RESET_DREQ_REG		0xa6c
#define SRST_DREQ_SD			BIT(1)
#define SRST_DREQ_SD_APB		BIT(0)
#define SC_SD_RESET_ST_REG		0x5a68
#define SRST_ST_SD			BIT(1)
#define SRST_ST_SD_APB			BIT(0)
#define SC_EMMC_RESET_REQ_REG		0xa58
#define SRST_REQ_EMMC			BIT(1)
#define SRST_REQ_EMMC_APB		BIT(0)
#define SC_EMMC_RESET_DREQ_REG		0xa5c
#define SRST_DREQ_EMMC			BIT(1)
#define SRST_DREQ_EMMC_APB		BIT(0)
#define SC_EMMC_RESET_ST_REG		0x5a58
#define SRST_ST_EMMC			BIT(1)
#define SRST_ST_EMMC_APB		BIT(0)

#define SC_EMMC_ICG_EN_REG		0x360
#define ICG_EN_EMMC			BIT(1)
#define ICG_EN_EMMC_APB			BIT(0)
#define SC_EMMC_ICG_DIS_REG		0x364
#define ICG_DIS_EMMC			BIT(1)
#define SC_EMMC_ICG_ST_REG		0x5360
#define ICG_ST_EMMC			BIT(1)
#define ICG_ST_EMMC_APB			BIT(0)
#define SC_EMMC_PHY_ICG_EN_REG		0x368
#define ICG_EN_EMMC_PHY_APB		BIT(0)
#define SC_EMMC_PHY_ICG_DIS_REG		0x36c
#define ICG_DIS_EMMC_PHY_APB		BIT(0)
#define SC_EMMC_PHY_ICG_ST_REG		0x5368
#define ICG_ST_EMMC_PHY_APB		BIT(0)
#define SC_SD_ICG_EN_REG		0x370
#define ICG_EN_SD			BIT(1)
#define ICG_EN_SD_APB			BIT(0)
#define SC_SD_ICG_DIS_REG		0x374
#define ICG_DIS_SD			BIT(1)
#define SC_SD_ICG_ST_REG		0x5370
#define ICG_ST_SD			BIT(1)
#define ICG_ST_SD_APB			BIT(0)

#define SC_SD_CLK_CFG_REG		0x160
#define SD_CLK_DIV_CFG			GENMASK(4, 1)
#define SD_CLK_SEL			BIT(0)
#define SC_EMMC_CLK_CFG_REG		0x164
#define EMMC_CLK_DIV_CFG		GENMASK(4, 1)
#define EMMC_CLK_SEL			BIT(0)
#define SC_SAMPLE_CCLK_SEL_REG		0x2400
#define EMMC_CLK_SEL_FIXED_CLK		0
#define EMMC_CLK_SEL_TUNED_CLK		BIT(0)

#define EMMC_POWER_EN_CTRL		0x2204
#define PAD_EMMC_POWER_EN_OUT		BIT(0)
#define PAD_SDIO_CARD_POWER_EN_OUT	BIT(1)

#define SC_SWITCH_S12V100_REG		0x150
#define SW_SEL				BIT(2)
#define SW_RST_IN			BIT(1)
#define SW_EN				BIT(0)
#define SC_SDIO_MS			0x3500
#define SDIO_PAD_3V3			0x0
#define SDIO_PAD_1V8			0x2
#define PADMG003			0x80c
#define PADMG004			0x810
#define PADMG005			0x814
#define PADMG006			0x818
#define PADMG007			0x81c
#define PADMG008			0x820

#define PERI_SUB_CTRL_BASE		0x80130000
#define SUB_CTRL_SIZE			0x10000
#define PERI_IOMUX_BASE			0x82320000
#define PERI_IOMUX_SIZE			0x10000
#define PERI_IOMG000			0x0
#define PAD_EMMC_POWER_EN		0x0
#define PAD_GPIO2_09			0x3
#define EFUSE_DC_FLAG_REG		0xc018e254
#define EFUSE_DC_FLAG_REG_BIT7		0x80

enum {
	MMC_0,
	MMC_MAX_NUM,
};

static void __iomem *g_emmc_subctrl_vbase[MMC_MAX_NUM];
static void __iomem *g_sd_subctrl_vbase[MMC_MAX_NUM];

static inline void sdhci_sctrl_writel(void __iomem *vbase, u32 val, u32 reg)
{
	writel(val, vbase + reg);
}

static inline u32 sdhci_sctrl_readl(void __iomem *vbase, u32 reg)
{
	return readl(vbase + reg);
}

int hisi_mmc_subctrl_init(u32 host_mode, u32 host_id, struct device *dev)
{
	if (host_id >= MMC_MAX_NUM)
		return -EINVAL;

	if (host_mode == 0) {
		g_emmc_subctrl_vbase[host_id] =
			devm_ioremap(dev, PERI_SUB_CTRL_BASE, SUB_CTRL_SIZE);
		if (!g_emmc_subctrl_vbase[host_id]) {
			dev_err(dev, "emmc subctrl devm_ioremap failed\n");
			return -ENOMEM;
		}
	} else {
		g_sd_subctrl_vbase[host_id] =
			devm_ioremap(dev, PERI_SUB_CTRL_BASE, SUB_CTRL_SIZE);
		if (!g_sd_subctrl_vbase[host_id]) {
			dev_err(dev, "sd subctrl devm_ioremap failed\n");
			return -ENOMEM;
		}
	}

	return 0;
}

int hisi_subctrl_emmc_clkgate_enable(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id],
			   ICG_EN_EMMC | ICG_EN_EMMC_APB, SC_EMMC_ICG_EN_REG);
	return 0;
}

int hisi_subctrl_sd_clkgate_enable(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id],
			   ICG_EN_SD | ICG_EN_SD_APB, SC_SD_ICG_EN_REG);
	return 0;
}

int hisi_subctrl_emmc_clkgate_disable(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id], ICG_DIS_EMMC,
			   SC_EMMC_ICG_DIS_REG);
	return 0;
}

int hisi_subctrl_sd_clkgate_disable(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id], ICG_DIS_SD,
			   SC_SD_ICG_DIS_REG);
	return 0;
}

bool hisi_subctrl_emmc_clkgate_is_enabled(struct device *mmc_dev, u32 host_id)
{
	u32 val = sdhci_sctrl_readl(g_emmc_subctrl_vbase[host_id],
				    SC_EMMC_ICG_ST_REG);
	u32 mask = ICG_ST_EMMC | ICG_ST_EMMC_APB;

	return (val & mask) == mask;
}

bool hisi_subctrl_sd_clkgate_is_enabled(struct device *mmc_dev, u32 host_id)
{
	u32 val = sdhci_sctrl_readl(g_sd_subctrl_vbase[host_id], SC_SD_ICG_ST_REG);
	u32 mask = ICG_ST_SD | ICG_ST_SD_APB;

	return (val & mask) == mask;
}

int hisi_subctrl_emmc_reset_assert(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id], SRST_REQ_EMMC,
			   SC_EMMC_RESET_REQ_REG);
	return 0;
}

int hisi_subctrl_sd_reset_assert(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id],
			   SRST_REQ_SD | SRST_REQ_SD_APB, SC_SD_RESET_REQ_REG);
	return 0;
}

int hisi_subctrl_emmc_reset_deassert(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id],
			   SRST_DREQ_EMMC | SRST_DREQ_EMMC_APB,
			   SC_EMMC_RESET_DREQ_REG);
	return 0;
}

int hisi_subctrl_sd_reset_deassert(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id],
			   SRST_DREQ_SD | SRST_DREQ_SD_APB,
			   SC_SD_RESET_DREQ_REG);
	return 0;
}

int hisi_subctrl_emmc_get_reset_status(struct device *mmc_dev, u32 host_id)
{
	u32 val = sdhci_sctrl_readl(g_emmc_subctrl_vbase[host_id],
				    SC_EMMC_RESET_ST_REG);
	u32 mask = SRST_ST_EMMC | SRST_ST_EMMC_APB;

	return !!(val & mask);
}

int hisi_subctrl_sd_get_reset_status(struct device *mmc_dev, u32 host_id)
{
	u32 val = sdhci_sctrl_readl(g_sd_subctrl_vbase[host_id], SC_SD_RESET_ST_REG);
	u32 mask = SRST_ST_SD | SRST_ST_SD_APB;

	return !!(val & mask);
}

int hisi_subctrl_emmc_phy_clkgate_enable(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id], ICG_EN_EMMC_PHY_APB,
			   SC_EMMC_PHY_ICG_EN_REG);
	return 0;
}

int hisi_subctrl_emmc_phy_clkgate_disable(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id], ICG_DIS_EMMC_PHY_APB,
			   SC_EMMC_PHY_ICG_DIS_REG);
	return 0;
}

bool hisi_subctrl_emmc_phy_clkgate_is_enabled(struct device *mmc_dev,
					      u32 host_id)
{
	u32 val = sdhci_sctrl_readl(g_emmc_subctrl_vbase[host_id],
				    SC_EMMC_PHY_ICG_ST_REG);

	return !!(val & ICG_ST_EMMC_PHY_APB);
}

int hisi_subctrl_emmc_phy_reset_assert(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id],
			   SRST_REQ_EMMC_PHY_TX | SRST_REQ_EMMC_PHY_RX,
			   SC_EMMC_PHY_RESET_REQ_REG);
	return 0;
}

int hisi_subctrl_emmc_phy_reset_deassert(struct device *mmc_dev, u32 host_id)
{
	sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id],
			   SRST_DREQ_EMMC_PHY_APB | SRST_DREQ_EMMC_PHY_TX |
			   SRST_DREQ_EMMC_PHY_RX,
			   SC_EMMC_PHY_RESET_DREQ_REG);
	return 0;
}

int hisi_subctrl_emmc_phy_get_reset_status(struct device *mmc_dev, u32 host_id)
{
	u32 val = sdhci_sctrl_readl(g_emmc_subctrl_vbase[host_id],
				    SC_EMMC_PHY_RESET_ST_REG);
	u32 mask = SRST_ST_EMMC_PHY_TX | SRST_ST_EMMC_PHY_RX;

	return !!(val & mask);
}

int hisi_subctrl_emmc_set_clk_cfg(struct device *mmc_dev, u32 host_id,
				  u32 sel, u32 div)
{
	u32 val = sdhci_sctrl_readl(g_emmc_subctrl_vbase[host_id],
				    SC_EMMC_CLK_CFG_REG);

	val &= ~EMMC_CLK_DIV_CFG;
	val |= FIELD_PREP(EMMC_CLK_DIV_CFG, div);
	if (sel == 0)
		val &= ~EMMC_CLK_SEL;
	else
		val |= EMMC_CLK_SEL;
	sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id], val,
			   SC_EMMC_CLK_CFG_REG);
	return 0;
}

int hisi_subctrl_sd_set_clk_cfg(struct device *mmc_dev, u32 host_id,
				u32 sel, u32 div)
{
	u32 val = sdhci_sctrl_readl(g_sd_subctrl_vbase[host_id], SC_SD_CLK_CFG_REG);

	val &= ~SD_CLK_DIV_CFG;
	val |= FIELD_PREP(SD_CLK_DIV_CFG, div);
	if (sel == 0)
		val &= ~SD_CLK_SEL;
	else
		val |= SD_CLK_SEL;
	sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id], val, SC_SD_CLK_CFG_REG);
	return 0;
}

int hisi_subctrl_emmc_set_sample_clk(struct device *mmc_dev, u32 host_id,
				     u32 sel)
{
	if (sel == 0)
		sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id],
				   EMMC_CLK_SEL_FIXED_CLK,
				   SC_SAMPLE_CCLK_SEL_REG);
	else
		sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id],
				   EMMC_CLK_SEL_TUNED_CLK,
				   SC_SAMPLE_CCLK_SEL_REG);
	return 0;
}

int hisi_subctrl_emmc_set_power_en(struct device *mmc_dev, u32 host_id, u32 en)
{
	u32 reg_val;
	void __iomem *peri_iomux_vbase;

	peri_iomux_vbase = ioremap(PERI_IOMUX_BASE, PERI_IOMUX_SIZE);
	if (!peri_iomux_vbase) {
		pr_err("ioremap failed\n");
		return -ENODEV;
	}

	reg_val = sdhci_sctrl_readl(g_emmc_subctrl_vbase[host_id],
				    EMMC_POWER_EN_CTRL);
	if (en == 1) {
		writel(PAD_GPIO2_09, peri_iomux_vbase + PERI_IOMG000);
		udelay(100);
		reg_val |= PAD_EMMC_POWER_EN_OUT;
	} else {
		reg_val &= ~PAD_EMMC_POWER_EN_OUT;
	}

	sdhci_sctrl_writel(g_emmc_subctrl_vbase[host_id], reg_val,
			   EMMC_POWER_EN_CTRL);
	writel(PAD_EMMC_POWER_EN, peri_iomux_vbase + PERI_IOMG000);
	iounmap(peri_iomux_vbase);
	return 0;
}

int hisi_subctrl_sd_set_power_en(struct device *mmc_dev, u32 host_id, u32 en)
{
	u32 reg_val = sdhci_sctrl_readl(g_sd_subctrl_vbase[host_id],
					EMMC_POWER_EN_CTRL);

	if (en == 1)
		reg_val |= PAD_SDIO_CARD_POWER_EN_OUT;
	else
		reg_val &= ~PAD_SDIO_CARD_POWER_EN_OUT;

	sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id], reg_val,
			   EMMC_POWER_EN_CTRL);
	return 0;
}

static bool hisi_subctrl_get_efuse_dc_flag(void)
{
	u32 reg;
	void __iomem *efuse_iomux;

	efuse_iomux = ioremap(EFUSE_DC_FLAG_REG, PERI_IOMUX_SIZE);
	if (!efuse_iomux) {
		pr_err("ioremap failed\n");
		return false;
	}

	reg = readl(efuse_iomux);
	iounmap(efuse_iomux);

	return !!(reg & EFUSE_DC_FLAG_REG_BIT7);
}

int hisi_subctrl_sd_volt_switch_1v8(struct device *mmc_dev, u32 host_id, u32 set)
{
	void __iomem *peri_iomux_vbase;
	u32 val;
	u32 temp;

	val = sdhci_sctrl_readl(g_sd_subctrl_vbase[host_id], SC_SWITCH_S12V100_REG);
	val |= SW_RST_IN;
	sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id], val, SC_SWITCH_S12V100_REG);
	val |= SW_EN;
	sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id], val, SC_SWITCH_S12V100_REG);

	peri_iomux_vbase = ioremap(PERI_IOMUX_BASE, PERI_IOMUX_SIZE);
	if (!peri_iomux_vbase) {
		pr_err("ioremap failed\n");
		return -ENODEV;
	}

	temp = readl(peri_iomux_vbase + PADMG004);
	temp = (temp & 0xfffeffff) | 0x00000070;
	writel(temp, peri_iomux_vbase + PADMG004);
	temp = readl(peri_iomux_vbase + PADMG005);
	temp = (temp & 0xfffeffff) | 0x00000070;
	writel(temp, peri_iomux_vbase + PADMG005);
	temp = readl(peri_iomux_vbase + PADMG006);
	temp = (temp & 0xfffeffff) | 0x00000070;
	writel(temp, peri_iomux_vbase + PADMG006);
	temp = readl(peri_iomux_vbase + PADMG007);
	temp = (temp & 0xfffeffff) | 0x00000070;
	writel(temp, peri_iomux_vbase + PADMG007);
	temp = readl(peri_iomux_vbase + PADMG008);
	temp = (temp & 0xfffeffff) | 0x00000070;
	writel(temp, peri_iomux_vbase + PADMG008);

	if (set == 1) {
		pr_info("switch to 1v8, config PADMG003\n");
		temp = readl(peri_iomux_vbase + PADMG003);
		if (hisi_subctrl_get_efuse_dc_flag())
			temp = (temp & 0xfffeff8f) | 0x00000040;
		else
			temp = (temp & 0xfffeff8f) | 0x00000050;
		writel(temp, peri_iomux_vbase + PADMG003);
		val |= SW_SEL;
	} else {
		pr_info("switch to 3v3, config PADMG003\n");
		writel(0x00010071, peri_iomux_vbase + PADMG003);
		val &= ~SW_SEL;
	}

	iounmap(peri_iomux_vbase);
	sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id], val, SC_SWITCH_S12V100_REG);
	mdelay(10);
	if (set == 1)
		sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id], SDIO_PAD_1V8,
				   SC_SDIO_MS);
	else
		sdhci_sctrl_writel(g_sd_subctrl_vbase[host_id], SDIO_PAD_3V3,
				   SC_SDIO_MS);

	return 0;
}

int sdhci_zodiac_get_clk_rst_info(struct sdhci_zodiac_host *zodiac_host)
{
	return 0;
}
