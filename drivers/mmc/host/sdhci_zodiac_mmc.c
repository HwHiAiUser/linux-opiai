// SPDX-License-Identifier: GPL-2.0-only
/*
 * HiSilicon Zodiac SDHCI host glue for Hi1910B/Ascend 310B.
 */

#include <linux/bitfield.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/property.h>

#include "emmc_phy.h"
#include "mmc_misc.h"
#include "sdhci-pltfm.h"
#include "sdhci_zodiac_mmc.h"

#define SDHCI_CRG_CTRL0			0x80
#define SDEMMC_SAMP_SEL		BIT(31)
#define SDEMMC_CLK_DIV		GENMASK(27, 24)
#define SDEMMC_CLK_DLY_DRV	GENMASK(20, 16)
#define SDEMMC_CLK_DLY_SAMPLE	GENMASK(12, 8)
#define TUNING_RESULT_VALUE_REG		0x88
#define SDEMMC_TIMING_CTRL_0		0x90
#define OE_MASK_EN			BIT(31)
#define CFG_READ_AUTO_CMD12_START	GENMASK(29, 24)
#define CFG_READ_WAIT_START		GENMASK(23, 21)
#define CFG_CMD_TIMEOUT_CNT		GENMASK(7, 0)
#define CFG_CMD_RESP_CNT		GENMASK(15, 8)
#define SDEMMC_TIMING_CTRL_1		0x94
#define CRC_ST_DET_DLY_MODE		BIT(15)
#define CRC_ST_DET_DLY			GENMASK(14, 8)
#define CRC_ST_CHK_DLY			GENMASK(7, 0)
#define SDEMMC_TIMING_CTRL_2		0x98
#define CFG_STOP_SDCLK_RD_START		GENMASK(4, 1)

#define SDHCI_ZODIAC_MIN_FREQ		100000
#define ZODIAC_MAX_TUNING_LOOP		40
#define SDHCI_ZODIAC_CLK_STABLE_TIME	500
#define ASYNC_MODE			1
#define L_CLK				0
#define H_CLK				1
#define FIXED_CLK			0
#define TUNED_CLK			1
#define DRIVER_NAME			"sdhci-zodiac"

static void sdhci_zodiac_dumpregs(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	pr_err("%s: Start dump maintenance and test register\n", DRIVER_NAME);
	pr_err("%s: tuning ctrl: 0x%08x\n", DRIVER_NAME,
	       sdhci_readl(host, SDHCI_CRG_CTRL0));
	pr_err("%s: 0x80=0x%08x 0x84=0x%08x 0x88=0x%08x\n", DRIVER_NAME,
	       sdhci_readl(host, 0x80), sdhci_readl(host, 0x84),
	       sdhci_readl(host, 0x88));
	pr_err("%s: 0x90=0x%08x 0x94=0x%08x 0x98=0x%08x\n", DRIVER_NAME,
	       sdhci_readl(host, 0x90), sdhci_readl(host, 0x94),
	       sdhci_readl(host, 0x98));

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE)
		sdhci_zodiac_dump_phy_regs(host);
}

static void sdhci_zodiac_hardware_reset(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE) {
		hisi_subctrl_emmc_reset_assert(zodiac_host->dev,
					       zodiac_host->host_id);
		hisi_subctrl_emmc_phy_reset_assert(zodiac_host->dev,
						   zodiac_host->host_id);
	} else {
		hisi_subctrl_sd_reset_assert(zodiac_host->dev,
					     zodiac_host->host_id);
	}
}

