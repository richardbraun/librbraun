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
 * Memory barriers.
 *
 * Keep in mind that memory barriers only act on the ordering of loads and
 * stores between internal processor queues and their caches. In particular,
 * it doesn't imply a store is complete after the barrier has completed, only
 * that other processors will see a new value thanks to the cache coherency
 * protocol. Memory barriers alone aren't suitable for device communication.
 *
 * The x86 architectural memory model (total-store ordering) already enforces
 * strong ordering for almost every access. The only exception is that stores
 * can be reordered after loads. As a result, load and store memory barriers
 * are simple compiler barriers whereas full memory barriers must generate
 * a barrier instruction.
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#ifndef _X86_MB_H
#define _X86_MB_H

#include "../macros.h"

#ifdef __LP64__

static inline void
mb_sync(void)
{
    asm volatile("mfence" : : : "memory");
}

#else /* __LP64__ */

static inline void
mb_sync(void)
{
    asm volatile("lock addl $0, 0(%%esp)" : : : "memory");
}

#endif /* __LP64__ */

static inline void
mb_load(void)
{
    barrier();
}

static inline void
mb_store(void)
{
    barrier();
}

#endif /* _X86_MB_H */
