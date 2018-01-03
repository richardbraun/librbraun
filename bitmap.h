/*
 * Copyright (c) 2013-2015 Richard Braun.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 *
 *
 * Arbitrary-length bit arrays.
 *
 * Most functions do not check whether the given parameters are valid. This
 * is the responsibility of the caller.
 */

#ifndef _BITMAP_H
#define _BITMAP_H

#include <limits.h>
#include <stdatomic.h>
#include <string.h>

#include "bitmap_i.h"

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
        bitmap_lookup(&bm, &bit);
    }

    *bm |= bitmap_mask(bit);
}

static inline void
bitmap_set_atomic(unsigned long *bm, int bit)
{
    atomic_ulong *ptr;

    if (bit >= LONG_BIT) {
        bitmap_lookup(&bm, &bit);
    }

    ptr = (atomic_ulong *)bm;
    atomic_fetch_or_explicit(ptr, bitmap_mask(bit), memory_order_release);
}

static inline void
bitmap_clear(unsigned long *bm, int bit)
{
    if (bit >= LONG_BIT) {
        bitmap_lookup(&bm, &bit);
    }

    *bm &= ~bitmap_mask(bit);
}

static inline void
bitmap_clear_atomic(unsigned long *bm, int bit)
{
    atomic_ulong *ptr;

    if (bit >= LONG_BIT) {
        bitmap_lookup(&bm, &bit);
    }

    ptr = (atomic_ulong *)bm;
    atomic_fetch_and_explicit(ptr, ~bitmap_mask(bit), memory_order_acquire);
}

static inline int
bitmap_test(const unsigned long *bm, int bit)
{
    if (bit >= LONG_BIT) {
        bitmap_lookup(&bm, &bit);
    }

    return ((*bm & bitmap_mask(bit)) != 0);
}

static inline int
bitmap_test_atomic(const unsigned long *bm, int bit)
{
    atomic_ulong *ptr;

    if (bit >= LONG_BIT) {
        bitmap_lookup(&bm, &bit);
    }

    ptr = (atomic_ulong *)bm;
    return ((atomic_load_explicit(ptr, memory_order_acquire)
             & bitmap_mask(bit)) != 0);
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
