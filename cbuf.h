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
 * Circular character buffer.
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#ifndef _CBUF_H
#define _CBUF_H

/*
 * Circular buffer descriptor.
 *
 * The buffer capacity must be a power-of-two. Indexes are absolute values
 * which can overflow. Their difference cannot exceed the capacity.
 */
struct cbuf {
    char *buf;
    unsigned long capacity;
    unsigned long start;
    unsigned long end;
};

static inline unsigned long
cbuf_capacity(const struct cbuf *cbuf)
{
    return cbuf->capacity;
}

static inline unsigned long
cbuf_start(const struct cbuf *cbuf)
{
    return cbuf->start;
}

static inline unsigned long
cbuf_end(const struct cbuf *cbuf)
{
    return cbuf->end;
}

static inline unsigned long
cbuf_size(const struct cbuf *cbuf)
{
    return cbuf->end - cbuf->start;
}

static inline void
cbuf_clear(struct cbuf *cbuf)
{
    cbuf->start = cbuf->end;
}

/*
 * Initialize a circular buffer.
 *
 * The descriptor is set to use the given buffer for storage. Capacity
 * must be a power-of-two.
 */
void cbuf_init(struct cbuf *cbuf, char *buf, unsigned long capacity);

/*
 * Append a byte to a circular buffer.
 *
 * The end index is incremented. If the buffer is full, the oldest byte
 * is overwritten and the start index is updated accordingly.
 */
void cbuf_push(struct cbuf *cbuf, char byte);

/*
 * Read a byte from a circular buffer.
 *
 * If the buffer is empty, ERR_AGAIN is returned. Otherwise, the oldest
 * byte is stored at the bytep address, the start index is incremented,
 * and 0 is returned.
 */
int cbuf_pop(struct cbuf *cbuf, char *bytep);

/*
 * Read a byte at a specific location.
 *
 * If the given index is outside buffer boundaries, ERR_INVAL is returned.
 * Otherwise the byte is stored at the bytep address and 0 is returned.
 * The buffer isn't changed by this operation.
 */
int cbuf_read(const struct cbuf *cbuf, unsigned long index, char *bytep);

#endif /* _CBUF_H */