static int sdhci_zodiac_hardware_disreset(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	int ret;

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE) {
		hisi_subctrl_emmc_reset_deassert(zodiac_host->dev,
						 zodiac_host->host_id);
		ret = hisi_subctrl_emmc_get_reset_status(zodiac_host->dev,
							 zodiac_host->host_id);
		if (ret) {
			dev_err(zodiac_host->dev,
				"mmc is not deassert status, status=%d\n", ret);
			return ret;
		}

		hisi_subctrl_emmc_phy_reset_deassert(zodiac_host->dev,
						     zodiac_host->host_id);
		ret = hisi_subctrl_emmc_phy_get_reset_status(zodiac_host->dev,
							     zodiac_host->host_id);
		if (ret) {
			dev_err(zodiac_host->dev,
				"mmc phy is not deassert status, status=%d\n",
				ret);
			return ret;
		}
	} else {
		hisi_subctrl_sd_reset_deassert(zodiac_host->dev,
					       zodiac_host->host_id);
		ret = hisi_subctrl_sd_get_reset_status(zodiac_host->dev,
						       zodiac_host->host_id);
		if (ret) {
			dev_err(zodiac_host->dev,
				"sd is not deassert status, status=%d\n", ret);
			return ret;
		}
	}

	return 0;
}

static int sdhci_zodiac_icg_en(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE) {
		hisi_subctrl_emmc_clkgate_enable(zodiac_host->dev,
						 zodiac_host->host_id);
		udelay(100);
		if (!hisi_subctrl_emmc_clkgate_is_enabled(zodiac_host->dev,
							  zodiac_host->host_id)) {
			dev_err(zodiac_host->dev, "mmc clkgate not enabled\n");
			return -EIO;
		}

		hisi_subctrl_emmc_phy_clkgate_enable(zodiac_host->dev,
						     zodiac_host->host_id);
		udelay(100);
		if (!hisi_subctrl_emmc_phy_clkgate_is_enabled(zodiac_host->dev,
							      zodiac_host->host_id)) {
			dev_err(zodiac_host->dev,
				"mmc phy clkgate not enabled\n");
			return -EIO;
		}
	} else {
		hisi_subctrl_sd_clkgate_enable(zodiac_host->dev,
					       zodiac_host->host_id);
		udelay(100);
		if (!hisi_subctrl_sd_clkgate_is_enabled(zodiac_host->dev,
							zodiac_host->host_id)) {
			dev_err(zodiac_host->dev, "sd clkgate not enabled\n");
			return -EIO;
		}
	}

	return 0;
}

static void sdhci_zodiac_icg_dis(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE) {
		hisi_subctrl_emmc_clkgate_disable(zodiac_host->dev,
						  zodiac_host->host_id);
		hisi_subctrl_emmc_phy_clkgate_disable(zodiac_host->dev,
						      zodiac_host->host_id);
	} else {
		hisi_subctrl_sd_clkgate_disable(zodiac_host->dev,
						zodiac_host->host_id);
	}
}

static unsigned int sdhci_zodiac_get_min_clock(struct sdhci_host *host)
{
	return SDHCI_ZODIAC_MIN_FREQ;
}

static void sdhci_zodiac_disable_clk(struct sdhci_host *host)
{
	u16 reg;
	ktime_t timeout;

	reg = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	reg &= ~SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, reg, SDHCI_CLOCK_CONTROL);
	reg &= ~SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, reg, SDHCI_CLOCK_CONTROL);

	timeout = ktime_add_ms(ktime_get(), SDHCI_ZODIAC_CLK_STABLE_TIME);
	for (;;) {
		reg = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
		if (!(reg & SDHCI_CLOCK_INT_STABLE))
			break;
		if (ktime_after(ktime_get(), timeout)) {
			pr_err("%s: Internal clock never disabled cleanly\n",
			       mmc_hostname(host->mmc));
			sdhci_zodiac_dumpregs(host);
			return;
		}
		udelay(10);
	}
}

static void sdhci_zodiac_enable_clk(struct sdhci_host *host)
{
	u16 reg;
	ktime_t timeout;

	reg = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
	reg |= SDHCI_CLOCK_INT_EN;
	sdhci_writew(host, reg, SDHCI_CLOCK_CONTROL);

	timeout = ktime_add_ms(ktime_get(), SDHCI_ZODIAC_CLK_STABLE_TIME);
	for (;;) {
		reg = sdhci_readw(host, SDHCI_CLOCK_CONTROL);
		if (reg & SDHCI_CLOCK_INT_STABLE)
			break;
		if (ktime_after(ktime_get(), timeout)) {
			pr_err("%s: Internal clock never stabilised\n",
			       mmc_hostname(host->mmc));
			sdhci_zodiac_dumpregs(host);
			return;
		}
		udelay(10);
	}

	reg |= SDHCI_CLOCK_CARD_EN;
	sdhci_writew(host, reg, SDHCI_CLOCK_CONTROL);
}

