/*
 * Copyright (c) 2013-2015 Richard Braun.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Arbitrary-length bit arrays.
 *
 * Most functions do not check whether the given parameters are valid. This
 * is the responsibility of the caller.
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#ifndef _BITMAP_H
#define _BITMAP_H

#include <limits.h>
#include <string.h>

#include "bitmap_i.h"
#include "x86/atomic.h"

#define BITMAP_DECLARE(name, nr_bits) unsigned long name[BITMAP_LONGS(nr_bits)]

int bitmap_cmp(const unsigned long *a, const unsigned long *b, int nr_bits);

static inline void
bitmap_zero(unsigned long *bm, int nr_bits)
{
    int n;

    n = BITMAP_LONGS(nr_bits);
    memset(bm, 0, n * sizeof(unsigned long));
}

static inline void
bitmap_fill(unsigned long *bm, int nr_bits)
{
    int n;

    n = BITMAP_LONGS(nr_bits);
    memset(bm, 0xff, n * sizeof(unsigned long));
}

static inline void
bitmap_copy(unsigned long *dest, const unsigned long *src, int nr_bits)
{
    int n;

    n = BITMAP_LONGS(nr_bits);
    memcpy(dest, src, n * sizeof(unsigned long));
}

static inline void
bitmap_set(unsigned long *bm, int bit)
{
    if (bit >= LONG_BIT) {
        bitmap_lookup(bm, bit);
    }

    *bm |= bitmap_mask(bit);
}

static inline void
bitmap_set_atomic(unsigned long *bm, int bit)
{
    if (bit >= LONG_BIT) {
        bitmap_lookup(bm, bit);
    }

    atomic_or_ulong(bm, bitmap_mask(bit));
}

static inline void
bitmap_clear(unsigned long *bm, int bit)
{
    if (bit >= LONG_BIT) {
        bitmap_lookup(bm, bit);
    }

    *bm &= ~bitmap_mask(bit);
}

static inline void
bitmap_clear_atomic(unsigned long *bm, int bit)
{
    if (bit >= LONG_BIT) {
        bitmap_lookup(bm, bit);
    }

    atomic_and_ulong(bm, ~bitmap_mask(bit));
}

static inline int
bitmap_test(const unsigned long *bm, int bit)
{
    if (bit >= LONG_BIT) {
        bitmap_lookup(bm, bit);
    }

    return ((*bm & bitmap_mask(bit)) != 0);
}

static inline void
bitmap_and(unsigned long *a, const unsigned long *b, int nr_bits)
{
    int i, n;

    n = BITMAP_LONGS(nr_bits);

    for (i = 0; i < n; i++) {
        a[i] &= b[i];
    }
}

static inline void
bitmap_or(unsigned long *a, const unsigned long *b, int nr_bits)
{
    int i, n;

    n = BITMAP_LONGS(nr_bits);

    for (i = 0; i < n; i++) {
        a[i] |= b[i];
    }
}

static inline void
bitmap_xor(unsigned long *a, const unsigned long *b, int nr_bits)
{
    int i, n;

    n = BITMAP_LONGS(nr_bits);

    for (i = 0; i < n; i++) {
        a[i] ^= b[i];
    }
}

static inline int
bitmap_find_next(const unsigned long *bm, int nr_bits, int bit)
{
    return bitmap_find_next_bit(bm, nr_bits, bit, 0);
}

static inline int
bitmap_find_first(const unsigned long *bm, int nr_bits)
{
    return bitmap_find_next(bm, nr_bits, 0);
}

static inline int
bitmap_find_next_zero(const unsigned long *bm, int nr_bits, int bit)
{
    return bitmap_find_next_bit(bm, nr_bits, bit, 1);
}

static inline int
bitmap_find_first_zero(const unsigned long *bm, int nr_bits)
{
    return bitmap_find_next_zero(bm, nr_bits, 0);
}

#define bitmap_for_each(bm, nr_bits, bit)                       \
for ((bit) = 0;                                                 \
     ((bit) < nr_bits)                                          \
     && (((bit) = bitmap_find_next(bm, nr_bits, bit)) != -1);   \
     (bit)++)

#define bitmap_for_each_zero(bm, nr_bits, bit)                      \
for ((bit) = 0;                                                     \
     ((bit) < nr_bits)                                              \
     && (((bit) = bitmap_find_next_zero(bm, nr_bits, bit)) != -1);  \
     (bit)++)

#endif /* _BITMAP_H */
