#ifndef _LINUX_OPIAI_VENDOR_COMPAT_H
#define _LINUX_OPIAI_VENDOR_COMPAT_H

#include <linux/device/class.h>
#include <linux/mm.h>
#include <linux/notifier.h>
#include <linux/oom.h>
#include <linux/panic_notifier.h>
#include <linux/proc_fs.h>
#include <linux/securec.h>
#include <linux/timer.h>
#include <linux/spi/spi.h>
#include <linux/version.h>

/*
 * Legacy vendor modules still use the pre-6.x class_create() signature.
 * Keep them building while we port the code incrementally.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
static inline struct class *opiai_class_create_compat(const char *name)
{
	return class_create(name);
}

#define __opiai_class_create1(name) opiai_class_create_compat(name)
#define __opiai_class_create2(owner, name) opiai_class_create_compat(name)
#define __opiai_class_create_get(_1, _2, name, ...) name
#define class_create(...) \
	__opiai_class_create_get(__VA_ARGS__, __opiai_class_create2, \
				 __opiai_class_create1)(__VA_ARGS__)
#endif

/*
 * Profiling task-exit hooks are gone in mainline. These modules only use them
 * for opportunistic cleanup, so treat them as optional on 6.18.
 */
#ifndef PROFILE_TASK_EXIT
#define PROFILE_TASK_EXIT 0
static inline int profile_event_register(int event, struct notifier_block *nb)
{
	return 0;
}

static inline int profile_event_unregister(int event, struct notifier_block *nb)
{
	return 0;
}
#endif

/*
 * nth_page() is not exported as a public macro on newer kernels. Vendor
 * modules still use it for compound-page cleanup, so reconstruct it from PFNs.
 */
#ifndef nth_page
#define nth_page(page, n) pfn_to_page(page_to_pfn(page) + (n))
#endif

/*
 * Vendor OOM callbacks are not present upstream. Falling back to a no-op keeps
 * the alloc fast path functional without introducing fake notifier plumbing.
 */
#ifndef OOM_TYPE_CGROUP
#define OOM_TYPE_CGROUP 0
static inline int oom_type_notifier_call(unsigned long event, void *data)
{
	return 0;
}
#endif

#ifndef register_hisi_oom_notifier
static inline int register_hisi_oom_notifier(struct notifier_block *nb)
{
	return register_oom_notifier(nb);
}

static inline int unregister_hisi_oom_notifier(struct notifier_block *nb)
{
	return unregister_oom_notifier(nb);
}
#endif

/*
 * procfs private-data helpers were renamed from PDE_DATA() to pde_data().
 */
#ifndef PDE_DATA
#define PDE_DATA(inode) pde_data(inode)
#endif

/*
 * printk_safe_flush_on_panic() disappeared from modern kernels. The closest
 * remaining primitive for these vendor modules is console_flush_on_panic().
 */
#ifndef printk_safe_flush_on_panic
static inline void printk_safe_flush_on_panic(void)
{
}
#endif

/*
 * Timer teardown helpers were renamed in newer kernels.
 */
#ifndef del_timer_sync
#define del_timer_sync(timer) timer_delete_sync(timer)
#endif

#ifndef from_timer
#define from_timer(var, callback_timer, timer_field) \
	timer_container_of(var, callback_timer, timer_field)
#endif

/*
 * The legacy integer GPIO devres free helper is gone. These vendor modules
 * still use devm_gpio_request_one(), so leaving release to devres is fine.
 */
#ifndef devm_gpio_free
static inline void devm_gpio_free(struct device *dev, unsigned int gpio)
{
}
#endif

/*
 * SPI controller helpers were renamed from master -> controller.
 */
#ifndef spi_alloc_master
#define spi_master spi_controller
#define spi_alloc_master(dev, size) spi_alloc_host(dev, size)
#define spi_master_get_devdata(master) spi_controller_get_devdata(master)
#define spi_register_master(master) spi_register_controller(master)
#define spi_unregister_master(master) spi_unregister_controller(master)
#endif

#endif
