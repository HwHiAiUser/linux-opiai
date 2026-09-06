/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Ascend Driver Kernel Compatibility Layer
 *
 * Provides compatibility macros/wrappers for porting drivers originally
 * written for Linux 4.19/5.10 to Linux 6.18.
 *
 * Include this header at the top of source files that use deprecated APIs.
 * This file will be incrementally expanded as more modules are migrated.
 */
#ifndef __ASCEND_COMPAT_H__
#define __ASCEND_COMPAT_H__

#include <linux/version.h>

/* ========================================================================
 * 1. Memory mapping
 * ======================================================================== */

/* ioremap_nocache() removed in 5.6, use ioremap() which is always uncached */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#ifndef ioremap_nocache
#define ioremap_nocache ioremap
#endif
#endif

/* ========================================================================
 * 2. Access checking
 * ======================================================================== */

/*
 * access_ok() lost the 'type' parameter in 5.0
 * Old: access_ok(VERIFY_WRITE, addr, size)
 * New: access_ok(addr, size)
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#define compat_access_ok(type, addr, size) access_ok((addr), (size))
#else
#define compat_access_ok(type, addr, size) access_ok((type), (addr), (size))
#endif

/* ========================================================================
 * 3. Timer API
 * ======================================================================== */

/*
 * setup_timer() removed in 4.15+, replaced by timer_setup()
 * Old: setup_timer(&timer, callback, data)  -- callback(unsigned long)
 * New: timer_setup(&timer, callback, flags) -- callback(struct timer_list *)
 *
 * Migration requires changing callback signature. compat.h cannot auto-wrap
 * this — each module must be manually converted. This note is a reminder.
 */

/* ========================================================================
 * 4. Proc filesystem
 * ======================================================================== */

/*
 * proc_create() uses struct proc_ops instead of struct file_operations
 * since Linux 5.6. Provide a compat wrapper.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#include <linux/proc_fs.h>

/* PDE_DATA renamed to pde_data in 5.17 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
#ifndef PDE_DATA
#define PDE_DATA(inode) pde_data(inode)
#endif
#endif

#endif /* >= 5.6 */

/* ========================================================================
 * 5. DMA / PCI
 * ======================================================================== */

/* pci_set_dma_mask / pci_set_consistent_dma_mask removed in 5.18 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 18, 0)
#include <linux/dma-mapping.h>
#ifndef pci_set_dma_mask
#define pci_set_dma_mask(pdev, mask) dma_set_mask(&(pdev)->dev, (mask))
#endif
#ifndef pci_set_consistent_dma_mask
#define pci_set_consistent_dma_mask(pdev, mask) dma_set_coherent_mask(&(pdev)->dev, (mask))
#endif
#endif

/* pci_enable_msix() removed in 4.12, use pci_alloc_irq_vectors() */
/* Manual migration required per module */

/* ========================================================================
 * 6. Signal / scheduler
 * ======================================================================== */

/* No major compat issues, but note:
 * - wait_event_interruptible() behavior unchanged
 * - signal_pending() unchanged
 */

/* freezer_do_not_count()/freezer_count() were dropped from newer kernels.
 * Older driver code only uses them as freezer accounting hints around waits. */
#include <linux/freezer.h>
#ifndef freezer_do_not_count
#define freezer_do_not_count() do {} while (0)
#endif
#ifndef freezer_count
#define freezer_count() do {} while (0)
#endif

/* wakeup_source_create/add/remove/destroy are now internal helpers.
 * Older drivers still use the create/add/remove/destroy sequence directly. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#define wakeup_source_create(name) wakeup_source_register(NULL, (name))
#define wakeup_source_add(ws) do {} while (0)
#define wakeup_source_remove(ws) do {} while (0)
#define wakeup_source_destroy(ws) wakeup_source_unregister((ws))
#endif

/* ========================================================================
 * 7. IDR / IDA
 * ======================================================================== */

/* idr_remove() return type changed to void in 4.19+ (already void in our baseline) */

/* ========================================================================
 * 8. Sysfs / Kobject
 * ======================================================================== */

