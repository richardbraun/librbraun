/*
 * Copyright (c) 2012-2015 Richard Braun.
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
 * Atomic operations.
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#ifndef _X86_ATOMIC_H
#define _X86_ATOMIC_H

#define ATOMIC_ADD(ptr, delta)      \
    asm volatile("lock add %1, %0"  \
                 : "+m" (*(ptr))    \
                 : "r" (delta))

#define ATOMIC_FETCHADD(ptr, oldval, delta)     \
    asm volatile("lock xadd %1, %0"             \
                 : "+m" (*(ptr)), "=r" (oldval) \
                 : "1" (delta)                  \
                 : "memory")

#define ATOMIC_AND(ptr, bits)       \
    asm volatile("lock and %1, %0"  \
                 : "+m" (*(ptr))    \
                 : "r" (bits))

#define ATOMIC_OR(ptr, bits)        \
    asm volatile("lock or %1, %0"   \
                 : "+m" (*(ptr))    \
                 : "r" (bits))

/* The xchg instruction doesn't need a lock prefix */
#define ATOMIC_SWAP(ptr, oldval, newval)        \
    asm volatile("xchg %1, %0"                  \
                 : "+m" (*(ptr)), "=r" (oldval) \
                 : "1" (newval)                 \
                 : "memory")

#define ATOMIC_CAS(ptr, oldval, predicate, newval)  \
    asm volatile("lock cmpxchg %3, %0"              \
                 : "+m" (*(ptr)), "=a" (oldval)     \
                 : "1" (predicate), "r" (newval)    \
                 : "memory")

static inline void
atomic_add_uint(volatile unsigned int *ptr, int delta)
{
    ATOMIC_ADD(ptr, delta);
}

/*
 * Implies a full memory barrier.
 */
static inline unsigned int
atomic_fetchadd_uint(volatile unsigned int *ptr, int delta)
{
    unsigned int oldval;

    ATOMIC_FETCHADD(ptr, oldval, delta);
    return oldval;
}

static inline void
atomic_and_uint(volatile unsigned int *ptr, unsigned int bits)
{
    ATOMIC_AND(ptr, bits);
}

static inline void
atomic_or_uint(volatile unsigned int *ptr, unsigned int bits)
{
    ATOMIC_OR(ptr, bits);
}

/*
 * Implies a full memory barrier.
 */
static inline unsigned int
atomic_swap_uint(volatile unsigned int *ptr, unsigned int newval)
{
    unsigned int oldval;

    ATOMIC_SWAP(ptr, oldval, newval);
    return oldval;
}

/*
 * Implies a full memory barrier.
 */
static inline unsigned int
atomic_cas_uint(volatile unsigned int *ptr, unsigned int predicate,
                unsigned int newval)
{
    unsigned int oldval;

    ATOMIC_CAS(ptr, oldval, predicate, newval);
    return oldval;
}

static inline void
atomic_add_ulong(volatile unsigned long *ptr, long delta)
{
    ATOMIC_ADD(ptr, delta);
}

/*
 * Implies a full memory barrier.
 */
static inline unsigned long
atomic_fetchadd_ulong(volatile unsigned long *ptr, long delta)
{
    unsigned long oldval;

    ATOMIC_FETCHADD(ptr, oldval, delta);
    return oldval;
}

static inline void
atomic_and_ulong(volatile unsigned long *ptr, unsigned long bits)
{
    ATOMIC_AND(ptr, bits);
}

static inline void
atomic_or_ulong(volatile unsigned long *ptr, unsigned long bits)
{
    ATOMIC_OR(ptr, bits);
}

/*
 * Implies a full memory barrier.
 */
static inline unsigned long
atomic_swap_ulong(volatile unsigned long *ptr, unsigned long newval)
{
    unsigned long oldval;

    ATOMIC_SWAP(ptr, oldval, newval);
    return oldval;
}

/*
 * Implies a full memory barrier.
 */
static inline unsigned long
atomic_cas_ulong(volatile unsigned long *ptr, unsigned long predicate,
                 unsigned long newval)
{
    unsigned long oldval;

    ATOMIC_CAS(ptr, oldval, predicate, newval);
    return oldval;
}

#endif /* _X86_ATOMIC_H */