static void sdhci_zodiac_set_clk_sel(struct sdhci_host *host, u8 timing)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE) {
		if (timing == MMC_TIMING_LEGACY)
			hisi_subctrl_emmc_set_clk_cfg(zodiac_host->dev,
						      zodiac_host->host_id,
						      L_CLK, 0);
		else
			hisi_subctrl_emmc_set_clk_cfg(zodiac_host->dev,
						      zodiac_host->host_id,
						      H_CLK, 0);

		if (timing == MMC_TIMING_MMC_HS400 ||
		    timing == MMC_TIMING_MMC_HS200)
			hisi_subctrl_emmc_set_sample_clk(zodiac_host->dev,
							 zodiac_host->host_id,
							 TUNED_CLK);
		else
			hisi_subctrl_emmc_set_sample_clk(zodiac_host->dev,
							 zodiac_host->host_id,
							 FIXED_CLK);
	} else {
		if (timing == MMC_TIMING_LEGACY)
			hisi_subctrl_sd_set_clk_cfg(zodiac_host->dev,
						    zodiac_host->host_id,
						    L_CLK, 0);
		else
			hisi_subctrl_sd_set_clk_cfg(zodiac_host->dev,
						    zodiac_host->host_id,
						    H_CLK, 0);
	}
}

static void sdhci_zodiac_timing_cfg(struct sdhci_host *host, u8 timing)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	struct sdhci_clk_cfg *crg = &zodiac_host->clk_cfg[timing];
	struct sdhci_timing_cfg *timing_cfg = &crg->timing_cfg;
	u32 val;

	val = sdhci_readl(host, SDEMMC_TIMING_CTRL_0);
	val &= ~(CFG_READ_WAIT_START | CFG_READ_AUTO_CMD12_START |
		 CFG_CMD_TIMEOUT_CNT | CFG_CMD_RESP_CNT);
	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE) {
		val &= ~OE_MASK_EN;
		if (timing_cfg->oe_mask_en == 1)
			val |= OE_MASK_EN;
	}
	val |= FIELD_PREP(CFG_READ_WAIT_START, timing_cfg->read_wait_start);
	val |= FIELD_PREP(CFG_READ_AUTO_CMD12_START,
			  timing_cfg->read_auto_cmd12_start);
	val |= FIELD_PREP(CFG_CMD_TIMEOUT_CNT, timing_cfg->cmd_timeout_cnt);
	val |= FIELD_PREP(CFG_CMD_RESP_CNT, timing_cfg->cmd_resp_cnt);
	sdhci_writel(host, val, SDEMMC_TIMING_CTRL_0);

	val = sdhci_readl(host, SDEMMC_TIMING_CTRL_1);
	val &= ~(CRC_ST_DET_DLY_MODE | CRC_ST_DET_DLY | CRC_ST_CHK_DLY);
	if (timing_cfg->crc_st_det_dly_mode == 1)
		val |= CRC_ST_DET_DLY_MODE;
	val |= FIELD_PREP(CRC_ST_DET_DLY, timing_cfg->crc_st_det_dly);
	val |= FIELD_PREP(CRC_ST_CHK_DLY, timing_cfg->crc_st_chk_dly);
	sdhci_writel(host, val, SDEMMC_TIMING_CTRL_1);

	val = sdhci_readl(host, SDEMMC_TIMING_CTRL_2);
	val &= ~CFG_STOP_SDCLK_RD_START;
	val |= FIELD_PREP(CFG_STOP_SDCLK_RD_START,
			  timing_cfg->stop_sdclk_rd_start);
	sdhci_writel(host, val, SDEMMC_TIMING_CTRL_2);
}

