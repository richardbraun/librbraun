/*
 * Copyright (c) 2018-2019 Richard Braun.
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
#include <limits.h>
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
union mbuf_hdr {
    uint8_t size1;
    uint16_t size2;
    uint32_t size4;
#ifdef __LP64__
    uint64_t size8;
#endif /* __LP64__ */
};

static size_t
mbuf_compute_hdr_size(unsigned int order)
{
    return order / CHAR_BIT;
}

static void *
mbuf_hdr_get_msg_size_addr(union mbuf_hdr *hdr, unsigned int order)
{
    switch (order) {
    case 8:
        return &hdr->size1;
    case 16:
        return &hdr->size2;
    default:
#ifdef __LP64__
        return &hdr->size8;
    case 32:
#endif /* __LP64__ */
        return &hdr->size4;
    }
}

static size_t
mbuf_hdr_get_msg_size(const union mbuf_hdr *hdr, unsigned int order)
{
    switch (order) {
    case 8:
        return hdr->size1;
    case 16:
        return hdr->size2;
    default:
#ifdef __LP64__
        return hdr->size8;
    case 32:
#endif /* __LP64__ */
        return hdr->size4;
    }
}

static void
mbuf_hdr_init(union mbuf_hdr *hdr, unsigned int order, size_t size)
{
    switch (order) {
    case 8:
        assert(size <= UINT8_MAX);
        hdr->size1 = size;
        break;
    case 16:
        assert(size <= UINT16_MAX);
        hdr->size2 = size;
        break;
    default:
#ifdef __LP64__
        hdr->size8 = size;
        break;
    case 32:
        assert(size <= UINT32_MAX);
#endif /* __LP64__ */
        hdr->size4 = size;
    }
}

static size_t
mbuf_hdr_get_total_msg_size(const union mbuf_hdr *hdr, unsigned int order)
{
    return mbuf_compute_hdr_size(order) + mbuf_hdr_get_msg_size(hdr, order);
}

static unsigned int
mbuf_compute_order(size_t max_msg_size)
{
    unsigned int order;

    assert(max_msg_size != 0);
    order = (sizeof(max_msg_size) * CHAR_BIT) - __builtin_clzl(max_msg_size);
    return P2ROUND(order, CHAR_BIT);
}

void
mbuf_init(struct mbuf *mbuf, void *buf, size_t capacity, size_t max_msg_size)
{
    cbuf_init(&mbuf->cbuf, buf, capacity);
    mbuf->max_msg_size = max_msg_size;
    mbuf->order = mbuf_compute_order(max_msg_size);
}

void
mbuf_clear(struct mbuf *mbuf)
{
    return cbuf_clear(&mbuf->cbuf);
}

static void
mbuf_clear_old_msgs(struct mbuf *mbuf, size_t total_size)
{
    union mbuf_hdr hdr;
    void *msg_size_addr;
    size_t hdr_size;

    hdr_size = mbuf_compute_hdr_size(mbuf->order);
    msg_size_addr = mbuf_hdr_get_msg_size_addr(&hdr, mbuf->order);

    do {
        size_t msg_size, size;
        int error __unused;

        size = hdr_size;
        error = cbuf_pop(&mbuf->cbuf, msg_size_addr, &size);
        assert(!error);

        if (size == 0) {
            break;
        }

        msg_size = mbuf_hdr_get_msg_size(&hdr, mbuf->order);
        size = msg_size;
        error = cbuf_pop(&mbuf->cbuf, NULL, &size);
        assert(!error && (size == msg_size));
    } while (cbuf_avail_size(&mbuf->cbuf) < total_size);
}

int
mbuf_push(struct mbuf *mbuf, const void *buf, size_t size, bool erase)
{
    union mbuf_hdr hdr;
    void *msg_size_addr;
    size_t hdr_size, total_size;
    int error __unused;

    if (size > mbuf->max_msg_size) {
        return EINVAL;
    }

    mbuf_hdr_init(&hdr, mbuf->order, size);

    total_size = mbuf_hdr_get_total_msg_size(&hdr, mbuf->order);

    if (total_size > cbuf_avail_size(&mbuf->cbuf)) {
        if (!erase || (total_size > cbuf_capacity(&mbuf->cbuf))) {
            return EMSGSIZE;
        }

        mbuf_clear_old_msgs(mbuf, total_size);
    }

    hdr_size = mbuf_compute_hdr_size(mbuf->order);
    msg_size_addr = mbuf_hdr_get_msg_size_addr(&hdr, mbuf->order);

    error = cbuf_push(&mbuf->cbuf, msg_size_addr, hdr_size, erase);
    assert(!error);
    error = cbuf_push(&mbuf->cbuf, buf, size, erase);
    assert(!error);

    return 0;
}

int
mbuf_pop(struct mbuf *mbuf, void *buf, size_t *sizep)
{
    size_t start;
    int error;

    start = cbuf_start(&mbuf->cbuf);
    error = mbuf_read(mbuf, &start, buf, sizep);

    if (!error) {
        cbuf_set_start(&mbuf->cbuf, start);
    }

    return error;
}

int
mbuf_read(const struct mbuf *mbuf, size_t *indexp, void *buf, size_t *sizep)
{
    union mbuf_hdr hdr;
    void *msg_size_addr;
    size_t hdr_size, msg_size, size;
    int error;

    hdr_size = mbuf_compute_hdr_size(mbuf->order);
    msg_size_addr = mbuf_hdr_get_msg_size_addr(&hdr, mbuf->order);

    size = hdr_size;
    error = cbuf_read(&mbuf->cbuf, *indexp, msg_size_addr, &size);

    if (error) {
        return error;
    } else if (size == 0) {
        return EAGAIN;
    }

    assert(size == hdr_size);

    msg_size = mbuf_hdr_get_msg_size(&hdr, mbuf->order);

    if (msg_size > *sizep) {
        error = EMSGSIZE;
        goto out;
    }

    size = msg_size;
    error = cbuf_read(&mbuf->cbuf, *indexp + hdr_size, buf, &size);

    if (error) {
        goto out;
    }

    assert(size == msg_size);

    *indexp += hdr_size + size;

out:
    *sizep = msg_size;
    return error;
}
