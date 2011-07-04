/*
 * Copyright (c) 2010, 2011 Richard Braun.
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
 * Object caching and general purpose memory allocator.
 */

#ifndef _MEM_H
#define _MEM_H

#include <stddef.h>

/*
 * Backend source of memory for a cache.
 */
struct mem_source {
    void * (*alloc_fn)(size_t);
    void (*free_fn)(void *, size_t);
};

/*
 * Object cache opaque declaration.
 */
struct mem_cache;

/*
 * Type for constructor functions.
 *
 * The pre-constructed state of an object is supposed to include only
 * elements such as e.g. linked lists, locks, reference counters. Therefore
 * constructors are expected to 1) never fail and 2) not need any
 * user-provided data. The first constraint implies that object construction
 * never performs dynamic resource allocation, which also means there is no
 * need for destructors.
 */
typedef void (*mem_cache_ctor_t)(void *);

/*
 * Cache creation flags.
 */
#define MEM_CACHE_VERIFY    0x1 /* Use debugging facilities */

/*
 * Create a cache.
 */
struct mem_cache * mem_cache_create(const char *name, size_t obj_size,
                                    size_t align, mem_cache_ctor_t ctor,
                                    struct mem_source *source, int flags);

/*
 * Destroy a cache.
 */
void mem_cache_destroy(struct mem_cache *cache);

/*
 * Allocate an object from a cache.
 */
void * mem_cache_alloc(struct mem_cache *cache);

/*
 * Release an object to its cache.
 */
void mem_cache_free(struct mem_cache *cache, void *obj);

/*
 * Display internal cache stats on stderr.
 *
 * If cache is NULL, this function displays all managed caches.
 */
void mem_cache_info(struct mem_cache *cache);

/*
 * Set up the memory allocator module.
 */
void mem_setup(void);

/*
 * Allocate size bytes of uninitialized memory.
 */
void * mem_alloc(size_t size);

/*
 * Allocate size bytes of zeroed memory.
 */
void * mem_zalloc(size_t size);

/*
 * Release memory obtained with mem_alloc() or mem_zalloc().
 *
 * The size argument must strictly match the value given at allocation time.
 */
void mem_free(void *ptr, size_t size);

/*
 * Display global memory information on stderr.
 */
void mem_info(void);

#endif /* _MEM_H */