static void sdhci_zodiac_set_crg_timing_para(struct sdhci_host *host, u8 timing)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	struct sdhci_clk_cfg *cfg = &zodiac_host->clk_cfg[timing];
	u32 val;

	val = sdhci_readl(host, SDHCI_CRG_CTRL0);
	val &= ~(SDEMMC_SAMP_SEL | SDEMMC_CLK_DIV |
		 SDEMMC_CLK_DLY_DRV | SDEMMC_CLK_DLY_SAMPLE);
	if (cfg->sample_mode == ASYNC_MODE)
		val |= SDEMMC_SAMP_SEL;
	val |= FIELD_PREP(SDEMMC_CLK_DIV, cfg->clk_div);
	val |= FIELD_PREP(SDEMMC_CLK_DLY_DRV, cfg->clk_dly_drv);
	val |= FIELD_PREP(SDEMMC_CLK_DLY_SAMPLE, cfg->clk_dly_sample);
	sdhci_writel(host, val, SDHCI_CRG_CTRL0);

	host->max_clk = cfg->max_clk;
	if (timing == MMC_TIMING_LEGACY ||
	    timing == MMC_TIMING_UHS_SDR50 ||
	    timing == MMC_TIMING_UHS_SDR104 ||
	    timing == MMC_TIMING_MMC_HS200 ||
	    timing == MMC_TIMING_MMC_HS400 ||
	    timing == MMC_TIMING_MMC_DDR52)
		sdhci_zodiac_timing_cfg(host, timing);
}

static void sdhci_zodiac_set_card_power_en(struct sdhci_host *host, u32 en)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE)
		hisi_subctrl_emmc_set_power_en(zodiac_host->dev,
					       zodiac_host->host_id, en);
	else
		hisi_subctrl_sd_set_power_en(zodiac_host->dev,
					     zodiac_host->host_id, en);
}

static int sdhci_zodiac_reset_controller(struct sdhci_host *host)
{
	int ret;

	sdhci_zodiac_icg_dis(host);
	sdhci_zodiac_hardware_reset(host);

	ret = sdhci_zodiac_icg_en(host);
	if (ret)
		return ret;

	udelay(100);
	sdhci_zodiac_icg_dis(host);

	ret = sdhci_zodiac_hardware_disreset(host);
	if (ret)
		return ret;

	return sdhci_zodiac_icg_en(host);
}

static int sdhci_zodiac_init_cfg(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	int ret;

	ret = sdhci_zodiac_reset_controller(host);
	if (ret)
		return ret;

	sdhci_zodiac_set_card_power_en(host, 1);
	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE)
		return sdhci_combo_phy_init(host);

	return 0;
}

static void sdhci_zodiac_select_clk_src(struct sdhci_host *host,
					struct mmc_ios *ios)
{
	int ret;

	if (ios->timing > MMC_TIMING_MMC_HS400) {
		pr_err("sdhci_zodiac_select_clk_src failed, timing %d exceed max\n",
		       ios->timing);
		return;
	}

	sdhci_zodiac_icg_dis(host);
	sdhci_zodiac_set_clk_sel(host, ios->timing);
	ret = sdhci_zodiac_icg_en(host);
	if (ret)
		return;

	sdhci_zodiac_set_crg_timing_para(host, ios->timing);
}

static int sdhci_zodiac_phy_delay_measurement(struct sdhci_host *host,
					      struct mmc_ios *ios)
{
	if (host->clock == MMC_HS200_MAX_DTR ||
	    host->clock == MMC_HIGH_DDR_MAX_DTR ||
	    host->clock == host->mmc->f_max)
		return sdhci_combo_phy_delay_measurement(host, ios);

	return 0;
}

static void sdhci_zodiac_set_clock(struct sdhci_host *host, unsigned int clock)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	u16 clk;

	host->mmc->actual_clock = 0;
	sdhci_zodiac_disable_clk(host);
	if (clock == 0)
		return;

	sdhci_zodiac_select_clk_src(host, &host->mmc->ios);
	clk = sdhci_calc_clk(host, clock, &host->mmc->actual_clock);
	sdhci_writew(host, clk, SDHCI_CLOCK_CONTROL);
	sdhci_enable_clk(host, clk);

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE)
		sdhci_zodiac_phy_delay_measurement(host, &host->mmc->ios);
}