/* bus_find_device_by_name() unchanged */
/* class_create() lost the owner parameter in 6.4 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
#define compat_class_create(owner, name) class_create(name)
/* Auto-redirect 2-arg class_create(THIS_MODULE, name) to 1-arg */
#define _compat_class_create_1(name)          class_create(name)
#define _compat_class_create_2(owner, name)   class_create(name)
#define _compat_cc_sel(_1, _2, NAME, ...)     NAME
#undef class_create
#define class_create(...) _compat_cc_sel(__VA_ARGS__, _compat_class_create_2, _compat_class_create_1)(__VA_ARGS__)
#else
#define compat_class_create(owner, name) class_create(owner, name)
#endif

/* ========================================================================
 * 9. Semaphore
 * ======================================================================== */

/*
 * DEFINE_SEMAPHORE() gained count parameter in 6.4
 * Old: DEFINE_SEMAPHORE(name)        -- count=1
 * New: DEFINE_SEMAPHORE(name, count)
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
#define COMPAT_DEFINE_SEMAPHORE(name) DEFINE_SEMAPHORE(name, 1)
#else
#define COMPAT_DEFINE_SEMAPHORE(name) DEFINE_SEMAPHORE(name)
#endif

/* ========================================================================
 * 10. Miscellaneous
 * ======================================================================== */

/* pci_free_irq_vectors() available since 4.10 - safe to use */

/* get_user_pages() signature changes:
 * 4.9:  get_user_pages(tsk, mm, start, nr, write, force, pages, vmas)
 * 4.10: get_user_pages(start, nr, gup_flags, pages, vmas)
 * 5.9:  get_user_pages(start, nr, gup_flags, pages)  -- vmas removed for non-locked
 * 6.5:  get_user_pages(start, nr, gup_flags, pages)   -- further simplification
 * Each module using this must be adapted individually.
 */

/* strlcpy() deprecated in 6.8, use strscpy() */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 8, 0)
#ifndef strlcpy
#define strlcpy(dst, src, size) strscpy((dst), (src), (size))
#endif
#endif

/* get_random_int() is gone in newer kernels; old driver code only needs a
 * 32-bit random value and can use get_random_u32() transparently. */
#include <linux/random.h>
#ifndef get_random_int
#define get_random_int() get_random_u32()
#endif

/* profile_event_register/unregister removed in 5.x
 * PROFILE_TASK_EXIT profiling hooks no longer available */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
#ifndef PROFILE_TASK_EXIT
#define PROFILE_TASK_EXIT 0
#endif
static inline int profile_event_register(int t, struct notifier_block *n) { return 0; }
static inline int profile_event_unregister(int t, struct notifier_block *n) { return 0; }
#endif

/* Historical devdrv IPC ids are referenced from product-shared source files
 * even when the selected per-product manager header no longer defines them. */
#ifndef LPR52_TYPE1
#define LPR52_TYPE1 0xa
#endif
#ifndef LPR52_LOGLEVEL_CMD
#define LPR52_LOGLEVEL_CMD 0x2
#endif
#ifndef LPR52_SOURECE_ID
#define LPR52_SOURECE_ID 0
#endif
#ifndef LPR52_TARGET_ID
#define LPR52_TARGET_ID 0x4
#endif

/* Legacy synchronous compression users still reference the pre-acomp type id. */
#include <linux/crypto.h>
#ifndef CRYPTO_ALG_TYPE_COMPRESS
#define CRYPTO_ALG_TYPE_COMPRESS CRYPTO_ALG_TYPE_ACOMPRESS
#endif
#ifndef CRYPTO_ALG_TYPE_COMPRESS_MASK
#define CRYPTO_ALG_TYPE_COMPRESS_MASK CRYPTO_ALG_TYPE_ACOMPRESS_MASK
#endif

