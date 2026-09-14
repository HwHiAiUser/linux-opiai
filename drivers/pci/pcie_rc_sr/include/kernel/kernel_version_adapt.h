/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-06-26
 */

#ifndef __KERNEL_VERSION_ADAPT_H
#define __KERNEL_VERSION_ADAPT_H

#ifndef DVPP_UTEST
#include <linux/version.h>
#include <linux/sched.h>
#include <linux/mm_types.h>
#include <linux/vmalloc.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
#include <linux/time.h>
#include <linux/time64.h>
#include <linux/timekeeping.h>
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
#define devm_ioremap_nocache devm_ioremap
#ifndef AOS_LLVM_BUILD
#define getrawmonotonic64 ktime_get_raw_ts64
#endif
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0)
#ifndef STRUCT_TIMEVAL_SPEC
#define STRUCT_TIMEVAL_SPEC
struct timespec {
    __kernel_old_time_t          tv_sec;         /* seconds */
    long                    tv_nsec;        /* nanoseconds */
};
struct timeval {
    __kernel_old_time_t          tv_sec;         /* seconds */
    __kernel_suseconds_t    tv_usec;        /* microseconds */
};
#endif
#endif

unsigned long kallsyms_lookup_name(const char *name);
void *__symbol_get(const char *name);
void __symbol_put(const char *name);

static inline unsigned long __kallsyms_lookup_name(const char *name)
{
    unsigned long symbol = 0;

    if (name == NULL) {
        return 0;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)) && (!defined(AOS_LLVM_BUILD))
    symbol = (unsigned long)__symbol_get(name);
    if (symbol == 0) {
        return 0;
    }
    __symbol_put(name);

#else
    symbol = kallsyms_lookup_name(name);
#endif
    return symbol;
}

static inline unsigned long __kallsyms_get(const char *name)
{
    unsigned long symbol = 0;

    if (name == NULL) {
        return 0;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)) && (!defined(AOS_LLVM_BUILD))
    symbol = (unsigned long)__symbol_get(name);
    if (symbol == 0) {
        return 0;
    }
#else
    symbol = kallsyms_lookup_name(name);
#endif
    return symbol;
}

static inline void __kallsyms_put(const char *name)
{
    if (name == NULL) {
        return;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)) && (!defined(AOS_LLVM_BUILD))
    __symbol_put(name);
#endif
}

#ifndef AOS_LLVM_BUILD
// os version below 3.17 real_start_time type is struct timespec
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
static inline u64 get_start_time(struct task_struct *cur)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 5, 0)
    return cur->start_boottime;
#else
    return cur->real_start_time;
#endif
}
#else
static inline struct timespec get_start_time(struct task_struct *cur)
{
    return cur->real_start_time;
}
#endif

static inline struct rw_semaphore *get_mmap_sem(struct mm_struct *mm)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
    return &mm->mmap_lock;
#else
    return &mm->mmap_sem;
#endif
}
#endif

static inline void *ka_vmalloc(unsigned long size, gfp_t gfp_mask, pgprot_t prot)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)) && (!defined AOS_LLVM_BUILD)
    (void)prot;
    return __vmalloc(size, gfp_mask);
#else
    return __vmalloc(size, gfp_mask, prot);
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
static inline struct timespec current_kernel_time(void)
{
    struct timespec64 ts64;
    struct timespec ts;

    ktime_get_coarse_real_ts64(&ts64);
    ts.tv_sec = (__kernel_long_t)ts64.tv_sec;
    ts.tv_nsec = ts64.tv_nsec;
    return ts;
}
#endif

#endif

#if defined(__sw_64__)
#ifndef PAGE_SHARED_EXEC
#define PAGE_SHARED_EXEC PAGE_SHARED
#endif
#endif

#endif  /* __KERNEL_VERSION_ADAPT_H */
