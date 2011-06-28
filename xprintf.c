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
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>

#include "xprintf.h"

/*
 * Formatting flags.
 *
 * FORMAT_LOWER must be 0x20 as it is OR'd with digits, eg.
 * '0': 0x30 | 0x20 => 0x30 ('0')
 * 'A': 0x41 | 0x20 => 0x61 ('a')
 */
#define FORMAT_ALT_FORM     0x01    /* "Alternate form"                     */
#define FORMAT_ZERO_PAD     0x02    /* Zero padding on the left             */
#define FORMAT_LEFT_JUSTIFY 0x04    /* Align text on the left               */
#define FORMAT_BLANK        0x08    /* Blank space before positive number   */
#define FORMAT_SIGN         0x10    /* Always place a sign (either + or -)  */
#define FORMAT_LOWER        0x20    /* To lowercase (for %x)                */
#define FORMAT_CONV_SIGNED  0x40    /* Format specifies signed conversion   */

enum {
    MODIFIER_NONE,
    MODIFIER_CHAR,
    MODIFIER_SHORT,
    MODIFIER_LONG,
    MODIFIER_LONGLONG,
    MODIFIER_PTR,       /* Used only for %p */
    MODIFIER_SIZE,
    MODIFIER_PTRDIFF
};

enum {
    SPECIFIER_INVALID,
    SPECIFIER_INT,
    SPECIFIER_CHAR,
    SPECIFIER_STR,
    SPECIFIER_NRCHARS,
    SPECIFIER_PERCENT
};

/*
 * Size for the temporary number buffer. The minimum base is 8 so 3 bits
 * are consumed per digit. Add one to round up. The conversion algorithm
 * doesn't use the null byte.
 */
#define MAX_NUM_SIZE (((sizeof(unsigned long long) * CHAR_BIT) / 3) + 1)

/*
 * Special size for xvsnprintf(), used by xsprintf()/xvsprintf() when the
 * buffer size is unknown.
 */
#define XPRINT_NOLIMIT ((size_t)-1)

/*
 * Size of the static buffer used by xprintf()/xvprintf().
 */
#define XPRINT_BUFSIZE 1024

static char xprint_buffer[XPRINT_BUFSIZE];
static pthread_mutex_t xprint_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char digits[] = "0123456789ABCDEF";

static inline char * xputchar(char *str, char *end, char c)
{
    if (str < end)
        *str = c;

    str++;

    return str;
}

static inline int xisdigit(char c)
{
    return (c >= '0') && (c <= '9');
}

int xprintf(const char *format, ...)
{
    va_list ap;
    int length;

    va_start(ap, format);
    length = xvprintf(format, ap);
    va_end(ap);

    return length;
}

int xvprintf(const char *format, va_list ap)
{
    size_t size;
    int length;

    pthread_mutex_lock(&xprint_mutex);
    length = xvsnprintf(xprint_buffer, sizeof(xprint_buffer), format, ap);
    size = ((unsigned int)length >= sizeof(xprint_buffer))
           ? sizeof(xprint_buffer) - 1
           : (unsigned int)length;
    fwrite(xprint_buffer, 1, size, stdout);
    pthread_mutex_unlock(&xprint_mutex);

    return length;
}

int xsprintf(char *str, const char *format, ...)
{
    va_list ap;
    int length;

    va_start(ap, format);
    length = xvsprintf(str, format, ap);
    va_end(ap);

    return length;
}

int xvsprintf(char *str, const char *format, va_list ap)
{
    return xvsnprintf(str, XPRINT_NOLIMIT, format, ap);
}

int xsnprintf(char *str, size_t size, const char *format, ...)
{
    va_list ap;
    int length;

    va_start(ap, format);
    length = xvsnprintf(str, size, format, ap);
    va_end(ap);

    return length;
}

int xvsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    unsigned long long n;
    int i, len, found, flags, width, precision, modifier, specifier, shift;
    unsigned char r, base, mask;
    char c, *s, *start, *end, sign, tmp[MAX_NUM_SIZE];

    start = str;

    if (size == 0)
        end = NULL;
    else if (size == XPRINT_NOLIMIT)
        end = (char *)-1;
    else
        end = start + size - 1;

    while ((c = *format) != '\0') {
        if (c != '%') {
            str = xputchar(str, end, c);
            format++;
            continue;
        }

        /* Flags */

        found = 1;
        flags = 0;

        do {
            format++;
            c = *format;

            switch (c) {
            case '#':
                flags |= FORMAT_ALT_FORM;
                break;
            case '0':
                flags |= FORMAT_ZERO_PAD;
                break;
            case '-':
                flags |= FORMAT_LEFT_JUSTIFY;
                break;
            case ' ':
                flags |= FORMAT_BLANK;
                break;
            case '+':
                flags |= FORMAT_SIGN;
                break;
            default:
                found = 0;
                break;
            }
        } while (found);

        /* Width */

        if (xisdigit(c)) {
            width = 0;

            while (xisdigit(c)) {
                width = width * 10 + (c - '0');
                format++;
                c = *format;
            }
        } else if (c == '*') {
            width = va_arg(ap, int);

            if (width < 0) {
                flags |= FORMAT_LEFT_JUSTIFY;
                width = -width;
            }

            format++;
            c = *format;
        } else {
            width = 0;
        }

        /* Precision */

        if (c == '.') {
            format++;
            c = *format;

            if (xisdigit(c)) {
                precision = 0;

                while (xisdigit(c)) {
                    precision = precision * 10 + (c - '0');
                    format++;
                    c = *format;
                }
            } else if (c == '*') {
                precision = va_arg(ap, int);

                if (precision < 0)
                    precision = 0;

                format++;
                c = *format;
            } else {
                precision = 0;
            }
        } else {
            /* precision is >= 0 only if explicit */
            precision = -1;
        }

        /* Length modifier */

        switch (c) {
        case 'h':
        case 'l':
            format++;

            if (c == *format) {
                modifier = (c == 'h') ? MODIFIER_CHAR : MODIFIER_LONGLONG;
                goto skip_modifier;
            } else {
                modifier = (c == 'h') ? MODIFIER_SHORT : MODIFIER_LONG;
                c = *format;
            }

            break;
        case 'z':
            modifier = MODIFIER_SIZE;
            goto skip_modifier;
        case 't':
            modifier = MODIFIER_PTRDIFF;
skip_modifier:
            format++;
            c = *format;
            break;
        default:
            modifier = MODIFIER_NONE;
            break;
        }

        /* Specifier */

        switch (c) {
        case 'd':
        case 'i':
            flags |= FORMAT_CONV_SIGNED;
        case 'u':
            base = 10;
            goto integer;
        case 'o':
            base = 8;
            goto integer;
        case 'p':
            flags |= FORMAT_ALT_FORM;
            modifier = MODIFIER_PTR;
        case 'x':
            flags |= FORMAT_LOWER;
        case 'X':
            base = 16;
integer:
            specifier = SPECIFIER_INT;
            break;
        case 'c':
            specifier = SPECIFIER_CHAR;
            break;
        case 's':
            specifier = SPECIFIER_STR;
            break;
        case 'n':
            specifier = SPECIFIER_NRCHARS;
            break;
        case '%':
            specifier = SPECIFIER_PERCENT;
            break;
        default:
            specifier = SPECIFIER_INVALID;
            break;
        }

        /* Output */

        switch (specifier) {
        case SPECIFIER_INT:
            switch (modifier) {
            case MODIFIER_CHAR:
                if (flags & FORMAT_CONV_SIGNED)
                    n = (signed char)va_arg(ap, int);
                else
                    n = (unsigned char)va_arg(ap, int);
                break;
            case MODIFIER_SHORT:
                if (flags & FORMAT_CONV_SIGNED)
                    n = (short)va_arg(ap, int);
                else
                    n = (unsigned short)va_arg(ap, int);
                break;
            case MODIFIER_LONG:
                if (flags & FORMAT_CONV_SIGNED)
                    n = va_arg(ap, long);
                else
                    n = va_arg(ap, unsigned long);
                break;
            case MODIFIER_LONGLONG:
                if (flags & FORMAT_CONV_SIGNED)
                    n = va_arg(ap, long long);
                else
                    n = va_arg(ap, unsigned long long);
                break;
            case MODIFIER_PTR:
                n = (unsigned long)va_arg(ap, void *);
                break;
            case MODIFIER_SIZE:
                if (flags & FORMAT_CONV_SIGNED)
                    n = va_arg(ap, ssize_t);
                else
                    n = va_arg(ap, size_t);
                break;
            case MODIFIER_PTRDIFF:
                n = va_arg(ap, ptrdiff_t);
                break;
            default:
                if (flags & FORMAT_CONV_SIGNED)
                    n = va_arg(ap, int);
                else
                    n = va_arg(ap, unsigned int);
                break;
            }

            if ((flags & FORMAT_LEFT_JUSTIFY) || (precision >= 0))
                flags &= ~FORMAT_ZERO_PAD;

            sign = 0;

            if (flags & FORMAT_ALT_FORM) {
                /* '0' for octal */
                width--;

                /* '0x' or '0X' for hexadecimal */
                if (base == 16)
                    width--;
            } else if (flags & FORMAT_CONV_SIGNED) {
                if ((long long)n < 0) {
                    sign = '-';
                    width--;
                    n = -(long long)n;
                } else if (flags & FORMAT_SIGN) {
                    /* FORMAT_SIGN must precede FORMAT_BLANK. */
                    sign = '+';
                    width--;
                } else if (flags & FORMAT_BLANK) {
                    sign = ' ';
                    width--;
                }
            }

            /* Conversion, in reverse order */

            i = 0;

            if (n == 0) {
                if (precision != 0)
                    tmp[i++] = '0';
            } else if (base == 10) {
                /*
                 * Try to avoid 64 bits operations if the processor doesn't
                 * support them. Note that even when using modulus and
                 * division operators close to each other, the compiler may
                 * forge two calls to __udivdi3() and __umoddi3() instead of
                 * one to __udivmoddi3(), whereas processor instructions are
                 * generally correctly used once, giving both the remainder
                 * and the quotient, through plain or reciprocal division.
                 */
#ifndef __LP64__
                if (modifier == MODIFIER_LONGLONG) {
#endif /* __LP64__ */
                    do {
                        r = n % 10;
                        n /= 10;
                        tmp[i++] = digits[r];
                    } while (n != 0);
#ifndef __LP64__
                } else {
                    unsigned long m;

                    m = (unsigned long)n;

                    do {
                        r = m % 10;
                        m /= 10;
                        tmp[i++] = digits[r];
                    } while (m != 0);
                }
#endif /* __LP64__ */
            } else {
                mask = base - 1;
                shift = (base == 8) ? 3 : 4;

                do {
                    r = (unsigned char)n & mask;
                    n >>= shift;
                    tmp[i++] = digits[r] | (flags & FORMAT_LOWER);
                } while (n != 0);
            }

            if (i > precision)
                precision = i;

            width -= precision;

            if (!(flags & (FORMAT_LEFT_JUSTIFY | FORMAT_ZERO_PAD)))
                while (width-- > 0)
                    str = xputchar(str, end, ' ');

            if (flags & FORMAT_ALT_FORM) {
                str = xputchar(str, end, '0');

                if (base == 16)
                    str = xputchar(str, end, 'X' | (flags & FORMAT_LOWER));
            } else if (sign) {
                str = xputchar(str, end, sign);
            }

            if (!(flags & FORMAT_LEFT_JUSTIFY)) {
                c = (flags & FORMAT_ZERO_PAD) ? '0' : ' ';

                while (width-- > 0)
                    str = xputchar(str, end, c);
            }

            while (i < precision--)
                str = xputchar(str, end, '0');

            while (i-- > 0)
                str = xputchar(str, end, tmp[i]);

            while (width-- > 0)
                str = xputchar(str, end, ' ');

            break;
        case SPECIFIER_CHAR:
            c = (unsigned char)va_arg(ap, int);

            if (!(flags & FORMAT_LEFT_JUSTIFY))
                while (--width > 0)
                    str = xputchar(str, end, ' ');

            str = xputchar(str, end, c);

            while (--width > 0)
                str = xputchar(str, end, ' ');

            break;
        case SPECIFIER_STR:
            s = va_arg(ap, char *);

            if (s == NULL)
                s = "(null)";

            len = 0;

            for (len = 0; s[len] != '\0'; len++)
                if (len == precision)
                    break;

            if (!(flags & FORMAT_LEFT_JUSTIFY))
                while (len < width--)
                    str = xputchar(str, end, ' ');

            for (i = 0; i < len; i++) {
                str = xputchar(str, end, *s);
                s++;
            }

            while (len < width--)
                str = xputchar(str, end, ' ');

            break;
        case SPECIFIER_NRCHARS:
            if (modifier == MODIFIER_CHAR) {
                signed char *ptr = va_arg(ap, signed char *);
                *ptr = str - start;
            } else if (modifier == MODIFIER_SHORT) {
                short *ptr = va_arg(ap, short *);
                *ptr = str - start;
            } else if (modifier == MODIFIER_LONG) {
                long *ptr = va_arg(ap, long *);
                *ptr = str - start;
            } else if (modifier == MODIFIER_LONGLONG) {
                long long *ptr = va_arg(ap, long long *);
                *ptr = str - start;
            } else if (modifier == MODIFIER_SIZE) {
                ssize_t *ptr = va_arg(ap, ssize_t *);
                *ptr = str - start;
            } else if (modifier == MODIFIER_PTRDIFF) {
                ptrdiff_t *ptr = va_arg(ap, ptrdiff_t *);
                *ptr = str - start;
            } else {
                int *ptr = va_arg(ap, int *);
                *ptr = str - start;
            }

            break;
        case SPECIFIER_PERCENT:
        case SPECIFIER_INVALID:
            str = xputchar(str, end, '%');
            break;
        default:
            break;
        }

        if (specifier != SPECIFIER_INVALID)
            format++;
    }

    if (str < end)
        *str = '\0';
    else if (end != NULL)
        *end = '\0';

    return str - start;
}