/* Legacy sync compression helpers were folded into acomp. Recreate the old
 * crypto_comp API shape on top of acomp so migrated drivers keep working. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
#include <crypto/acompress.h>
#define crypto_comp crypto_acomp

static inline struct crypto_comp *crypto_alloc_comp(const char *alg_name,
	u32 type, u32 mask)
{
	type &= ~CRYPTO_ALG_TYPE_MASK;
	type |= CRYPTO_ALG_TYPE_ACOMPRESS;
	mask |= CRYPTO_ALG_TYPE_ACOMPRESS_MASK;

	return crypto_alloc_acomp(alg_name, type, mask);
}

static inline void crypto_free_comp(struct crypto_comp *tfm)
{
	crypto_free_acomp(tfm);
}

static inline int crypto_comp_compress(struct crypto_comp *tfm, const u8 *src,
	unsigned int slen, u8 *dst, unsigned int *dlen)
{
	DECLARE_CRYPTO_WAIT(wait);
	ACOMP_REQUEST_ON_STACK(req, tfm);
	int ret;

	acomp_request_set_src_nondma(req, src, slen);
	acomp_request_set_dst_nondma(req, dst, *dlen);
	acomp_request_set_callback(req, CRYPTO_TFM_REQ_MAY_SLEEP,
		crypto_req_done, &wait);
	ret = crypto_wait_req(crypto_acomp_compress(req), &wait);
	if (ret == 0)
		*dlen = req->dlen;

	return ret;
}

static inline int crypto_comp_decompress(struct crypto_comp *tfm,
	const u8 *src, unsigned int slen, u8 *dst, unsigned int *dlen)
{
	DECLARE_CRYPTO_WAIT(wait);
	ACOMP_REQUEST_ON_STACK(req, tfm);
	int ret;

	acomp_request_set_src_nondma(req, src, slen);
	acomp_request_set_dst_nondma(req, dst, *dlen);
	acomp_request_set_callback(req, CRYPTO_TFM_REQ_MAY_SLEEP,
		crypto_req_done, &wait);
	ret = crypto_wait_req(crypto_acomp_decompress(req), &wait);
	if (ret == 0)
		*dlen = req->dlen;

	return ret;
}
#endif

/* __get_free_pages / free_pages unchanged */

/* of_device_is_available() unchanged */

/* platform_driver_register / platform_device_register unchanged */

/* pci_iomap / pci_iounmap unchanged */

/* ========================================================================
 * 11. Firmware
 * ======================================================================== */

/* request_firmware() / release_firmware() unchanged */

/* ========================================================================
 * 12. GPIO
 * ======================================================================== */

/* In 6.x, struct gpio_chip definition moved to <linux/gpio/driver.h>
 * Include it so that modules using <linux/gpio.h> with struct gpio_chip still work */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
#include <linux/gpio/driver.h>
#endif

/* gpio_request/gpio_free deprecated in favor of gpiod_* API since 3.x
 * but still functional through 6.x via <linux/gpio.h>.
 * Full migration to gpiod_* is recommended but not required.
 */

/* irq_domain_add_simple() renamed to irq_domain_create_simple() in 6.6+
 * and takes fwnode_handle* instead of device_node* */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
#include <linux/irqdomain.h>
#include <linux/of.h>
#define irq_domain_add_simple(node, size, first, ops, host_data) \
    irq_domain_create_simple(of_fwnode_handle(node), size, first, ops, host_data)
#endif

/* Legacy drivers still use the old 2-argument MSI descriptor iterator. */
#include <linux/msi.h>
#ifndef for_each_msi_entry
#define for_each_msi_entry(desc, dev) msi_for_each_desc((desc), (dev), MSI_DESC_ALL)
#endif

/* Old out-of-tree drivers still use the platform_msi_domain_* helpers.
 * The current kernel exposes the equivalent platform_device_msi_* APIs. */
#ifndef platform_msi_domain_alloc_irqs
#define platform_msi_domain_alloc_irqs(dev, nvec, write_msi_msg) \
	platform_device_msi_init_and_alloc_irqs((dev), (nvec), (write_msi_msg))
#endif
#ifndef platform_msi_domain_free_irqs
#define platform_msi_domain_free_irqs(dev) \
	platform_device_msi_free_irqs_all((dev))
#endif