static void sdhci_zodiac_hw_reset(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	int ret;

	sdhci_zodiac_hardware_reset(host);
	ret = sdhci_zodiac_hardware_disreset(host);
	if (ret) {
		dev_err(zodiac_host->dev,
			"sdhci_zodiac_hw_reset disreset failed\n");
		return;
	}

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE)
		sdhci_combo_phy_init(host);
}

static int sdhci_zodiac_enable_dma(struct sdhci_host *host)
{
	u8 val;

	if (host->runtime_suspended)
		return 0;

	val = sdhci_readb(host, SDHCI_HOST_CONTROL);
	val &= ~SDHCI_CTRL_DMA_MASK;
	if (host->flags & SDHCI_USE_ADMA) {
		if (host->flags & SDHCI_USE_64_BIT_DMA)
			val |= SDHCI_CTRL_ADMA64;
		else
			val |= SDHCI_CTRL_ADMA32;
	} else {
		val |= SDHCI_CTRL_SDMA;
	}
	sdhci_writeb(host, val, SDHCI_HOST_CONTROL);

	return 0;
}

static int sdhci_zodiac_execute_hwtuning(struct sdhci_host *host, u32 opcode)
{
	int i;
	int err = -EIO;
	u16 val;

	for (i = 0; i < ZODIAC_MAX_TUNING_LOOP; i++) {
		sdhci_zodiac_disable_clk(host);
		sdhci_zodiac_enable_clk(host);
		sdhci_send_tuning(host, opcode);

		if (!host->tuning_done) {
			pr_err("%s: Tuning timeout, falling back to fixed sampling clock\n",
			       mmc_hostname(host->mmc));
			sdhci_abort_tuning(host, opcode);
			return -ETIMEDOUT;
		}

		val = sdhci_readw(host, SDHCI_HOST_CONTROL2);
		if (!(val & SDHCI_CTRL_EXEC_TUNING)) {
			if (val & SDHCI_CTRL_TUNED_CLK)
				return 0;

			pr_err("%s: Tuning failed, falling back to fixed sampling clock\n",
			       mmc_hostname(host->mmc));
			sdhci_reset_tuning(host);
			return -EAGAIN;
		}

		if (host->tuning_delay > 0)
			mdelay(host->tuning_delay);
	}

	return err;
}

static int sdhci_zodiac_platform_execute_tuning(struct sdhci_host *host,
						u32 opcode)
{
	int err;

	if (host->tuning_delay < 0)
		host->tuning_delay = (opcode == MMC_SEND_TUNING_BLOCK) ? 1 : 0;

	sdhci_start_tuning(host);
	err = sdhci_zodiac_execute_hwtuning(host, opcode);
	sdhci_end_tuning(host);
	sdhci_zodiac_disable_clk(host);
	sdhci_zodiac_enable_clk(host);

	if (err) {
		sdhci_zodiac_dumpregs(host);
		pr_err("hw tuning fail: %d\n", err);
		return err;
	}

	pr_info("hw tuning success, best_phase=%u\n",
		sdhci_readl(host, TUNING_RESULT_VALUE_REG));
	return 0;
}

static void sdhci_zodiac_power_off(struct sdhci_host *host)
{
	sdhci_writeb(host, 0, SDHCI_POWER_CONTROL);
	sdhci_zodiac_set_card_power_en(host, 0);
}

static void sdhci_zodiac_sd_sel_1v8(struct sdhci_host *host, u32 set)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	hisi_subctrl_sd_volt_switch_1v8(zodiac_host->dev, zodiac_host->host_id,
					set);
}

static void sdhci_zodiac_power_up(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	if (zodiac_host->host_mode == ZODIAC_SD_MODE)
		sdhci_zodiac_sd_sel_1v8(host, 0);
	else if (zodiac_host->host_mode == ZODIAC_SDIO_MODE)
		sdhci_zodiac_sd_sel_1v8(host, 1);

	sdhci_writeb(host, SDHCI_POWER_ON, SDHCI_POWER_CONTROL);
	sdhci_zodiac_set_card_power_en(host, 1);
}

