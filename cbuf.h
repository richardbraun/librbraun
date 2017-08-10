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
 * Circular byte buffer.
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#ifndef _CBUF_H
#define _CBUF_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Circular buffer descriptor.
 *
 * The buffer capacity must be a power-of-two. Indexes are absolute values
 * which can overflow. Their difference cannot exceed the capacity.
 */
struct cbuf {
    uint8_t *buf;
    size_t capacity;
    size_t start;
    size_t end;
};

static inline size_t
cbuf_capacity(const struct cbuf *cbuf)
{
    return cbuf->capacity;
}

static inline size_t
cbuf_start(const struct cbuf *cbuf)
{
    return cbuf->start;
}

static inline size_t
cbuf_end(const struct cbuf *cbuf)
{
    return cbuf->end;
}

static inline size_t
cbuf_size(const struct cbuf *cbuf)
{
    return cbuf->end - cbuf->start;
}

static inline void
cbuf_clear(struct cbuf *cbuf)
{
    cbuf->start = cbuf->end;
}

static inline bool
cbuf_range_valid(const struct cbuf *cbuf, size_t start, size_t end)
{
    return (((end - start) <= cbuf_size(cbuf))
            && ((start - cbuf->start) <= cbuf_size(cbuf))
            && ((cbuf->end - end) <= cbuf_size(cbuf)));
}

/*
 * Initialize a circular buffer.
 *
 * The descriptor is set to use the given buffer for storage. Capacity
 * must be a power-of-two.
 */
void cbuf_init(struct cbuf *cbuf, void *buf, size_t capacity);

/*
 * Push data to a circular buffer.
 *
 * If the function isn't allowed to erase old data and the circular buffer
 * doesn't have enough unused bytes for the new data, ERROR_AGAIN is returned.
 */
int cbuf_push(struct cbuf *cbuf, const void *buf, size_t size, bool erase);

/*
 * Pop data from a circular buffer.
 *
 * On entry, the sizep argument points to the size of the output buffer.
 * On exit, it is updated to the number of bytes actually transferred.
 *
 * If the buffer is empty, ERROR_AGAIN is returned, and the size of the
 * output buffer is undefined.
 */
int cbuf_pop(struct cbuf *cbuf, void *buf, size_t *sizep);

/*
 * Push a byte to a circular buffer.
 *
 * If the function isn't allowed to erase old data and the circular buffer
 * is full, ERROR_AGAIN is returned.
 */
int cbuf_pushb(struct cbuf *cbuf, uint8_t byte, bool erase);

/*
 * Pop a byte from a circular buffer.
 *
 * If the buffer is empty, ERROR_AGAIN is returned.
 */
int cbuf_popb(struct cbuf *cbuf, void *bytep);

/*
 * Write into a circular buffer at a specific location.
 *
 * If the given index is outside buffer boundaries, ERROR_INVAL is returned.
 * The given [index, size) range may extend beyond the end of the circular
 * buffer.
 */
int cbuf_write(struct cbuf *cbuf, size_t index, const void *buf, size_t size);

/*
 * Read from a circular buffer at a specific location.
 *
 * On entry, the sizep argument points to the size of the output buffer.
 * On exit, it is updated to the number of bytes actually transferred.
 *
 * If the given index is outside buffer boundaries, ERROR_INVAL is returned.
 *
 * The circular buffer isn't changed by this operation.
 */
int cbuf_read(const struct cbuf *cbuf, size_t index, void *buf, size_t *sizep);

#endif /* _CBUF_H */
