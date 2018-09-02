/*
 * Copyright (c) 2018 Richard Braun.
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
 * FIFO message buffer.
 */

#ifndef MBUF_H
#define MBUF_H

#include <stdbool.h>
#include <stddef.h>

#include "cbuf.h"

/*
 * Message buffer.
 *
 * Message buffers are built on top of circular byte buffers. They provide
 * discrete message transfer from a producer to a consumer.
 */
struct mbuf {
    struct cbuf cbuf;
};

/*
 * Initialize a message buffer.
 *
 * The descriptor is set to use the given buffer for storage. Capacity
 * must be a power-of-two.
 */
void mbuf_init(struct mbuf *mbuf, void *buf, size_t capacity);

/*
 * Clear a message buffer.
 */
void mbuf_clear(struct mbuf *mbuf);

/*
 * Push a message to a message buffer.
 *
 * If the message doesn't fit in the message buffer, either because it is
 * larger than the capacity, or because the function isn't allowed to erase
 * old messages and the message buffer doesn't have enough available memory
 * for the new message, EMSGSIZE is returned.
 */
int mbuf_push(struct mbuf *mbuf, const void *buf, size_t size, bool erase);

/*
 * Pop a message from a message buffer.
 *
 * On entry, the sizep argument points to the size of the output buffer.
 * On return, it is updated to the size of the message. If the message
 * doesn't fit in the output buffer, it is not popped, EMSGSIZE is
 * returned, but the sizep argument is updated nonetheless to let the
 * user know the message size, to potentially retry with a larger buffer.
 *
 * If the buffer is empty, EAGAIN is returned, and the size of the output
 * buffer is unmodified.
 *
 * The output buffer may be NULL, in which case this function acts as if
 * it wasn't, but without writing output data.
 */
int mbuf_pop(struct mbuf *mbuf, void *buf, size_t *sizep);

/*
 * Get the number of availabe bytes in a message buffer.
 */
size_t mbuf_avail_size(const struct mbuf *mbuf);

#endif /* MBUF_H */
