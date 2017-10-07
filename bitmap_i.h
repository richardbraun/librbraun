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

#ifndef _BITMAP_I_H
#define _BITMAP_I_H

#include <limits.h>

#include "macros.h"

#ifndef LONG_BIT
#define LONG_BIT ((int)(CHAR_BIT * sizeof(long)))
#endif /* LONG_BIT */

#define BITMAP_LONGS(nr_bits) DIV_CEIL(nr_bits, LONG_BIT)

/*
 * Adjust the bitmap pointer and the bit index so that the latter refers
 * to a bit inside the word pointed by the former.
 *
 * Implemented as a macro for const-correctness.
 */
#define bitmap_lookup(bmp, bitp)        \
MACRO_BEGIN                             \
    int i;                              \
                                        \
    i = BITMAP_LONGS(*(bitp) + 1) - 1;  \
    *(bmp) += i;                        \
    *(bitp) -= i * LONG_BIT;            \
MACRO_END

static inline unsigned long
bitmap_mask(int bit)
{
    return (1UL << bit);
}

/*
 * Return the index of the next set bit in the bitmap, starting (and
 * including) the given bit index, or -1 if the bitmap is empty. If
 * complement is true, bits are toggled before searching so that the
 * result is the index of the next zero bit.
 */
int bitmap_find_next_bit(const unsigned long *bm, int nr_bits, int bit,
                         int complement);

#endif /* _BITMAP_I_H */
