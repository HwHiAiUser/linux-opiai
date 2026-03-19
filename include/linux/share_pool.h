/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SHARE_POOL_H
#define _LINUX_SHARE_POOL_H

#include <linux/errno.h>
#include <linux/err.h>
#include <linux/mm_types.h>
#include <linux/nodemask.h>
#include <linux/types.h>

/*
 * The original vendor tree carries a large share-pool implementation in mm/.
 * Our 6.18 port does not bring that subsystem back, but several vendor
 * modules still include this header. Provide a small compatibility surface so
 * those modules can keep building with the share-pool feature effectively
 * disabled.
 */

#define SP_HUGEPAGE            (1UL << 0)
#define SP_HUGEPAGE_ONLY       (1UL << 1)
#define SP_DVPP                (1UL << 2)
#define SP_SPEC_NODE_ID        (1UL << 3)
#define SP_PFN_MAP             (1UL << 4)

#define SPG_ID_DEFAULT         0
#define SPG_ID_NONE            (-1)

static inline int mg_sp_group_add_task(int pid, unsigned long prot, int spg_id)
{
	(void)pid;
	(void)prot;
	(void)spg_id;
	return -EPERM;
}

static inline int mg_sp_group_del_task(int pid, int spg_id)
{
	(void)pid;
	(void)spg_id;
	return -EPERM;
}

static inline void *mg_sp_alloc(unsigned long size, unsigned long sp_flags, int spg_id)
{
	(void)size;
	(void)sp_flags;
	(void)spg_id;
	return NULL;
}

static inline void *mg_sp_alloc_nodemask(unsigned long size, unsigned long sp_flags,
					 int spg_id, nodemask_t nodemask)
{
	(void)size;
	(void)sp_flags;
	(void)spg_id;
	(void)nodemask;
	return NULL;
}

static inline int mg_sp_free(unsigned long addr, int id)
{
	(void)addr;
	(void)id;
	return -EPERM;
}

static inline void *mg_sp_make_share_k2u(unsigned long kva, unsigned long size,
					 unsigned long sp_flags, int pid, int spg_id)
{
	(void)kva;
	(void)size;
	(void)sp_flags;
	(void)pid;
	(void)spg_id;
	return ERR_PTR(-EPERM);
}

static inline void *mg_sp_make_share_u2k(unsigned long uva, unsigned long size, int pid)
{
	(void)uva;
	(void)size;
	(void)pid;
	return ERR_PTR(-EPERM);
}

static inline int mg_sp_unshare(unsigned long va, unsigned long size, int spg_id)
{
	(void)va;
	(void)size;
	(void)spg_id;
	return -EPERM;
}

static inline bool mg_is_sharepool_addr(unsigned long addr)
{
	(void)addr;
	return false;
}

static inline int mg_sp_id_of_current(void)
{
	return -EPERM;
}

static inline bool is_cdm_node(int nid)
{
	(void)nid;
	return false;
}

#endif /* _LINUX_SHARE_POOL_H */
