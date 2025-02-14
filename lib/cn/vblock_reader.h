/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2020 Micron Technology, Inc.  All rights reserved.
 */

#ifndef HSE_KVS_CN_VBLOCK_READER_H
#define HSE_KVS_CN_VBLOCK_READER_H

#include <hse_util/inttypes.h>
#include <hse_util/workqueue.h>
#include <hse_ikvdb/tuple.h>

#include "kvs_mblk_desc.h"

/* ra_flags for vbr_readahead()
 *
 * VBR_REVERSE    caller is performing reverse iteration
 * VBR_FULLSCAN   caller intends to read all values
 */
#define VBR_REVERSE     (0x0001u)
#define VBR_FULLSCAN    (0x0002u)

struct mpool;
struct mpool_mcache_map;
struct mblock_props;

/**
 * struct ra_hist - readahead history cache record
 * vgidx:   vblock group index
 * bkt:     current or next range already read ahead
 */
struct ra_hist {
    u16 vgidx;
    u16 bkt;
};

/**
 * struct vbr_madvise_work - async vblock readahead params
 */
struct vbr_madvise_work {
    struct work_struct  vmw_work;
    struct vblock_desc *vmw_vbd;
    uint                vmw_off;
    uint                vmw_len;
    int                 vmw_advice;
};

/**
 * struct vblock_desc - a descriptor for reading data from a vblock
 *
 * When a vblock is set up for reading, the @vblock_hdr_omf struct is read
 * from media and the relevant information is stored in a @vblock_desc struct.
 */
struct vblock_desc {
    struct kvs_mblk_desc vbd_mblkdesc; /* underlying block descriptor */
    u32                  vbd_off;      /* byte offset of vblock data */
    u32                  vbd_len;      /* byte length of vblock data */
    u64                  vbd_vgroup;   /* vblock group ID (dgen_hi) */
    atomic_int           vbd_vgidx;    /* vblock group index */
    atomic_int           vbd_refcnt;   /* vbr_madvise_async() refcnt */
};

/**
 * vbr_desc_read() - Read the region descriptor for the given vblock
 * @vlock_desc:    (output) vblock descriptor
 */
merr_t
vbr_desc_read(
    struct mpool *           ds,
    struct mpool_mcache_map *map,
    uint                     maxidx,
    uint *                   vgroupsp,
    u64 *                    argv,
    struct mblock_props *    props,
    struct vblock_desc *     vblock_desc);

merr_t
vbr_desc_update(
    struct mpool *           ds,
    struct mpool_mcache_map *map,
    uint                     maxidx,
    uint *                   vgroupsp,
    u64 *                    argv,
    struct mblock_props *    props,
    struct vblock_desc *     vblock_desc);

/**
 * vbr_madvise_async() - initiate async vblock readahead
 */
bool
vbr_madvise_async(
    struct vblock_desc *     vbd,
    uint                     off,
    uint                     len,
    int                      advice,
    struct workqueue_struct *wq);

/**
 * vbr_readahead() - tickle read-ahead logic
 * @vbd:   vblock descriptor
 *
 * If this function decides there may be a benefit to vblock readahead
 * it will either call vbr_madvise() or vbr_madvise_async() to perform
 * the work.
 */
void
vbr_readahead(
    struct vblock_desc *     vbd,
    u32                      off,
    u32                      vlen,
    u32                      ra_flags,
    u32                      ra_len,
    u32                      ra_histc,
    struct ra_hist *         ra_histv,
    struct workqueue_struct *wq);

/**
 * vbr_madvise() - tickle read-ahead logic for a more aggressive
 *                        sequential read ahead
 * @vbd:   vblock descriptor
 */
void
vbr_madvise(struct vblock_desc *vbd, uint off, uint len, int advice);

/**
 * vbr_value() - Get ptr to a value stored in a vblock
 * @vbd:   identifies vblock to read
 * @vboff: offset of value within vblock
 * @vlen:  length of value
 */
void *
vbr_value(struct vblock_desc *vbd, uint vboff, uint vlen);

#endif