static void sdhci_zodiac_set_power(struct sdhci_host *host,
				   unsigned char mode, unsigned short vdd)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	if (zodiac_host->power_mode == mode)
		return;

	switch (mode) {
	case MMC_POWER_OFF:
		sdhci_zodiac_power_off(host);
		break;
	case MMC_POWER_UP:
		sdhci_zodiac_power_up(host);
		break;
	case MMC_POWER_ON:
		break;
	default:
		dev_err(zodiac_host->dev, "unknown power supply mode %u\n", mode);
		break;
	}

	zodiac_host->power_mode = mode;
}

static void sdhci_zodiac_voltage_switch(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);

	if (zodiac_host->host_mode == ZODIAC_SD_MODE)
		sdhci_zodiac_sd_sel_1v8(host, 1);
}

static unsigned int sdhci_zodiac_get_max_clock(struct sdhci_host *host)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);

	return pltfm_host->clock;
}

static struct sdhci_ops sdhci_zodiac_ops = {
	.get_min_clock = sdhci_zodiac_get_min_clock,
	.set_clock = sdhci_zodiac_set_clock,
	.enable_dma = sdhci_zodiac_enable_dma,
	.get_max_clock = sdhci_zodiac_get_max_clock,
	.set_bus_width = sdhci_set_bus_width,
	.reset = sdhci_reset,
	.set_uhs_signaling = sdhci_set_uhs_signaling,
	.platform_execute_tuning = sdhci_zodiac_platform_execute_tuning,
	.hw_reset = sdhci_zodiac_hw_reset,
	.set_power = sdhci_zodiac_set_power,
	.dump_vendor_regs = sdhci_zodiac_dumpregs,
	.voltage_switch = sdhci_zodiac_voltage_switch,
};

static struct sdhci_pltfm_data sdhci_zodiac_pdata = {
	.ops = &sdhci_zodiac_ops,
	.quirks = SDHCI_QUIRK_CAP_CLOCK_BASE_BROKEN |
		  SDHCI_QUIRK_BROKEN_TIMEOUT_VAL |
		  SDHCI_QUIRK_INVERTED_WRITE_PROTECT |
		  SDHCI_QUIRK_MULTIBLOCK_READ_ACMD12,
	.quirks2 = SDHCI_QUIRK2_PRESET_VALUE_BROKEN,
};

#ifdef CONFIG_PM_SLEEP
static int sdhci_zodiac_suspend(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	int ret;

	ret = sdhci_suspend_host(host);
	if (ret)
		return ret;

	sdhci_zodiac_set_card_power_en(host, 0);
	sdhci_zodiac_disable_clk(host);
	sdhci_zodiac_icg_dis(host);
	sdhci_zodiac_hardware_reset(host);

	return 0;
}

static int sdhci_zodiac_resume(struct device *dev)
{
	struct sdhci_host *host = dev_get_drvdata(dev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	int ret;

	ret = sdhci_zodiac_hardware_disreset(host);
	if (ret)
		return ret;

	ret = sdhci_zodiac_icg_en(host);
	if (ret)
		return ret;

	sdhci_zodiac_enable_clk(host);
	sdhci_zodiac_set_card_power_en(host, 1);

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE) {
		ret = sdhci_combo_phy_init(host);
		if (ret)
			return ret;
	}

	return sdhci_resume_host(host);
}

static const struct dev_pm_ops sdhci_zodiac_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(sdhci_zodiac_suspend, sdhci_zodiac_resume)
};
#endif

static int sdhci_zodiac_get_phy_resource(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	struct resource *res;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "phy");
	zodiac_host->phy.io_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(zodiac_host->phy.io_base)) {
		dev_err(&pdev->dev, "devm_ioremap phy iobase failed\n");
		return PTR_ERR(zodiac_host->phy.io_base);
	}

	zodiac_host->phy.io_size = resource_size(res);
	return 0;
}

