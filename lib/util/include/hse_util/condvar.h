/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2015-2021 Micron Technology, Inc.  All rights reserved.
 *
 * The condvar subsystem is an abstraction of the pthread condition
 * variable APIs taht provide the same general semantics in both user
 * and kernel space.
 *
 * Note that this implementation leverages the caller's mutex to protect
 * against concurrent update of the sleep queue and waiters count.  It is
 * therefore imperative that one and only one mutex be associated with a
 * given condvar, and that all callers of cv_signal() and cv_broadcast()
 * hold that mutex.  In practice this is not a burdensome requirement, as
 * this approach is generally necessary in order to avoid lost wakeups.
 */
#ifndef HSE_PLATFORM_CONDVAR_H
#define HSE_PLATFORM_CONDVAR_H

#include <pthread.h>
#include <stdlib.h>

#include <hse_util/assert.h>
#include <hse_util/compiler.h>
#include <hse_util/mutex.h>

struct cv {
    long           cv_waiters;
    pthread_cond_t cv_waitq;
    const char *   cv_desc;
};

static HSE_ALWAYS_INLINE void
cv_signal(struct cv *cv)
{
    int rc;

    if (cv->cv_waiters > 0) {
        rc = pthread_cond_signal(&cv->cv_waitq);

        if (HSE_UNLIKELY(rc))
            abort();
    }
}

static HSE_ALWAYS_INLINE void
cv_broadcast(struct cv *cv)
{
    int rc;

    if (cv->cv_waiters > 0) {
        rc = pthread_cond_broadcast(&cv->cv_waitq);

        if (HSE_UNLIKELY(rc))
            abort();
    }
}

/**
 * cv_init() - initialize a condition variable
 * @cv:     ptr to a condition variable
 * @desc:   ptr to cv description (used for WCHAN by ps).
 *
 * Provides the same general semantics as pthread_cond_init().
 *
 * The memory used by desc must remain valid until the cv is destroyed.
 */
void
cv_init(struct cv *cv, const char *desc);

/**
 * cv_destroy() - destroy a condition variable
 * @cv:     ptr to a condition variable
 *
 * Provides the same general semantics as pthread_cond_dstroy().
 */
void
cv_destroy(struct cv *cv);

/**
 * cv_timedwait() - wait for condition variable to be signaled or timeout
 * @cv:         ptr to a condition variable
 * @mtx:        ptr to mutex
 * @timeout:    maximum number of milliseconds to sleep
 *
 * Provides the same general semantics as pthread_cond_timedwait().
 * If %timeout is negative cv_timedwait() blocks indefinitely.
 *
 * Return: 0 if awakened by cv_signal() or cv_broadcast(), ETIMEDOUT
 * if the timeout expires.
 */
int
cv_timedwait(struct cv *cv, struct mutex *mtx, const int timeout);

/**
 * cv_wait() - wait for condition variable to be signaled
 * @cv:     ptr to a condition variable
 * @mtx:    ptr to mutex
 *
 * Provides the same general semantics as pthread_cond_wait().
 */
static HSE_ALWAYS_INLINE void
cv_wait(struct cv *cv, struct mutex *mtx)
{
    cv_timedwait(cv, mtx, -1);
}

#endif /* HSE_PLATFORM_CONDVAR_H */
