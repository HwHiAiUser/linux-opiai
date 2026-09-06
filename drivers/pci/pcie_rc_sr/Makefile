# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_PCIE_RC_SR) += drv_pcierc_sr.o
drv_pcierc_sr-objs := pcie_rc_drv.o drv_snapshot.o pcie_qos_common.o \
	pcie_ras_config.o pcie_rc_qos.o soc/mini_v3/pcie_rc_ctrl.o

ccflags-y += -I$(src) -I$(src)/soc/mini_v3
ccflags-y += -I$(src)/include/kernel -I$(src)/include/driver
ccflags-y += -I$(src)/include/internal
ccflags-y += -DCFG_ENV_PCIE_RC_SR -DNOT_SUPPORT_OSNAME
ccflags-y += -include $(src)/include/compat.h
ccflags-y += -Wno-unused-parameter -Wno-sign-compare -Wno-missing-field-initializers