static int sdhci_zodiac_parse_clk_cfg(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	struct device *dev = &pdev->dev;
	int ret;
	int i;
	char prop_name[32];

	for (i = 0; i < HISI_MMC_MAX_TIMING_MODE; i++) {
		scnprintf(prop_name, sizeof(prop_name), "timing_%d_cfg", i);
		ret = device_property_read_u32_array(
			dev, prop_name, (u32 *)&zodiac_host->clk_cfg[i],
			sizeof(zodiac_host->clk_cfg[i]) / sizeof(u32));
		if (ret) {
			dev_err(dev, "problem parsing %s property: %d\n",
				prop_name, ret);
			return ret;
		}
	}

	return 0;
}

static void sdhci_zodiac_parse_delay1_cfg(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	struct device *dev = &pdev->dev;

	if (device_property_read_u32_array(
		    dev, "mmc_ddr52_tx_delay",
		    (u32 *)&zodiac_host->delay1_cfg[ZODIAC_EMMC_DDR52],
		    sizeof(struct zodiac_emmc_delay_config) / sizeof(u32)))
		dev_info(dev, "using default tx delay for ddr52\n");

	if (device_property_read_u32_array(
		    dev, "mmc_hs200_tx_delay",
		    (u32 *)&zodiac_host->delay1_cfg[ZODIAC_EMMC_HS200],
		    sizeof(struct zodiac_emmc_delay_config) / sizeof(u32)))
		dev_info(dev, "using default tx delay for hs200\n");

	if (device_property_read_u32_array(
		    dev, "mmc_hs400_tx_delay",
		    (u32 *)&zodiac_host->delay1_cfg[ZODIAC_EMMC_HS400],
		    sizeof(struct zodiac_emmc_delay_config) / sizeof(u32)))
		dev_info(dev, "using default tx delay for hs400\n");
}

static int sdhci_zodiac_get_property(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_zodiac_host *zodiac_host = sdhci_pltfm_priv(pltfm_host);
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	int ret;

	if (of_device_is_compatible(np, "hisilicon,sdhci_emmc"))
		zodiac_host->host_mode = ZODIAC_EMMC_MODE;
	if (of_device_is_compatible(np, "hisilicon,sdhci_sd"))
		zodiac_host->host_mode = ZODIAC_SD_MODE;
	if (of_device_is_compatible(np, "hisilicon,sdhci_sdio"))
		zodiac_host->host_mode = ZODIAC_SDIO_MODE;
	if (of_device_is_compatible(np, "hisilicon,sdhci_emmc_sd")) {
		ret = device_property_read_u32(dev, "host_mode",
					       &zodiac_host->host_mode);
		if (ret) {
			dev_err(dev, "failed to get sdhci host_mode: %d\n", ret);
			return ret;
		}
	}

	if (zodiac_host->host_mode > ZODIAC_SDIO_MODE) {
		dev_err(dev, "host_mode %u exceeds max\n",
			zodiac_host->host_mode);
		return -EINVAL;
	}

	ret = device_property_read_u32(dev, "host_id", &zodiac_host->host_id);
	if (ret) {
		dev_err(dev, "failed to get sdhci host_id: %d\n", ret);
		return ret;
	}

	if (device_property_read_bool(dev, "broken_hs200"))
		host->quirks2 |= SDHCI_QUIRK2_BROKEN_HS200;

	ret = sdhci_zodiac_parse_clk_cfg(pdev);
	if (ret)
		return ret;

	if (zodiac_host->host_mode == ZODIAC_EMMC_MODE) {
		sdhci_zodiac_parse_delay1_cfg(pdev);
		ret = sdhci_zodiac_get_phy_resource(pdev);
		if (ret)
			return ret;
	}

	return 0;
}

static int sdhci_zodiac_parse_dts(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);
	int ret;

	ret = mmc_of_parse(host->mmc);
	if (ret) {
		dev_err(&pdev->dev, "mmc_of_parse failed\n");
		return ret;
	}

	sdhci_get_property(pdev);
	return sdhci_zodiac_get_property(pdev);
}

