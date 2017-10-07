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
#include <stdlib.h>
#include <string.h>

#pragma GCC diagnostic ignored "-Wformat"

#include "../check.h"
#include "../fmt.h"
#include "../macros.h"

#define TEST_STR_SIZE 64

#define TEST_INT            123
#define TEST_INT64          1099511627775
#define TEST_INT_OCTAL      0123
#define TEST_INT_HEX        0x123
#define TEST_INT_NEGATIVE   -888
#define TEST_INT_TOO_LARGE  894321568476132169879463132164984654
#define TEST_STR            "test"

static void
test_1(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(TEST_INT)
#define FORMAT "%d"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_2(void)
{
    int reta, retb;

#define STRING "abc"
#define FORMAT "abc"
    reta = sscanf(STRING, FORMAT);
    retb = fmt_sscanf(STRING, FORMAT);
    check(reta == retb);
#undef FORMAT
#undef STRING
}

static void
test_3(void)
{
    int reta, retb;
    int ia, ib;

#define STRING "ab" QUOTE(TEST_INT) "c"
#define FORMAT "ab%dc"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_4(void)
{
    int reta, retb;
    int ia, ib;

#define STRING "ab" QUOTE(TEST_INT) "c"
#define FORMAT "az%dc"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
#undef FORMAT
#undef STRING
}

static void
test_5(void)
{
    int reta, retb;
    int ia, ib;

#define STRING "abc"
#define FORMAT "abc%d"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
#undef FORMAT
#undef STRING
}

static void
test_6(void)
{
    int reta, retb;
    int ia, ib;
    int ja, jb;

#define STRING "a" QUOTE(TEST_INT_TOO_LARGE) "b" QUOTE(TEST_INT) "c"
#define FORMAT "a%db%dc"
    reta = sscanf(STRING, FORMAT, &ia, &ja);
    retb = fmt_sscanf(STRING, FORMAT, &ib, &jb);
    check(reta == retb);
    check(ja == jb);
#undef FORMAT
#undef STRING
}

static void
test_7(void)
{
    unsigned int ia, ib;
    int reta, retb;

#define STRING QUOTE(TEST_INT_NEGATIVE)
#define FORMAT "%u"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_8(void)
{
    unsigned int ia, ib;
    int reta, retb;

#define STRING "    " QUOTE(TEST_INT)
#define FORMAT "%u"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_9(void)
{
    int reta, retb;
    int ia, ib;

#define STRING "   " QUOTE(TEST_INT_NEGATIVE)
#define FORMAT "%d"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_10(void)
{
    int reta, retb;
    int ia, ib;

#define STRING "a%b" QUOTE(TEST_INT)
#define FORMAT "a%%b%d"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_11(void)
{
    int reta, retb;
    char ia, ib;

#define STRING QUOTE(TEST_INT)
#define FORMAT "%hhd"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_12(void)
{
    unsigned short ia, ib;
    int reta, retb;

#define STRING QUOTE(TEST_INT)
#define FORMAT "%hu"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_13(void)
{
    int reta, retb;
    long ia, ib;

#define STRING QUOTE(TEST_INT)
#define FORMAT "%ld"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_14(void)
{
    unsigned long long ia, ib;
    int reta, retb;

#define STRING QUOTE(TEST_INT)
#define FORMAT "%llu"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_15(void)
{
    unsigned long long ia, ib;
    int reta, retb;

#define STRING ""
#define FORMAT "%llu"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
#undef FORMAT
#undef STRING
}

static void
test_16(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(TEST_INT_OCTAL)
#define FORMAT "%i"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_17(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(TEST_INT_OCTAL) "abc"
#define FORMAT "%iabc"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_18(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(TEST_INT_HEX)
#define FORMAT "%i"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_19(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(TEST_INT_HEX) "abcxyz"
#define FORMAT "%iabcxyz"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_20(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(TEST_INT)
#define FORMAT "%o"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_21(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(TEST_INT)
#define FORMAT "%x"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_22(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(TEST_INT_HEX)
#define FORMAT "%x"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_23(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(-TEST_INT_HEX)
#define FORMAT "%x"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_24(void)
{
    int reta, retb;
    int ia, ib;

#define STRING "a" QUOTE(TEST_INT_HEX) "b"
#define FORMAT "a%ob"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_25(void)
{
    int reta, retb;
    int ia, ib;
    int ja, jb;

#define STRING "x" QUOTE(TEST_INT_OCTAL) "y" QUOTE(TEST_INT_HEX) "z"
#define FORMAT "x%oy%xz"
    reta = sscanf(STRING, FORMAT, &ia, &ja);
    retb = fmt_sscanf(STRING, FORMAT, &ib, &jb);
    check(reta == retb);
    check(ia == ib);
    check(ja == jb);
#undef FORMAT
#undef STRING
}

static void
test_26(void)
{
    int reta, retb;
    int ia, ib;
    int ja, jb;

#define STRING "x" QUOTE(TEST_INT_HEX) "y" QUOTE(TEST_INT_OCTAL) "z"
#define FORMAT "x%xy%oz"
    reta = sscanf(STRING, FORMAT, &ia, &ja);
    retb = fmt_sscanf(STRING, FORMAT, &ib, &jb);
    check(reta == retb);
    check(ia == ib);
    check(ja == jb);
#undef FORMAT
#undef STRING
}

static void
test_27(void)
{
    int reta, retb;
    int ia, ib;
    int ja, jb;

#define STRING QUOTE(TEST_INT) " " QUOTE(TEST_INT)
#define FORMAT "    %d    %d   "
    reta = sscanf(STRING, FORMAT, &ia, &ja);
    retb = fmt_sscanf(STRING, FORMAT, &ib, &jb);
    check(reta == retb);
    check(ia == ib);
    check(ja == jb);
#undef FORMAT
#undef STRING
}

static void
test_28(void)
{
    int reta, retb;
    void *ia, *ib;

#define STRING QUOTE(TEST_INT_HEX)
#define FORMAT "%p"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_29(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(TEST_INT) QUOTE(TEST_INT_NEGATIVE)
#define FORMAT "%*d%d"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_30(void)
{
    int reta, retb;
    int ia, ib;

#define STRING QUOTE(TEST_INT_TOO_LARGE)
#define FORMAT "%3d"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_31(void)
{
    int reta, retb;
    int ia, ib;

#define STRING "-0x" QUOTE(TEST_INT_TOO_LARGE)
#define FORMAT "%5i"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_32(void)
{
    int reta, retb;
    char stra[TEST_STR_SIZE], strb[TEST_STR_SIZE];

    memset(stra, 0, sizeof(stra));
    memset(strb, 0, sizeof(strb));

#define STRING TEST_STR
#define FORMAT "%s"
    reta = sscanf(STRING, FORMAT, &stra);
    retb = fmt_sscanf(STRING, FORMAT, &strb);
    check(reta == retb);
    check(strcmp(stra, strb) == 0);
#undef FORMAT
#undef STRING
}

static void
test_33(void)
{
    int reta, retb;
    char stra[TEST_STR_SIZE], strb[TEST_STR_SIZE];

    memset(stra, 0, sizeof(stra));
    memset(strb, 0, sizeof(strb));

#define STRING TEST_STR
#define FORMAT "%*s%s"
    reta = sscanf(STRING, FORMAT, &stra);
    retb = fmt_sscanf(STRING, FORMAT, &strb);
    check(reta == retb);
    check(strcmp(stra, strb) == 0);
#undef FORMAT
#undef STRING
}

static void
test_34(void)
{
    int reta, retb;
    char stra[TEST_STR_SIZE], strb[TEST_STR_SIZE];

    memset(stra, 0, sizeof(stra));
    memset(strb, 0, sizeof(strb));

#define STRING "abc " TEST_STR
#define FORMAT "%*s%s"
    reta = sscanf(STRING, FORMAT, &stra);
    retb = fmt_sscanf(STRING, FORMAT, &strb);
    check(reta == retb);
    check(strcmp(stra, strb) == 0);
#undef FORMAT
#undef STRING
}

static void
test_35(void)
{
    int reta, retb;
    char stra[TEST_STR_SIZE], strb[TEST_STR_SIZE];

    memset(stra, 0, sizeof(stra));
    memset(strb, 0, sizeof(strb));

#define STRING "abc " TEST_STR
#define FORMAT "%*s%3s"
    reta = sscanf(STRING, FORMAT, &stra);
    retb = fmt_sscanf(STRING, FORMAT, &strb);
    check(reta == retb);
    check(strcmp(stra, strb) == 0);
#undef FORMAT
#undef STRING
}

static void
test_36(void)
{
    int reta, retb;
    char stra[TEST_STR_SIZE], strb[TEST_STR_SIZE];

    memset(stra, 0, sizeof(stra));
    memset(strb, 0, sizeof(strb));

#define STRING TEST_STR
#define FORMAT "%*c%c"
    reta = sscanf(STRING, FORMAT, &stra);
    retb = fmt_sscanf(STRING, FORMAT, &strb);
    check(reta == retb);
    check(strcmp(stra, strb) == 0);
#undef FORMAT
#undef STRING
}

static void
test_37(void)
{
    int reta, retb;
    char stra[TEST_STR_SIZE], strb[TEST_STR_SIZE];

    memset(stra, 0, sizeof(stra));
    memset(strb, 0, sizeof(strb));

#define STRING "a"
#define FORMAT "%5c"
    reta = sscanf(STRING, FORMAT, &stra);
    retb = fmt_sscanf(STRING, FORMAT, &strb);
    check(reta == retb);
    check(strcmp(stra, strb) == 0);
#undef FORMAT
#undef STRING
}

static void
test_38(void)
{
    int reta, retb;
    int ia, ib;
    int na, nb;
    int ja, jb;

#define STRING QUOTE(TEST_INT) ":abc:" QUOTE(TEST_INT_NEGATIVE)
#define FORMAT "%d:abc:%n%d"
    reta = sscanf(STRING, FORMAT, &ia, &na, &ja);
    retb = fmt_sscanf(STRING, FORMAT, &ib, &nb, &jb);
    check(reta == retb);
    check(ia == ib);
    check(na == nb);
    check(ja == jb);
#undef FORMAT
#undef STRING
}

static void
test_39(void)
{
    int reta, retb;
    unsigned int ia, ib;

#define STRING "0"
#define FORMAT "%u"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_40(void)
{
    int reta, retb;
    int ia, ib;

#define STRING "-0"
#define FORMAT "%d"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_41(void)
{
    int reta, retb;
    unsigned int ia, ib;
    char ja, jb;
    unsigned int ka, kb;

#define STRING "34800"
#define FORMAT "%u%c%1u"
    reta = sscanf(STRING, FORMAT, &ia, &ja, &ka);
    retb = fmt_sscanf(STRING, FORMAT, &ib, &jb, &kb);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

static void
test_42(void)
{
    int reta, retb;
    char ia, ib;

#define STRING "abc"
#define FORMAT "%0c"
    reta = sscanf(STRING, FORMAT, &ia);
    retb = fmt_sscanf(STRING, FORMAT, &ib);
    check(reta == retb);
    check(ia == ib);
#undef FORMAT
#undef STRING
}

int
main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

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

    return 0;
}
