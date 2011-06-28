/*
 * Copyright (c) 2009, 2010 Richard Braun.
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
 * Helper macros.
 */

#ifndef _MACROS_H
#define _MACROS_H

#include <stddef.h>

#define MACRO_BEGIN     ({
#define MACRO_END       })

#define XQUOTE(x)       #x
#define QUOTE(x)        XQUOTE(x)

#define STRLEN(x)       (sizeof(x) - 1)
#define ARRAY_SIZE(x)   (sizeof(x) / sizeof((x)[0]))

#define MIN(a, b)       ((a) < (b) ? (a) : (b))
#define MAX(a, b)       ((a) > (b) ? (a) : (b))

#define P2ALIGNED(x, a) (((x) & ((a) - 1)) == 0)
#define ISP2(x)         P2ALIGNED(x, x)
#define P2ALIGN(x, a)   ((x) & -(a))
#define P2ROUND(x, a)   (-(-(x) & -(a)))
#define P2END(x, a)     (-(~(x) & -(a)))

#define structof(ptr, type, member) \
    ((type *)((char *)ptr - offsetof(type, member)))

#define alignof(x)      __alignof__(x)

#define likely(expr)    __builtin_expect(!!(expr), 1)
#define unlikely(expr)  __builtin_expect(!!(expr), 0)

#define barrier()       asm volatile("" : : : "memory")

#define __noreturn      __attribute__((noreturn))
#define __aligned(x)    __attribute__((aligned(x)))

#define __format_printf(fmt, args) \
    __attribute__((format(printf, fmt, args)))

#endif /* _MACROS_H */
