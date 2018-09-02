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
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "macros.h"
#include "mbuf.h"

/*
 * Message header.
 *
 * The size denotes the size of the data, without the header.
 */
struct mbuf_hdr {
    uint32_t size;
    char data[];
};

static int
mbuf_hdr_init(struct mbuf_hdr *hdr, size_t size)
{
    hdr->size = size;

    if (hdr->size != size) {
        return EMSGSIZE;
    }

    return 0;
}

static size_t
mbuf_hdr_total_size(const struct mbuf_hdr *hdr)
{
    return sizeof(*hdr) + hdr->size;
}

static size_t
mbuf_hdr_msg_size(const struct mbuf_hdr *hdr)
{
    return hdr->size;
}

void
mbuf_init(struct mbuf *mbuf, void *buf, size_t capacity)
{
    cbuf_init(&mbuf->cbuf, buf, capacity);
}

void
mbuf_clear(struct mbuf *mbuf)
{
    return cbuf_clear(&mbuf->cbuf);
}

static void
mbuf_clear_old_msgs(struct mbuf *mbuf, size_t total_size)
{
    struct mbuf_hdr hdr;
    size_t size;
    __unused int error;

    do {
        size = sizeof(hdr);
        error = cbuf_pop(&mbuf->cbuf, &hdr, &size);
        assert(!error);

        if (size == 0) {
            break;
        }

        size = mbuf_hdr_msg_size(&hdr);
        error = cbuf_pop(&mbuf->cbuf, NULL, &size);
        assert(!error && (size == mbuf_hdr_msg_size(&hdr)));
    } while (cbuf_avail_size(&mbuf->cbuf) < total_size);
}

int
mbuf_push(struct mbuf *mbuf, const void *buf, size_t size, bool erase)
{
    struct mbuf_hdr hdr;
    size_t total_size;
    int error;

    error = mbuf_hdr_init(&hdr, size);

    if (error) {
        return error;
    }

    total_size = mbuf_hdr_total_size(&hdr);

    if (total_size > cbuf_avail_size(&mbuf->cbuf)) {
        if (!erase || (total_size > cbuf_capacity(&mbuf->cbuf))) {
            return EMSGSIZE;
        }

        mbuf_clear_old_msgs(mbuf, total_size);
    }

    error = cbuf_push(&mbuf->cbuf, &hdr, sizeof(hdr), erase);
    assert(!error);
    error = cbuf_push(&mbuf->cbuf, buf, size, erase);
    assert(!error);

    return 0;
}

int
mbuf_pop(struct mbuf *mbuf, void *buf, size_t *sizep)
{
    struct mbuf_hdr hdr;
    size_t start, size;
    int error;

    start = cbuf_start(&mbuf->cbuf);

    size = sizeof(hdr);
    error = cbuf_read(&mbuf->cbuf, start, &hdr, &size);
    assert(!error);

    if (size == 0) {
        return EAGAIN;
    }

    assert(size == sizeof(hdr));
    size = mbuf_hdr_msg_size(&hdr);

    if (size > *sizep) {
        error = EMSGSIZE;
        goto out;
    }

    cbuf_set_start(&mbuf->cbuf, start + sizeof(hdr));
    error = cbuf_pop(&mbuf->cbuf, buf, &size);
    assert(!error && (size == mbuf_hdr_msg_size(&hdr)));

out:
    *sizep = size;
    return error;
}

size_t
mbuf_avail_size(const struct mbuf *mbuf)
{
    return cbuf_avail_size(&mbuf->cbuf);
}
