/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#include <linux/of_address.h>
#include <linux/of_pci.h>
#include <linux/pci-ecam.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/securec.h>

#include "pcie_rc_qos.h"
#include "pcie_ras_config.h"
#include "pcie_rc_ctrl.h"
#include "pcie_rc_drv.h"

#ifdef CFG_FEATURE_SERDES_H25
#include "sdk_hilink_pub.h"
#endif

#ifndef PCIE_RC_UT
#include <linux/kthread.h>
#define WAIT_SMMU_RESUMED_TIME 1

#define PCI_ECAM_MAP_BUS_FN_NAME  "pci_ecam_map_bus"
typedef void __iomem *(*pci_ecam_map_bus_fn)(struct pci_bus *bus, unsigned int devfn, int where);
STATIC pci_ecam_map_bus_fn g_pci_ecam_map_bus_fn = NULL;

#define PCI_HOST_COMMON_PROBE_FN_NAME  "pci_host_common_probe"
typedef int (*pci_host_common_probe_fn)(struct platform_device *pdev, struct pci_ecam_ops *ops);
STATIC pci_host_common_probe_fn g_pci_host_common_probe_fn = NULL;

#define PCI_HOST_COMMON_REMOVE_FN_NAME  "pci_host_common_remove"
typedef int (*pci_host_common_remove_fn)(struct platform_device *pdev);
STATIC pci_host_common_remove_fn g_pci_host_common_remove_fn = NULL;

#define DEVDRV_RESUME_DEVICE_FN_NAME "devdrv_resume_device"
typedef void (*devdrv_resume_device_fn)(void);
STATIC devdrv_resume_device_fn g_devdrv_resume_device_fn = NULL;

STATIC struct task_struct *g_rc_thread = NULL;

int g_rc_type;

STATIC char *type = "normal";
module_param(type, charp, S_IRUGO);

struct mutex g_snapshot_lock;
STATIC u32 g_active_dev_num = 0;

enum pcie_link_up_status {
    PCIE_EP_LINKUP_DOWN = 0,
    PCIE_EP_LINKUP_NORMAL = 1,
    PCIE_EP_LINKUP_REMOVE = 2
};
atomic_t g_link_up_status;
/* do not need to wait smmu driver resume in the startup phase */
static u32 g_smmu_resumed = 1;

struct mutex g_ctrl_lock;
struct pcie_core_ctrl g_rc_ctrls[PCIE_CORE_NUM];

int pcie_rc_get_type(void)
{
    return g_rc_type;
}

void __iomem *pcie_rc_get_apb_base(u32 core_id)
{
    return g_rc_ctrls[core_id].apb_base;
}

void pcie_rc_ctrl_lock(void)
{
    mutex_lock(&g_ctrl_lock);
}

void pcie_rc_ctrl_unlock(void)
{
    mutex_unlock(&g_ctrl_lock);
}

u32 pcie_rc_get_active_dev_num(void)
{
    return g_active_dev_num;
}

STATIC void __iomem *drv_pcirc_ecam_map_bus(struct pci_bus *bus, unsigned int devfn, int where)
{
    return g_pci_ecam_map_bus_fn(bus, devfn, where);
}

STATIC int drv_pcirc_config_read(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 *val)
{
#ifdef CFG_FEATURE_SERDES_H25
    if (hilink_is_ready() == 0) {
        WARN_ONCE(1, "serdes not ready.\n");
        return -ENOLINK;
    }
#endif
    return pci_generic_config_read(bus, devfn, where, size, val);
}

STATIC int drv_pcirc_config_write(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 val)
{
#ifdef CFG_FEATURE_SERDES_H25
    if (hilink_is_ready() == 0) {
        WARN_ONCE(1, "serdes not ready.\n");
        return -ENOLINK;
    }
#endif
    return pci_generic_config_write(bus, devfn, where, size, val);
}

int drv_pcierc_apb_init(u32 core_id)
{
    g_rc_ctrls[core_id].core_id = core_id;

    if (g_rc_ctrls[core_id].apb_base == NULL) {
        g_rc_ctrls[core_id].apb_base = ioremap(drv_pcierc_get_reg_base(core_id), (size_t)PCIE_REG_SIZE);
        if (g_rc_ctrls[core_id].apb_base == NULL) {
            return -ENOMEM;
        }
    }
    return 0;
}

