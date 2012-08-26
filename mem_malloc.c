/*
 * Copyright (c) 2010 Richard Braun.
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
 * Libc-compatible malloc() functions implementation.
 *
 * Keep in mind this code is mainly used for tests. Also, initialization is
 * not thread-safe.
 */

#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "mem.h"
#include "macros.h"
#include "mem_malloc.h"

struct btag {
    void *addr;
    size_t size;
} __aligned(8);

void *
malloc(size_t size)
{
    struct btag *btag;

    mem_setup();

    size += sizeof(*btag);
    btag = mem_alloc(size);

    if (btag == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    btag->addr = btag;
    btag->size = size;

    return btag + 1;
}

void *
calloc(size_t nmemb, size_t size)
{
    size_t bytes;
    void *buf;

    mem_setup();

    bytes = nmemb * size;
    buf = malloc(bytes);

    if (buf == NULL)
        return NULL;

    memset(buf, 0, bytes);

    return buf;
}

void *
realloc(void *ptr, size_t size)
{
    struct btag *btag;
    size_t old_size;
    char *buf;

    mem_setup();

    if (ptr == NULL)
        return malloc(size);
    else if (size == 0) {
        free(ptr);
        return NULL;
    }

    buf = malloc(size);

    if (buf == NULL)
        return NULL;

    btag = (struct btag *)ptr - 1;
    old_size = btag->size - sizeof(*btag);
    memcpy(buf, ptr, MIN(old_size, size));
    mem_free(btag->addr, btag->size);

    return buf;
}

int
posix_memalign(void **ptr, size_t align, size_t size)
{
    struct btag *btag;
    char *buf;

    mem_setup();

    if (!ISP2(align))
        return EINVAL;

    size += sizeof(*btag);
    size = P2ROUND(size, align * 2);
    buf = mem_alloc(size);

    if (buf == NULL)
        return ENOMEM;

    btag = (struct btag *)P2ROUND((unsigned long)buf + sizeof(*btag), align)
           - 1;
    btag->addr = buf;
    btag->size = size;

    *ptr = btag + 1;
    return 0;
}

void
free(void *ptr)
{
    struct btag *btag;

    mem_setup();

    if (ptr == NULL)
        return;

    btag = (struct btag *)ptr - 1;
    mem_free(btag->addr, btag->size);
}
