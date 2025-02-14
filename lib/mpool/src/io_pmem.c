/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2021 Micron Technology, Inc.  All rights reserved.
 */

#include <hse_util/base.h>
#include <hse_util/mman.h>
#include <hse_util/assert.h>
#include <hse_util/page.h>

#include "io.h"

#include <libpmem.h>

merr_t
io_pmem_read(
    int                 src_fd,
    off_t               off,
    const struct iovec *iov,
    int                 iovcnt,
    int                 flags,
    size_t             *rdlen)
{
    return io_sync_ops.read(src_fd, off, iov, iovcnt, flags, rdlen);
}

merr_t
io_pmem_write(
    int                 dst_fd,
    off_t               off,
    const struct iovec *iov,
    int                 iovcnt,
    int                 flags,
    size_t             *wrlen)
{
    return io_sync_ops.write(dst_fd, off, iov, iovcnt, flags, wrlen);
}

merr_t
io_pmem_mmap(void **addr, size_t len, int prot, int flags, int fd, off_t offset)
{
    void *addrout;

    INVARIANT(addr);

    flags |= (MAP_SHARED_VALIDATE | MAP_SYNC);

    addrout = mmap(*addr, len, prot, flags, fd, offset);
    if (addrout == MAP_FAILED)
        return merr(errno);

    *addr = addrout;

    return 0;
}

merr_t
io_pmem_munmap(void *addr, size_t len)
{
    int rc;

    INVARIANT(addr);

    rc = munmap(addr, len);

    return (rc == -1) ? merr(errno) : 0;
}

merr_t
io_pmem_msync(void *addr, size_t len, int flags)
{
    pmem_persist(addr, len);

    return 0;
}

const struct io_ops io_pmem_ops = {
    .read = io_pmem_read,
    .write = io_pmem_write,
    .mmap = io_pmem_mmap,
    .munmap = io_pmem_munmap,
    .msync = io_pmem_msync,
};