/* i2c_driver.probe lost the id parameter in 6.3+ */
/* i2c_driver.remove returns void instead of int in 6.1+ */
/* These must be fixed per-module in source code */

/* cyclecounter.read callback gained const in 6.x */
/* Must be fixed per-module in source code */

/* Include vendor compat layer (nth_page, etc.) */
#include <linux/opiai_vendor_compat.h>

/* del_timer_sync renamed to timer_delete_sync in 6.4+ */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
#define del_timer_sync timer_delete_sync
#endif


/* follow_pfn() removed in 6.x; stub it (returns -EINVAL) */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
static inline int follow_pfn(struct vm_area_struct *vma, unsigned long address, unsigned long *pfn) { return -EINVAL; }
#endif

/* hrtimer_init renamed to hrtimer_setup in 6.x series */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 12, 0)
#define hrtimer_init(timer, clock, mode) hrtimer_setup(timer, NULL, clock, mode)
#endif
/* Huawei custom hugepage API stub */
#ifndef HUGETLB_ALLOC_BUDDY
#define HUGETLB_ALLOC_BUDDY 0
static inline struct page *hugetlb_alloc_hugepage(int nid, int flag) { return NULL; }
#endif

/* Huawei custom mmap API stubs */

#ifndef __do_mmap_mm

static inline unsigned long __do_mmap_mm(struct mm_struct *mm, struct file *file, unsigned long addr,

    unsigned long len, unsigned long prot, unsigned long flags,

    unsigned long pgoff, unsigned long *populate, struct list_head *uf) { return -ENOTSUPP; }

#endif

/* IOMMU device feature API removed in kernel 6.x */

#ifndef IOMMU_DEV_FEAT_IOPF

#define IOMMU_DEV_FEAT_IOPF 0

#define IOMMU_DEV_FEAT_SVA 1

static inline int iommu_dev_enable_feature(struct device *dev, int feat) { return 0; }

static inline int iommu_dev_disable_feature(struct device *dev, int feat) { return 0; }

#endif

/* ========================================================================
 * 13. Kernel symbol compatibility for 6.x
 * ======================================================================== */

/* PMD_ALIGN removed as a standalone macro; reconstruct from PMD_SIZE */
#include <asm/pgtable-hwdef.h>
#ifndef PMD_ALIGN
#define PMD_ALIGN(addr) ALIGN(addr, PMD_SIZE)
#endif

/* del_timer() renamed to timer_delete() in 6.x */
#ifndef del_timer
#define del_timer(t) timer_delete(t)
#endif

/* bus_set_iommu() removed in 6.x — IOMMU ops are registered per-device now */
#include <linux/iommu.h>
static inline int bus_set_iommu(struct bus_type *bus, const struct iommu_ops *ops)
{
	return 0; /* no-op: modern kernels register IOMMU ops via driver probe */
}

/* iommu_domain_alloc(bus) removed in 6.x — replaced by per-device API.
 * Callers that really need a paging domain should migrate to
 * iommu_paging_domain_alloc(dev).  This stub keeps legacy wrappers building. */
static inline struct iommu_domain *iommu_domain_alloc(struct bus_type *bus)
{
	return NULL;
}

/*
 * compat_lookup_name() — resolve any kernel symbol (code or data) from a module.
 *
 * kallsyms_lookup_name() is not exported since 5.7. We bootstrap it:
 *   1. Use kprobe to find kallsyms_lookup_name (it IS a function)
 *   2. Call it to resolve any symbol including data (e.g. psci_ops)
 */
#include <linux/kprobes.h>

typedef unsigned long (*_kallsyms_lookup_name_t)(const char *name);

static inline unsigned long compat_lookup_name(const char *name)
{
	static _kallsyms_lookup_name_t _fn;

	if (!_fn) {
		struct kprobe kp = { .symbol_name = "kallsyms_lookup_name" };
		if (register_kprobe(&kp) < 0)
			return 0;
		_fn = (_kallsyms_lookup_name_t)kp.addr;
		unregister_kprobe(&kp);
	}
	return _fn ? _fn(name) : 0;
}

#endif /* __ASCEND_COMPAT_H__ */