void drv_pcierc_apb_uninit(u32 core_id)
{
    if (g_rc_ctrls[core_id].apb_base != NULL) {
        iounmap(g_rc_ctrls[core_id].apb_base);
        g_rc_ctrls[core_id].apb_base = NULL;
    }
}

/* ECAM ops */
STATIC struct pci_ecam_ops drv_pcierc_ecam_ops = {
    .bus_shift  = BUS_SHIFT,
    .pci_ops    = {
        .map_bus    = drv_pcirc_ecam_map_bus,
        .read       = drv_pcirc_config_read,
        .write      = drv_pcirc_config_write,
    }
};

STATIC const struct of_device_id gen_pci_of_match[] = {
    { .compatible = "ascend-pcie-rc",
      .data = &drv_pcierc_ecam_ops },
    { },
};

STATIC int drv_pcierc_enum_device(struct platform_device *pdev)
{
    const struct of_device_id *of_id;
    struct pci_ecam_ops *ops;
    int ret;

    of_id = of_match_node(gen_pci_of_match, pdev->dev.of_node);
    if (of_id == NULL) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_STARTUP_MATCH_NODE_FAIL);
        drv_pcie_rc_err("of_match_node is null.\n");
        return -EINVAL;
    }

    ops = (struct pci_ecam_ops *)of_id->data;
    ret = g_pci_host_common_probe_fn(pdev, ops);
    if (ret != 0) {
        drv_pcie_rc_err("Failed to call pci_host_common_probe. (ret=%d)\n", ret);
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_STARTUP_COMMON_PROBE_FAIL);
    }

    return ret;
}

static int drv_pcierc_resume_device(void *data)
{
    struct platform_device *pdev = (struct platform_device *)data;
    u32 port_id = 0;
    u32 core_id;
    int ret;
    bool self_heal_flag = false;
    int dev_id = 0;

    ret = of_property_read_u32(pdev->dev.of_node, "pcie-core-id", &core_id);
    if ((ret < 0) || (core_id >= PCIE_CORE_NUM)) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_STARTUP_GET_CORE_ID_FAIL);
        drv_pcie_rc_err("Failed to get pcie-core-id. (ret=%d; core_id=%u)\n", ret, core_id);
        return -EINVAL;
    }

    while (1) {
        if (!IS_ERR_OR_NULL(g_rc_thread)) {
            if (kthread_should_stop()) {
                drv_pcie_rc_info("rc link up thread stoped. (core_id=%u)\n", core_id);
                break;
            }
        }
        if (atomic_read(&g_link_up_status) == (int)PCIE_EP_LINKUP_NORMAL) {
            msleep(READ_STATUS_DELAY_TIME);
            continue;
        }
        drv_pcierc_wait_link_up(core_id, port_id, &self_heal_flag);

        /* wait smmu driver resume */
        while (g_smmu_resumed != 1) {
            msleep(WAIT_SMMU_RESUMED_TIME);
        }

        if (g_active_dev_num == 0) {
            ret = drv_pcierc_enum_device(pdev);
            if (ret != 0) {
                drv_pcierc_apb_uninit(core_id);
                drv_pcie_rc_err("Failed to enum pcie device. (core_id=%u)\n", core_id);
                return -EINVAL;
            }
            if (self_heal_flag == true) {
                drv_pcierc_recover_qos_cfg(dev_id, core_id);
            }
            g_active_dev_num++;
            drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_STARTUP_EXPECT);
        } else {
            g_devdrv_resume_device_fn =
                (devdrv_resume_device_fn)(uintptr_t)__kallsyms_get(DEVDRV_RESUME_DEVICE_FN_NAME);
            if (g_devdrv_resume_device_fn == NULL) {
                drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_RESUME_DEVICE_FAIL);
                drv_pcie_rc_err("kallsyms_get for devdrv_resume_device fail.\n");
            } else {
                g_devdrv_resume_device_fn();
                __kallsyms_put(DEVDRV_RESUME_DEVICE_FN_NAME);
                drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_RESUME_EXPECT);
            }
        }
        atomic_set(&g_link_up_status, (int)PCIE_EP_LINKUP_NORMAL);
        self_heal_flag = false;
    }
    return 0;
}