static void sdhci_zodiac_host_quirks_cfg(struct sdhci_host *host)
{
	host->quirks2 |= SDHCI_QUIRK2_HOST_NO_CMD23;
	host->quirks |= SDHCI_QUIRK_NO_ENDATTR_IN_NOPDESC;
	host->mmc->caps2 |= MMC_CAP2_NO_PRESCAN_POWERUP;

	if (host->mmc->pm_caps & MMC_PM_KEEP_POWER) {
		host->mmc->pm_flags |= MMC_PM_KEEP_POWER;
		host->quirks2 |= SDHCI_QUIRK2_HOST_OFF_CARD_ON;
	}

	if (!(host->quirks2 & SDHCI_QUIRK2_BROKEN_64_BIT_DMA))
		host->dma_mask = DMA_BIT_MASK(64);
	else
		host->dma_mask = DMA_BIT_MASK(32);
	mmc_dev(host->mmc)->dma_mask = &host->dma_mask;
}

static void sdhci_zodiac_set_desc_size(struct sdhci_host *host)
{
	host->caps = sdhci_readl(host, SDHCI_CAPABILITIES);
	if (host->caps & SDHCI_CAN_64BIT)
		host->alloc_desc_sz = 16;
}

static int sdhci_zodiac_probe(struct platform_device *pdev)
{
	struct sdhci_host *host;
	struct sdhci_pltfm_host *pltfm_host;
	struct sdhci_zodiac_host *zodiac_host;
	int ret;

	dev_info(&pdev->dev, "sdhci_zodiac_probe start\n");

	host = sdhci_pltfm_init(pdev, &sdhci_zodiac_pdata, sizeof(*zodiac_host));
	if (IS_ERR(host))
		return PTR_ERR(host);

	pltfm_host = sdhci_priv(host);
	zodiac_host = sdhci_pltfm_priv(pltfm_host);
	zodiac_host->dev = &pdev->dev;
	zodiac_host->host = host;

	ret = sdhci_zodiac_parse_dts(pdev);
	if (ret)
		return ret;

	ret = hisi_mmc_subctrl_init(zodiac_host->host_mode, zodiac_host->host_id,
				    zodiac_host->dev);
	if (ret)
		return ret;

	ret = sdhci_zodiac_get_clk_rst_info(zodiac_host);
	if (ret)
		return ret;

	ret = sdhci_zodiac_init_cfg(host);
	if (ret)
		return ret;

	sdhci_zodiac_host_quirks_cfg(host);
	sdhci_zodiac_set_desc_size(host);

	ret = sdhci_add_host(host);
	if (ret)
		return ret;

	dev_info(&pdev->dev, "%s: sdhci_zodiac_probe done\n",
		 dev_name(zodiac_host->dev));
	return 0;
}

static void sdhci_zodiac_remove(struct platform_device *pdev)
{
	struct sdhci_host *host = platform_get_drvdata(pdev);

	sdhci_remove_host(host, 1);
	sdhci_zodiac_icg_dis(host);
	sdhci_zodiac_set_card_power_en(host, 0);
}

static const struct of_device_id sdhci_zodiac_of_match[] = {
	{ .compatible = "hisilicon,sdhci_emmc_sd" },
	{ .compatible = "hisilicon,sdhci_emmc" },
	{ .compatible = "hisilicon,sdhci_sd" },
	{ .compatible = "hisilicon,sdhci_sdio" },
	{ }
};
MODULE_DEVICE_TABLE(of, sdhci_zodiac_of_match);

static struct platform_driver sdhci_zodiac_driver = {
	.driver = {
		.name = "sdhci_zodiac",
		.of_match_table = sdhci_zodiac_of_match,
#ifdef CONFIG_PM_SLEEP
		.pm = &sdhci_zodiac_dev_pm_ops,
#endif
	},
	.probe = sdhci_zodiac_probe,
	.remove = sdhci_zodiac_remove,
};
module_platform_driver(sdhci_zodiac_driver);

MODULE_DESCRIPTION("Driver for the Zodiac SDHCI Controller");
MODULE_AUTHOR("OpenAI");
MODULE_LICENSE("GPL");
