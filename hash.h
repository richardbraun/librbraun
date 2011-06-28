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
 * Hash functions for integers and strings.
 *
 * Integer hashing follows Thomas Wang's paper about his 32/64-bits mix
 * functions :
 * - http://www.concentric.net/~Ttwang/tech/inthash.htm
 *
 * String hashing uses a variant of the djb2 algorithm with k=31, as in
 * the implementation of the hashCode() method of the Java String class :
 * - http://www.javamex.com/tutorials/collections/hash_function_technical.shtml
 *
 * Note that this algorithm isn't suitable to obtain usable 64-bits hashes
 * and is expected to only serve as an array index producer.
 *
 * These functions all have a bits parameter that indicates the number of
 * relevant bits the caller is interested in. When returning a hash, its
 * value must be truncated so that it can fit in the requested bit size.
 * It can be used by the implementation to select high or low bits, depending
 * on their relative randomness. To get complete, unmasked hashes, use the
 * HASH_ALLBITS macro.
 */

#ifndef _HASH_H
#define _HASH_H

#include <stdint.h>

#ifdef __LP64__
#define HASH_ALLBITS 64
#define hash_long(n, bits) hash_int64(n, bits)
#else /* __LP64__ */
#define HASH_ALLBITS 32
#define hash_long(n, bits) hash_int32(n, bits)
#endif

static inline uint32_t hash_int32(uint32_t n, unsigned int bits)
{
    uint32_t hash;

    hash = n;
    hash = ~hash + (hash << 15);
    hash ^= (hash >> 12);
    hash += (hash << 2);
    hash ^= (hash >> 4);
    hash += (hash << 3) + (hash << 11);
    hash ^= (hash >> 16);

    return hash >> (32 - bits);
}

static inline uint64_t hash_int64(uint64_t n, unsigned int bits)
{
    uint64_t hash;

    hash = n;
    hash = ~hash + (hash << 21);
    hash ^= (hash >> 24);
    hash += (hash << 3) + (hash << 8);
    hash ^= (hash >> 14);
    hash += (hash << 2) + (hash << 4);
    hash ^= (hash >> 28);
    hash += (hash << 31);

    return hash >> (64 - bits);
}

static inline unsigned long hash_ptr(const void *ptr, unsigned int bits)
{
    return hash_long((unsigned long)ptr, bits);
}

static inline unsigned long hash_str(const char *str, unsigned int bits)
{
    unsigned long hash;
    char c;

    for (hash = 0; (c = *str) != '\0'; str++)
        hash = ((hash << 5) - hash) + c;

    return hash & ((1 << bits) - 1);
}

#endif /* _HASH_H */