STATIC int pcie_rc_enum_device(struct platform_device *pdev, u32 core_id)
{
    int ret;

    if (g_rc_type == RC_TYPE_NORMAL) {
        ret = drv_pcierc_enum_device(pdev);
        if (ret != 0) {
            drv_pcie_rc_err("Failed to enum pcie device. (core_id=%u)\n", core_id);
            return -EINVAL;
        }
        g_active_dev_num++;
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_STARTUP_EXPECT);
    } else {
        pcie_mask_fault_config(g_rc_ctrls[core_id].apb_base);
        g_rc_thread = kthread_run(drv_pcierc_resume_device, pdev, "rc_resume_device_thread");
        if (IS_ERR(g_rc_thread)) {
            drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_STARTUP_CREAT_THREAD_FAIL);
            drv_pcie_rc_err("Failed to create rc_thread\n");
            return -EINVAL;
        }
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_STARTUP_WAIT_LINK_UP);
    }

    return ret;
}

STATIC int drv_pcierc_probe(struct platform_device *pdev)
{
    u32 core_id;
    int ret;

    atomic_set(&g_link_up_status, PCIE_EP_LINKUP_DOWN);

    drv_snapshot_bootdot_init(PCIE_MODULE_ID, SNAPSHOT_STATUS_STARTUP, PCIERC_STARTUP_EXPECT);
    drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_STARTUP_BEGIN);

    ret = of_property_read_u32(pdev->dev.of_node, "pcie-core-id", &core_id);
    if ((ret < 0) || (core_id >= PCIE_CORE_NUM)) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_STARTUP_GET_CORE_ID_FAIL);
        drv_pcie_rc_err("Failed to get pcie-core-id. (ret=%d; core_id=%u)\n", ret, core_id);
        return -EINVAL;
    }

    drv_pcie_rc_info("PCIe rc probe start. (core_id=%u)\n", core_id);

    ret = drv_pcierc_apb_init(core_id);
    if (ret != 0) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_STARTUP_APB_INIT_FAIL);
        drv_pcie_rc_err("Function ioremap pcie base reg failed. (core=%u)\n", core_id);
        return ret;
    }

    ret = pcie_rc_enum_device(pdev, core_id);
    if (ret != 0) {
        drv_pcierc_apb_uninit(core_id);
        drv_pcie_rc_err("Failed to enum pcie device. (core_id=%u)\n", core_id);
        return -EINVAL;
    }

    (void)dev_set_name(&pdev->dev, "pcie_rc");
    drv_pcie_rc_info("PCIe rc probe end. (core_id=%u; active_dev_num=%u)\n", core_id, g_active_dev_num);

    return ret;
}

STATIC void drv_pcierc_remove(struct platform_device *pdev)
{
    int ret;

    drv_snapshot_bootdot_init(PCIE_MODULE_ID, SNAPSHOT_STATUS_REMOVE, PCIERC_REMOVE_EXPECT);
    drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_REMOVE_BEGIN);
    g_active_dev_num--;

    if (!IS_ERR_OR_NULL(g_rc_thread)) {
        (void)kthread_stop(g_rc_thread);
        g_rc_thread = NULL;
    }
    g_pci_host_common_remove_fn(pdev);
    ret = drv_pcierc_ctrl_uninit(pdev);
    if (ret != 0) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_REMOVE_UNINIT_CTRL_FAIL);
        drv_pcie_rc_err("Failed to uninit pcie controler.\n");

    }

    drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_REMOVE_EXPECT);

}

