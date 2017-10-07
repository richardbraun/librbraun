/*
 * Copyright (c) 2013-2015 Richard Braun.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
