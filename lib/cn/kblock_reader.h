/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2020 Micron Technology, Inc.  All rights reserved.
 */

#ifndef HSE_KVS_CN_KBLOCK_READER_H
#define HSE_KVS_CN_KBLOCK_READER_H

#include <hse_util/inttypes.h>
#include <hse_ikvdb/tuple.h>

struct mpool;
struct mpool_mcache_map;
struct mblock_props;
struct bloom_desc;
struct wbt_desc;
struct kvs_mblk_desc;
struct kvs_rparams;

struct kblk_metrics {
    u32 num_keys;
    u32 num_tombstones;
    u64 tot_key_bytes;
    u64 tot_val_bytes;
    u32 tot_wbt_pages;
    u32 tot_blm_pages;
};

/**
 * kbr_get_kblock_desc() - Get the KVBLOCK descriptor for the given
 *                     KBLOCK ID.
 * @kblock_id:      KBLOCK ID for KBLOCK to read
 * @kblock_desc:    (output) KVBLOCK_DESC for KBLOCK
 */
merr_t
kbr_get_kblock_desc(
    struct mpool *           ds,
    struct mpool_mcache_map *map,
    struct mblock_props     *props,
    u32                      map_idx,
    u64                      kblock_id,
    struct kvs_mblk_desc *   kblock_desc);

/**
 * kbr_read_wbt_region_desc() - Read the WBT region descriptor for
 *                          the given KBLOCK ID.
 * @kblock_desc:    KVBLOCK_DESC for KBLOCK to read
 * @wbt_rgn_desc:   (output) WBT descriptor
 */
merr_t
kbr_read_wbt_region_desc(struct kvs_mblk_desc *kblock_desc, struct wbt_desc *wbt_rgn_desc);

merr_t
kbr_read_wbt_region_desc_mem(void *hdr_page, struct wbt_desc *desc);

/**
 * kbr_read_blm_region_desc() - Read the Bloom filter region
 *                          descriptor for the given KBLOCK ID.
 * @kblock_desc:    KVBLOCK_DESC for KBLOCK to read
 * @blm_rgn_desc:   (output) bloom region descriptor
 * @kblock_id:      KBLOCK ID for KBLOCK to read
 */
merr_t
kbr_read_blm_region_desc(struct kvs_mblk_desc *kblock_desc, struct bloom_desc *blm_desc);

/**
 * kbr_read_blm_pages() - Read the Bloom filter pages
 *                          into an allocated buffer.
 * @kblock_desc:    KVBLOCK_DESC for KBLOCK to read
 * @cn_bloom_lookup: private and stable copy from rparams
 * @blm_desc:       bloom region descriptor
 * @blm_pages_out:  (output) buffer with bloom pages
 */
merr_t
kbr_read_blm_pages(
    struct kvs_mblk_desc *kblock_desc,
    ulong                 cn_bloom_lookup,
    struct bloom_desc *   blm_desc,
    u8 **                 blm_pages_out);

/**
 * kbr_read_pt_region_desc() - Read ptree pages into the descriptor
 * @kblkdesc: KVBLOCK_DESC for KBLOCK to read
 * @desc:     wbt descriptor
 */
merr_t
kbr_read_pt_region_desc(struct kvs_mblk_desc *kblkdesc, struct wbt_desc *desc);

merr_t
kbr_read_seqno_range(struct kvs_mblk_desc *kblkdesc, u64 *seqno_min, u64 *seqno_max);

void
kbr_free_blm_pages(struct kvs_mblk_desc *kbd, ulong cn_bloom_lookup, void *blm_pages);

/**
 * kbr_read_metrics() - Read kblock header to obtain metrics.
 *
 * @kblock_desc:    KVBLOCK_DESC for KBLOCK to read
 * @metrics:        (output) metrics saved here
 */
merr_t
kbr_read_metrics(struct kvs_mblk_desc *kblock_desc, struct kblk_metrics *metrics);

/**
 * kbr_madvise_wbt_leaf_nodes() - advise about caching of wbtree leaf nodes
 */
void
kbr_madvise_wbt_leaf_nodes(struct kvs_mblk_desc *kblkdesc, struct wbt_desc *desc, int advice);

/**
 * kbr_madvise_wbt_int_nodes() - advise about caching of wbtree internal modes
 */
void
kbr_madvise_wbt_int_nodes(struct kvs_mblk_desc *kblkdesc, struct wbt_desc *desc, int advice);

/**
 * kbr_madvise_kmd() - advise about caching of key meta-data region
 */
void
kbr_madvise_kmd(struct kvs_mblk_desc *kblkdesc, struct wbt_desc *desc, int advice);

/**
 * kbr_madvise_bloom() - advise about caching of bloom filter
 */
void
kbr_madvise_bloom(struct kvs_mblk_desc *kblkdesc, struct bloom_desc *desc, int advice);

#endif