STATIC int drv_pcierc_pm_suspend(struct device *dev)
{
    struct platform_device *pdev = NULL;
    static u32 snapshot_cnt = 0;
    int ret;
    u32 core_id;

    mutex_lock(&g_snapshot_lock);
    snapshot_cnt = (snapshot_cnt == g_active_dev_num) ? 1 : snapshot_cnt + 1;
    if (snapshot_cnt == 1) {  /* Only the first controler need to init */
        drv_snapshot_bootdot_init(PCIE_MODULE_ID, SNAPSHOT_STATUS_SUSPEND, PCIERC_SUSPEND_EXPECT);
    }
    mutex_unlock(&g_snapshot_lock);
    drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_SUSPEND_BEGIN);

    drv_pcie_rc_info("PCIe RC suspend start.\n");
    pdev = container_of(dev, struct platform_device, dev);
    if (g_rc_type == RC_TYPE_MDC) {
        if (atomic_read(&g_link_up_status) == PCIE_EP_LINKUP_DOWN) {
            drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_SUSPEND_LINK_UP_FAIL);
            drv_pcie_rc_err("Failed to PCIe link up.\n");
            return EINVAL;
        }
    }

    ret = of_property_read_u32(pdev->dev.of_node, "pcie-core-id", &core_id);
    if ((ret < 0) || (core_id >= PCIE_CORE_NUM)) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_RESUME_GET_CORE_ID_FAIL);
        drv_pcie_rc_err("Failed to get pcie-core-id. (ret=%d; core_id=%u)\n", ret, core_id);
        return -EINVAL;
    }

    ret = pcie_rc_suspend_qos_config(core_id, pdev);
    if (ret != 0) {
        drv_pcie_rc_err("Failed to save pcie_rc qos controler.\n");
        return ret;
    }

    ret = drv_pcierc_ctrl_uninit(pdev);
    if (ret != 0) {
        drv_pcie_rc_err("Failed to uninit pcie controler.\n");
        return ret;
    }

    g_smmu_resumed = 0;

    drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_SUSPEND_EXPECT);
    drv_pcie_rc_info("PCIe RC suspend succ.\n");

    return 0;
}

STATIC int drv_pcierc_pm_resume(struct device *dev)
{
    struct platform_device *pdev = NULL;
    static u32 snapshot_cnt = 0;
    int ret;
    u32 core_id;

    drv_pcie_rc_info("PCIe RC resume start.\n");
    pdev = container_of(dev, struct platform_device, dev);

    mutex_lock(&g_snapshot_lock);
    snapshot_cnt = (snapshot_cnt == g_active_dev_num) ? 1 : snapshot_cnt + 1;
    if (snapshot_cnt == 1) {  /* Only the first controler need to init */
        drv_snapshot_bootdot_init(PCIE_MODULE_ID, SNAPSHOT_STATUS_RESUME, PCIERC_RESUME_EXPECT);
    }
    mutex_unlock(&g_snapshot_lock);
    drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_RESUME_BEGIN);

    ret = of_property_read_u32(pdev->dev.of_node, "pcie-core-id", &core_id);
    if ((ret < 0) || (core_id >= PCIE_CORE_NUM)) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_RESUME_GET_CORE_ID_FAIL);
        drv_pcie_rc_err("Failed to get pcie-core-id. (ret=%d; core_id=%u)\n", ret, core_id);
        return -EINVAL;
    }

    ret = drv_pcierc_apb_init(core_id);
    if (ret != 0) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_RESUME_APB_INIT_FAIL);
        drv_pcie_rc_err("Function ioremap pcie base reg failed. (core=%u)\n", core_id);
        return ret;
    }

    drv_pcierc_ctrl_init(core_id);

    if (g_rc_type == RC_TYPE_MDC) {
        drv_pcierc_resume_self_heal_handle();
    }

    ret = pcie_rc_resume_qos_config(core_id, pdev);
    if (ret != 0) {
        drv_pcie_rc_err("Failed to resume qos config.\n");
        drv_pcierc_apb_uninit(core_id);
        return ret;
    }
    if (g_rc_type == RC_TYPE_NORMAL) {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_RESUME_EXPECT);
    } else {
        drv_snapshot_bootdot_set(PCIE_MODULE_ID, PCIERC_RESUME_WAIT_LINK_UP);
    }

    atomic_set(&g_link_up_status, PCIE_EP_LINKUP_DOWN);

    drv_pcie_rc_info("PCIe RC resume succ.\n");

    return 0;
}

