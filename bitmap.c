/*
 * Copyright (c) 2013-2015 Richard Braun.
 * All rights reserved.
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
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#include <limits.h>
#include <stddef.h>
#include <string.h>

#include "bitmap.h"
#include "bitmap_i.h"

int
bitmap_cmp(const unsigned long *a, const unsigned long *b, int nr_bits)
{
    unsigned long last_a, last_b, mask;
    int n, rv;

    n = BITMAP_LONGS(nr_bits) - 1;

    if (n != 0) {
        rv = memcmp(a, b, n * sizeof(unsigned long));

        if (rv != 0) {
            return rv;
        }

        nr_bits -= n * LONG_BIT;
    }

    last_a = a[n];
    last_b = b[n];

    if (nr_bits != LONG_BIT) {
        mask = (1UL << nr_bits) - 1;
        last_a &= mask;
        last_b &= mask;
    }

    if (last_a == last_b) {
        return 0;
    } else if (last_a < last_b) {
        return -1;
    } else {
        return 1;
    }
}

static inline unsigned long
bitmap_find_next_compute_complement(unsigned long word, int nr_bits)
{
    if (nr_bits < LONG_BIT) {
        word |= (((unsigned long)-1) << nr_bits);
    }

    return ~word;
}

int
bitmap_find_next_bit(const unsigned long *bm, int nr_bits, int bit,
                     int complement)
{
    const unsigned long *start, *end;
    unsigned long word;

    start = bm;
    end = bm + BITMAP_LONGS(nr_bits);

    if (bit >= LONG_BIT) {
        bitmap_lookup(&bm, &bit);
        nr_bits -= ((bm - start) * LONG_BIT);
    }

    word = *bm;

    if (complement) {
        word = bitmap_find_next_compute_complement(word, nr_bits);
    }

    if (bit < LONG_BIT) {
        word &= ~(bitmap_mask(bit) - 1);
    }

    for (;;) {
        bit = __builtin_ffsl(word);

        if (bit != 0) {
            return ((bm - start) * LONG_BIT) + bit - 1;
        }

        bm++;

        if (bm >= end) {
            return -1;
        }

        nr_bits -= LONG_BIT;
        word = *bm;

        if (complement) {
            word = bitmap_find_next_compute_complement(word, nr_bits);
        }
    }
}
