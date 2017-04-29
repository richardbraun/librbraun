/*
 * Copyright (c) 2011-2015 Richard Braun.
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
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#ifndef _CPU_H
#define _CPU_H

#include <sched.h>

#include "macros.h"

/*
 * Maximum number of supported processors.
 */
#define NR_CPUS CONFIG_NR_CPUS

/*
 * Filter invalid values.
 *
 * The maximum number of processors must be a power-of-two in order to avoid
 * divisions.
 */
#if (NR_CPUS < 1) || !ISP2(NR_CPUS)
#error "invalid number of configured processors"
#endif

/*
 * L1 cache line shift and size.
 */
#define CPU_L1_SHIFT    CONFIG_CPU_L1_SHIFT
#define CPU_L1_SIZE     (1 << CPU_L1_SHIFT)

/*
 * Return the ID of the currently running CPU.
 *
 * This implementation uses a glibc-specific function that relies on a
 * Linux-specific system call to obtain the CPU ID. If this function fails
 * (e.g. when run in valgrind), 0 is returned.
 *
 * The returned CPU ID cannot be greater than the maximum number of supported
 * processors.
 */
static inline int
cpu_id(void)
{
#if NR_CPUS == 1
    return 0;
#else
    int id;

    id = sched_getcpu();

    if (id == -1) {
        return 0;
    } else if (id >= NR_CPUS) {
        id &= (NR_CPUS - 1);
    }

    return id;
#endif
}

#endif /* _CPU_H */
