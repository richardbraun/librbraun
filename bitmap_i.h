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
