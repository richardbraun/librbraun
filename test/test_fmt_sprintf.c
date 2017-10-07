/*
 * Copyright (c) 2010-2017 Richard Braun.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wformat"

#include "../check.h"
#include "../fmt.h"
#include "../macros.h"

#define TEST_SPRINTF(format, ...)                                    \
MACRO_BEGIN                                                         \
    char stra[256], strb[256];                                      \
    int la, lb;                                                     \
    la = snprintf(stra, sizeof(stra), format, ## __VA_ARGS__);      \
    lb = fmt_snprintf(strb, sizeof(strb), format, ## __VA_ARGS__);  \
    check(la == lb);                                                \
    check(strcmp(stra, strb) == 0);                                 \
MACRO_END

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

#define FORMAT "%c"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 'a');
#undef FORMAT

#define FORMAT "%8c"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 'a');
#undef FORMAT

#define FORMAT "%-8c"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 'a');
#undef FORMAT

#define FORMAT "%.s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT

#define FORMAT "%.3s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT

#define FORMAT "%4.3s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT

#define FORMAT "%-4.3s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT

#define FORMAT "%3.4s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT

#define FORMAT "%-3.4s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT

#define FORMAT "%#o"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%#x"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%#X"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%08d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%-8d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%-08d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%0-8d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "% d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%+d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%+ d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%12d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%*d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 12, 123);
#undef FORMAT

#define FORMAT "%.12d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%.012d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%.*d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 12, 123);
#undef FORMAT

#define FORMAT "%.d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%.*d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -12, 123);
#undef FORMAT

#define FORMAT "%.4d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%5.4d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%4.5d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0);
#undef FORMAT

#define FORMAT "%.0d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0);
#undef FORMAT

#define FORMAT "%.0o"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0);
#undef FORMAT

#define FORMAT "%.0x"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0);
#undef FORMAT

#define FORMAT "%1.0d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0);
#undef FORMAT

#define FORMAT "%08.0d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT

#define FORMAT "%08d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT

#define FORMAT "%08d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%8d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%8d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT

#define FORMAT "%.8d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT

#define FORMAT "%.80d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT

#define FORMAT "%-80d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT

#define FORMAT "%80d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT

#define FORMAT "%80.40d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT

#define FORMAT "%-+80.40d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%+x"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT

#define FORMAT "%#x"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%#o"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT

#define FORMAT "%p"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "123");
#undef FORMAT

#define FORMAT "%#lx"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0xdeadbeefL);
#undef FORMAT

#define FORMAT "%zd"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, (size_t)-123);
#undef FORMAT

#define FORMAT "%#llx"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0xdeadbeefbadcafeLL);
#undef FORMAT

#define FORMAT "%llo"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0xffffffffffffffffLL);
#undef FORMAT

#define FORMAT "%%"
    TEST_SPRINTF("%s: " FORMAT, FORMAT);
#undef FORMAT

#define FORMAT "%y"
    TEST_SPRINTF("%s: " FORMAT, FORMAT);
#undef FORMAT

    char stra[10], strb[10];
    int la, lb;
    la = snprintf(stra, sizeof(stra), "%s", "123456789a");
    lb = fmt_snprintf(strb, sizeof(strb), "%s", "123456789a");
    check(la == lb);
    check(strncmp(stra, strb, 10) == 0);

#define FORMAT "12%n3%#08x4%n5"
    int lc, ld;
    sprintf(stra, FORMAT, &la, 123, &lb);
    fmt_sprintf(strb, FORMAT, &lc, 123, &ld);
    check(la == lc);
    check(lb == ld);
#undef FORMAT

    la = snprintf(NULL, 0, "%s", "123");
    lb = fmt_snprintf(NULL, 0, "%s", "123");
    check(la == lb);

    return 0;
}
