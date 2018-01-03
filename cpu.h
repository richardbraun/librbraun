/*
 * Copyright (c) 2011-2015 Richard Braun.
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