STATIC int drv_pcierc_device_resume(struct device *dev)
{
    int rc_type;
    rc_type = pcie_rc_get_type();
    if (rc_type == RC_TYPE_NORMAL) {
        return 0;
    }
    drv_pcie_rc_info("resume after smmu driver.\n");
    g_smmu_resumed = 1;
    return 0;
}

STATIC const struct dev_pm_ops drv_pcierc_pm_ops = {
    SET_NOIRQ_SYSTEM_SLEEP_PM_OPS(drv_pcierc_pm_suspend, drv_pcierc_pm_resume)
    SET_SYSTEM_SLEEP_PM_OPS(NULL, drv_pcierc_device_resume)
};

STATIC struct platform_driver drv_pcierc_driver = {
    .driver = {
        .name = "ascend-pci-rc-driver",
        .of_match_table = gen_pci_of_match,
        .suppress_bind_attrs = true,
        .pm = &drv_pcierc_pm_ops,
    },
    .probe = drv_pcierc_probe,
    .remove = drv_pcierc_remove,
};

STATIC void pcie_rc_pre_init(void)
{
    u32 i;

    mutex_init(&g_snapshot_lock);
    mutex_init(&g_ctrl_lock);
    for (i = 0; i < PCIE_CORE_NUM; i++) {
        g_rc_ctrls[i].apb_base = NULL;
    }

    return;
}

STATIC int __init drv_pcierc_module_init(void)
{
    int ret;

    if (strcmp(type, "mdc") == 0) {
        g_rc_type = RC_TYPE_MDC;
    } else {
        g_rc_type = RC_TYPE_NORMAL;
    }

    pcie_rc_pre_init();

    ret = pcie_rc_qos_init();
    if (ret != 0) {
        drv_pcie_rc_err("pcie rc qos init failed. (ret = %d)\n", ret);
        return -EINVAL;
    }

    if (g_pci_ecam_map_bus_fn == NULL) {
        g_pci_ecam_map_bus_fn =
            (pci_ecam_map_bus_fn)(uintptr_t)__kallsyms_lookup_name(PCI_ECAM_MAP_BUS_FN_NAME);
        if (g_pci_ecam_map_bus_fn == NULL) {
            drv_pcie_rc_err("kallsyms_lookup_name for pci_ecam_map_bus fail.\n");
            goto ERR;
        }
    }

    if (g_pci_host_common_probe_fn == NULL) {
        g_pci_host_common_probe_fn =
            (pci_host_common_probe_fn)(uintptr_t)__kallsyms_lookup_name(PCI_HOST_COMMON_PROBE_FN_NAME);
        if (g_pci_host_common_probe_fn == NULL) {
            drv_pcie_rc_err("kallsyms_lookup_name for pci_host_common_probe fail.\n");
            goto ERR;
        }
    }

    if (g_pci_host_common_remove_fn == NULL) {
        g_pci_host_common_remove_fn =
            (pci_host_common_remove_fn)(uintptr_t)__kallsyms_lookup_name(PCI_HOST_COMMON_REMOVE_FN_NAME);
        if (g_pci_host_common_remove_fn == NULL) {
            drv_pcie_rc_err("kallsyms_lookup_name for pci_host_common_remove fail.\n");
            goto ERR;
        }
    }

    ret = platform_driver_register(&drv_pcierc_driver);
    if (ret != 0) {
        drv_pcie_rc_err("register ascend pci rc driver fail, ret = %d\n", ret);
        goto ERR;
    }
    return 0;
ERR:
    pcie_rc_qos_uninit();
    return -EINVAL;
}

STATIC void __exit drv_pcierc_module_exit(void)
{
    platform_driver_unregister(&drv_pcierc_driver);
    pcie_rc_qos_uninit();
}

module_init(drv_pcierc_module_init);
module_exit(drv_pcierc_module_exit);

MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("Hisilicon pcie rc driver");
MODULE_LICENSE("GPL");

#else

int drv_pcierc_module_init(void)
{
    return 0;
}

#endif /* end of PCIE_RC_UT */
