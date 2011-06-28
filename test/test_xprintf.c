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
 */

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>

#include "../macros.h"
#include "../xprintf.h"

#define TEST_PRINTF(format, ...)                        \
MACRO_BEGIN                                             \
    char stra[256], strb[256];                          \
    int la, lb;                                         \
    la = snprintf(stra, 256, format, ## __VA_ARGS__);   \
    printf(" printf: %s", stra);                        \
    lb = xsnprintf(strb, 256, format, ## __VA_ARGS__);  \
    xprintf("xprintf: %s", stra);                       \
    assert(la == lb);                                   \
    assert(strcmp(stra, strb) == 0);                    \
MACRO_END

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

#define FORMAT "%c"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 'a');
#undef FORMAT

#define FORMAT "%8c"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 'a');
#undef FORMAT

#define FORMAT "%-8c"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 'a');
#undef FORMAT

#define FORMAT "%.s"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, "12345");
#undef FORMAT

#define FORMAT "%.3s"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, "12345");
#undef FORMAT

#define FORMAT "%4.3s"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, "12345");
#undef FORMAT

#define FORMAT "%-4.3s"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, "12345");
#undef FORMAT

#define FORMAT "%3.4s"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, "12345");
#undef FORMAT

#define FORMAT "%-3.4s"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, "12345");
#undef FORMAT

#define FORMAT "%#o"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%#x"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%#X"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%08d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%-8d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%-08d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%0-8d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "% d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%+d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%+ d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%12d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%*d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 12, 123);
#undef FORMAT

#define FORMAT "%.12d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%.012d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%.*d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 12, 123);
#undef FORMAT

#define FORMAT "%.d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%.*d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, -12, 123);
#undef FORMAT

#define FORMAT "%.4d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%5.4d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%4.5d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 0);
#undef FORMAT

#define FORMAT "%.0d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 0);
#undef FORMAT

#define FORMAT "%.0o"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 0);
#undef FORMAT

#define FORMAT "%.0x"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 0);
#undef FORMAT

#define FORMAT "%1.0d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 0);
#undef FORMAT

#define FORMAT "%08.0d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, -123);
#undef FORMAT

#define FORMAT "%08d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, -123);
#undef FORMAT

#define FORMAT "%08d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%8d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%8d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, -123);
#undef FORMAT

#define FORMAT "%.8d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, -123);
#undef FORMAT

#define FORMAT "%.80d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, -123);
#undef FORMAT

#define FORMAT "%-80d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, -123);
#undef FORMAT

#define FORMAT "%80d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, -123);
#undef FORMAT

#define FORMAT "%80.40d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, -123);
#undef FORMAT

#define FORMAT "%-+80.40d"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%+x"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, -123);
#undef FORMAT

#define FORMAT "%#x"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%#o"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 123);
#undef FORMAT

#define FORMAT "%p"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, "123");
#undef FORMAT

#define FORMAT "%#lx"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 0xdeadbeefL);
#undef FORMAT

#define FORMAT "%zd"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, (size_t)-123);
#undef FORMAT

#define FORMAT "%#llx"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 0xdeadbeefbadcafeLL);
#undef FORMAT

#define FORMAT "%llo"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT, 0xffffffffffffffffLL);
#undef FORMAT

#define FORMAT "%%"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT);
#undef FORMAT

#define FORMAT "%y"
    TEST_PRINTF("%s: '" FORMAT "'\n", FORMAT);
#undef FORMAT

    char stra[10], strb[10];
    int la, lb;
    la = snprintf(stra, sizeof(stra), "%s", "123456789a");
    printf(" printf: %s\n", stra);
    lb = xsnprintf(strb, sizeof(strb), "%s", "123456789a");
    xprintf("xprintf: %s\n", stra);
    assert(la == lb);
    assert(strncmp(stra, strb, 10) == 0);

#define FORMAT "12%n3%#08x4%n5"
    int lc, ld;
    sprintf(stra, FORMAT, &la, 123, &lb);
    printf(" printf: la: %d, lb: %d\n", la, lb);
    xsprintf(strb, FORMAT, &lc, 123, &ld);
    xprintf("xprintf: lc: %d, ld: %d\n", lc, ld);
    assert(la == lc);
    assert(lb == ld);
#undef FORMAT

    la = snprintf(NULL, 0, "%s", "123");
    printf(" printf: %d\n", la);
    lb = xsnprintf(NULL, 0, "%s", "123");
    xprintf("xprintf: %d\n", lb);
    assert(la == lb);

    return 0;
}
