/*
 * Copyright (c) 2015 Richard Braun.
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

#include <assert.h>

#include "cbuf.h"
#include "error.h"
#include "macros.h"

/* Negative close to 0 so that an overflow occurs early */
#define CBUF_INIT_INDEX ((unsigned long)-500)

void
cbuf_init(struct cbuf *cbuf, char *buf, unsigned long capacity)
{
    assert(ISP2(capacity));

    cbuf->buf = buf;
    cbuf->capacity = capacity;
    cbuf->start = CBUF_INIT_INDEX;
    cbuf->end = cbuf->start;
}

static unsigned long
cbuf_index(const struct cbuf *cbuf, unsigned long abs_index)
{
    return abs_index & (cbuf->capacity - 1);
}

void
cbuf_push(struct cbuf *cbuf, char byte)
{
    cbuf->buf[cbuf_index(cbuf, cbuf->end)] = byte;
    cbuf->end++;

    /* Mind integer overflows */
    if (cbuf_size(cbuf) > cbuf->capacity) {
        cbuf->start = cbuf->end - cbuf->capacity;
    }
}

int
cbuf_pop(struct cbuf *cbuf, char *bytep)
{
    if (cbuf_size(cbuf) == 0) {
        return ERR_AGAIN;
    }

    *bytep = cbuf->buf[cbuf_index(cbuf, cbuf->start)];
    cbuf->start++;
    return 0;
}

int
cbuf_read(const struct cbuf *cbuf, unsigned long index, char *bytep)
{
    /* Mind integer overflows */
    if ((cbuf->end - index - 1) >= cbuf_size(cbuf)) {
        return ERR_INVAL;
    }

    *bytep = cbuf->buf[cbuf_index(cbuf, index)];
    return 0;
}
