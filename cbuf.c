/*
 * Copyright (c) 2015-2017 Richard Braun.
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

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "cbuf.h"
#include "error.h"
#include "macros.h"

/* Negative close to 0 so that an overflow occurs early */
#define CBUF_INIT_INDEX ((size_t)-500)

void
cbuf_init(struct cbuf *cbuf, void *buf, size_t capacity)
{
    assert(ISP2(capacity));

    cbuf->buf = buf;
    cbuf->capacity = capacity;
    cbuf->start = CBUF_INIT_INDEX;
    cbuf->end = cbuf->start;
}

static size_t
cbuf_index(const struct cbuf *cbuf, size_t abs_index)
{
    return abs_index & (cbuf->capacity - 1);
}

static void
cbuf_update_start(struct cbuf *cbuf)
{
    /* Mind integer overflows */
    if (cbuf_size(cbuf) > cbuf->capacity) {
        cbuf->start = cbuf->end - cbuf->capacity;
    }
}

int
cbuf_push(struct cbuf *cbuf, const void *buf, size_t size, bool erase)
{
    size_t free_size;

    if (!erase) {
        free_size = cbuf_capacity(cbuf) - cbuf_size(cbuf);

        if (size > free_size) {
            return ERROR_AGAIN;
        }
    }

    return cbuf_write(cbuf, cbuf_end(cbuf), buf, size);
}

int
cbuf_pop(struct cbuf *cbuf, void *buf, size_t *sizep)
{
    __unused int error;

    if (cbuf_size(cbuf) == 0) {
        return ERROR_AGAIN;
    }

    error = cbuf_read(cbuf, cbuf_start(cbuf), buf, sizep);
    assert(!error);
    cbuf->start += *sizep;
    return 0;
}

int
cbuf_pushb(struct cbuf *cbuf, uint8_t byte, bool erase)
{
    size_t free_size;

    if (!erase) {
        free_size = cbuf_capacity(cbuf) - cbuf_size(cbuf);

        if (free_size == 0) {
            return ERROR_AGAIN;
        }
    }

    cbuf->buf[cbuf_index(cbuf, cbuf->end)] = byte;
    cbuf->end++;
    cbuf_update_start(cbuf);
    return 0;
}

int
cbuf_popb(struct cbuf *cbuf, void *bytep)
{
    uint8_t *ptr;

    if (cbuf_size(cbuf) == 0) {
        return ERROR_AGAIN;
    }

    ptr = bytep;
    *ptr = cbuf->buf[cbuf_index(cbuf, cbuf->start)];
    cbuf->start++;
    return 0;
}

int
cbuf_write(struct cbuf *cbuf, size_t index, const void *buf, size_t size)
{
    uint8_t *start, *end, *buf_end;
    size_t new_end, skip;

    if (!cbuf_range_valid(cbuf, index, cbuf->end)) {
        return ERROR_INVAL;
    }

    new_end = index + size;

    if (!cbuf_range_valid(cbuf, cbuf->start, new_end)) {
        cbuf->end = new_end;

        if (size > cbuf_capacity(cbuf)) {
            skip = size - cbuf_capacity(cbuf);
            buf += skip;
            index += skip;
            size = cbuf_capacity(cbuf);
        }
    }

    start = &cbuf->buf[cbuf_index(cbuf, index)];
    end = start + size;
    buf_end = cbuf->buf + cbuf->capacity;

    if ((end <= cbuf->buf) || (end > buf_end)) {
        skip = buf_end - start;
        memcpy(start, buf, skip);
        buf += skip;
        start = cbuf->buf;
        size -= skip;
    }

    memcpy(start, buf, size);
    cbuf_update_start(cbuf);
    return 0;
}

int
cbuf_read(const struct cbuf *cbuf, size_t index, void *buf, size_t *sizep)
{
    const uint8_t *start, *end, *buf_end;
    size_t size;

    /* At least one byte must be available */
    if (!cbuf_range_valid(cbuf, index, index + 1)) {
        return ERROR_INVAL;
    }

    size = cbuf->end - index;

    if (*sizep > size) {
        *sizep = size;
    }

    start = &cbuf->buf[cbuf_index(cbuf, index)];
    end = start + *sizep;
    buf_end = cbuf->buf + cbuf->capacity;

    if ((end > cbuf->buf) && (end <= buf_end)) {
        size = *sizep;
    } else {
        size = buf_end - start;
        memcpy(buf, start, size);
        buf += size;
        start = cbuf->buf;
        size = *sizep - size;
    }

    memcpy(buf, start, size);
    return 0;
}
