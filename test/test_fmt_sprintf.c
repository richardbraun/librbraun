/*
 * Copyright (c) 2010-2018 Richard Braun.
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
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wformat"

#include <check.h>
#include <fmt.h>
#include <macros.h>

#define TEST_SPRINTF(format, ...)                                   \
MACRO_BEGIN                                                         \
    char stra[256], strb[256];                                      \
    int la, lb;                                                     \
                                                                    \
    la = snprintf(stra, sizeof(stra), format, ## __VA_ARGS__);      \
    lb = fmt_snprintf(strb, sizeof(strb), format, ## __VA_ARGS__);  \
    check(la == lb);                                                \
    check(strcmp(stra, strb) == 0);                                 \
MACRO_END

static void
test_1(void)
{
#define FORMAT "%c"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 'a');
#undef FORMAT
}

static void
test_2(void)
{
#define FORMAT "%8c"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 'a');
#undef FORMAT
}

static void
test_3(void)
{
#define FORMAT "%-8c"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 'a');
#undef FORMAT
}

static void
test_4(void)
{
#define FORMAT "%.s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT
}

static void
test_5(void)
{
#define FORMAT "%.3s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT
}

static void
test_6(void)
{
#define FORMAT "%4.3s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT
}

static void
test_7(void)
{
#define FORMAT "%-4.3s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT
}

static void
test_8(void)
{
#define FORMAT "%3.4s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT
}

static void
test_9(void)
{
#define FORMAT "%-3.4s"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "12345");
#undef FORMAT
}

static void
test_10(void)
{
#define FORMAT "%#o"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_11(void)
{
#define FORMAT "%#x"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_12(void)
{
#define FORMAT "%#X"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_13(void)
{
#define FORMAT "%08d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_14(void)
{
#define FORMAT "%-8d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_15(void)
{
#define FORMAT "%-08d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_16(void)
{
#define FORMAT "%0-8d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_17(void)
{
#define FORMAT "% d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_18(void)
{
#define FORMAT "%+d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_19(void)
{
#define FORMAT "%+ d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_20(void)
{
#define FORMAT "%12d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_21(void)
{
#define FORMAT "%*d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 12, 123);
#undef FORMAT
}

static void
test_22(void)
{
#define FORMAT "%.12d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_23(void)
{
#define FORMAT "%.012d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_24(void)
{
#define FORMAT "%.*d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 12, 123);
#undef FORMAT
}

static void
test_25(void)
{
#define FORMAT "%.d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_26(void)
{
#define FORMAT "%.*d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -12, 123);
#undef FORMAT
}

static void
test_27(void)
{
#define FORMAT "%.4d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_28(void)
{
#define FORMAT "%5.4d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_29(void)
{
#define FORMAT "%4.5d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_30(void)
{
#define FORMAT "%d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0);
#undef FORMAT
}

static void
test_31(void)
{
#define FORMAT "%.0d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0);
#undef FORMAT
}

static void
test_32(void)
{
#define FORMAT "%.0o"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0);
#undef FORMAT
}

static void
test_33(void)
{
#define FORMAT "%.0x"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0);
#undef FORMAT
}

static void
test_34(void)
{
#define FORMAT "%1.0d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0);
#undef FORMAT
}

static void
test_35(void)
{
#define FORMAT "%08.0d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT
}

static void
test_36(void)
{
#define FORMAT "%08d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT
}

static void
test_37(void)
{
#define FORMAT "%08d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_38(void)
{
#define FORMAT "%8d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_39(void)
{
#define FORMAT "%8d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT
}

static void
test_40(void)
{
#define FORMAT "%.8d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT
}

static void
test_41(void)
{
#define FORMAT "%.80d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT
}

static void
test_42(void)
{
#define FORMAT "%-80d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT
}

static void
test_43(void)
{
#define FORMAT "%80d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT
}

static void
test_44(void)
{
#define FORMAT "%80.40d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT
}

static void
test_45(void)
{
#define FORMAT "%-+80.40d"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_46(void)
{
#define FORMAT "%+x"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, -123);
#undef FORMAT
}

static void
test_47(void)
{
#define FORMAT "%#x"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_48(void)
{
#define FORMAT "%#o"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 123);
#undef FORMAT
}

static void
test_49(void)
{
#define FORMAT "%p"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, "123");
#undef FORMAT
}

static void
test_50(void)
{
#define FORMAT "%#lx"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0xdeadbeefL);
#undef FORMAT
}

static void
test_51(void)
{
#define FORMAT "%zd"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, (size_t)-123);
#undef FORMAT
}

static void
test_52(void)
{
#define FORMAT "%#llx"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0xdeadbeefbadcafeLL);
#undef FORMAT
}

static void
test_53(void)
{
#define FORMAT "%llo"
    TEST_SPRINTF("%s: " FORMAT, FORMAT, 0xffffffffffffffffLL);
#undef FORMAT
}

static void
test_54(void)
{
#define FORMAT "%%"
    TEST_SPRINTF("%s: " FORMAT, FORMAT);
#undef FORMAT
}

static void
test_55(void)
{
#define FORMAT "%y"
    TEST_SPRINTF("%s: " FORMAT, FORMAT);
#undef FORMAT
}

static void
test_56(void)
{
    char stra[11], strb[11];
    int la, lb;

    la = snprintf(stra, sizeof(stra), "%s", "123456789a");
    lb = fmt_snprintf(strb, sizeof(strb), "%s", "123456789a");
    check(la == lb);
    check(strncmp(stra, strb, 10) == 0);
}

static void
test_57(void)
{
    char stra[256], strb[256];                                      \
    int la, lb, lc, ld;

#define FORMAT "12%n3%#08x4%n5"
    sprintf(stra, FORMAT, &la, 123, &lb);
    fmt_sprintf(strb, FORMAT, &lc, 123, &ld);
    check(la == lc);
    check(lb == ld);
#undef FORMAT
}

static void
test_58(void)
{
    int la, lb;

    la = snprintf(NULL, 0, "%s", "123");
    lb = fmt_snprintf(NULL, 0, "%s", "123");
    check(la == lb);
}

int
main(void)
{
    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
    test_6();
    test_7();
    test_8();
    test_9();
    test_10();
    test_11();
    test_12();
    test_13();
    test_14();
    test_15();
    test_16();
    test_17();
    test_18();
    test_19();
    test_20();
    test_21();
    test_22();
    test_23();
    test_24();
    test_25();
    test_26();
    test_27();
    test_28();
    test_29();
    test_30();
    test_31();
    test_32();
    test_33();
    test_34();
    test_35();
    test_36();
    test_37();
    test_38();
    test_39();
    test_40();
    test_41();
    test_42();
    test_43();
    test_44();
    test_45();
    test_46();
    test_47();
    test_48();
    test_49();
    test_50();
    test_51();
    test_52();
    test_53();
    test_54();
    test_55();
    test_56();
    test_57();
    test_58();

    return EXIT_SUCCESS;
}
