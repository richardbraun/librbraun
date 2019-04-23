/*
 * Copyright (c) 2018 Richard Braun.
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

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <check.h>
#include <macros.h>
#include <mbuf.h>

static void
test_regular(void)
{
    char mbuf_buf[32], buffer[8];
    struct mbuf mbuf;
    size_t size;
    int error;

    mbuf_init(&mbuf, mbuf_buf, sizeof(mbuf_buf), 255);

#define STRING "abcdef"
    error = mbuf_push(&mbuf, STRING, STRLEN(STRING) + 1, false);
    check(!error);
#undef STRING

#define STRING "xyz"
    error = mbuf_push(&mbuf, STRING, STRLEN(STRING) + 1, false);
    check(!error);
#undef STRING

#define STRING "abcdef"
    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(!error);
    check(size == (STRLEN(STRING) + 1));
#undef STRING

#define STRING "xyz"
    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(!error);
    check(size == (STRLEN(STRING) + 1));
#undef STRING

    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(error == EAGAIN);
}

static void
test_write_full(void)
{
    char mbuf_buf[16], buffer[8];
    struct mbuf mbuf;
    size_t size;
    int error;

    mbuf_init(&mbuf, mbuf_buf, sizeof(mbuf_buf), (size_t)-1);

#define STRING "abcdef"
    error = mbuf_push(&mbuf, STRING, STRLEN(STRING) + 1, false);
    check(!error);
#undef STRING

#define STRING "xyz"
    error = mbuf_push(&mbuf, STRING, STRLEN(STRING) + 1, false);
    check(error == EMSGSIZE);
#undef STRING

#define STRING "abcdef"
    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(!error);
    check(size == (STRLEN(STRING) + 1));
#undef STRING

    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(error == EAGAIN);
}

static void
test_overwrite(void)
{
    char mbuf_buf[16], buffer[8];
    struct mbuf mbuf;
    size_t size;
    int error;

    mbuf_init(&mbuf, mbuf_buf, sizeof(mbuf_buf), (size_t)-1);

#define STRING "abcdef"
    error = mbuf_push(&mbuf, STRING, STRLEN(STRING) + 1, true);
    check(!error);
#undef STRING

#define STRING "xyz"
    error = mbuf_push(&mbuf, STRING, STRLEN(STRING) + 1, true);
    check(!error);
#undef STRING

#define STRING "xyz"
    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(!error);
    check(size == (STRLEN(STRING) + 1));
#undef STRING

    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(error == EAGAIN);
}

static void
test_msg_size_power_of_two(void)
{
    char mbuf_buf[512], buffer[256];
    struct mbuf mbuf;
    size_t size;
    int error;

    mbuf_init(&mbuf, mbuf_buf, sizeof(mbuf_buf), sizeof(buffer));

    memset(buffer, 0xab, sizeof(buffer));

    error = mbuf_push(&mbuf, buffer, sizeof(buffer), true);
    check(!error);

    memset(buffer, 0xf0, sizeof(buffer));

    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(!error && (size == sizeof(buffer)));

    for (size_t i = 0; i < sizeof(buffer); i++) {
        check(buffer[i] == (char)0xab);
    }
}

static void
test_msg_size_uint8_max(void)
{
    char mbuf_buf[512], buffer[255];
    struct mbuf mbuf;
    size_t size;
    int error;

    mbuf_init(&mbuf, mbuf_buf, sizeof(mbuf_buf), sizeof(buffer));

    memset(buffer, 0xab, sizeof(buffer));

    error = mbuf_push(&mbuf, buffer, sizeof(buffer), true);
    check(!error);

    memset(buffer, 0xf0, sizeof(buffer));

    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(!error && (size == sizeof(buffer)));

    for (size_t i = 0; i < sizeof(buffer); i++) {
        check(buffer[i] == (char)0xab);
    }
}

static void
test_msg_too_big_to_fit(void)
{
    char mbuf_buf[8], buffer[8];
    struct mbuf mbuf;
    size_t size;
    int error;

    mbuf_init(&mbuf, mbuf_buf, sizeof(mbuf_buf), (size_t)-1);

#define STRING "abcdef"
    error = mbuf_push(&mbuf, STRING, STRLEN(STRING) + 1, true);
    check(error == EMSGSIZE);
#undef STRING

    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(error == EAGAIN);
}

static void
test_msg_bigger_than_max(void)
{
    char mbuf_buf[512], buffer[256];
    struct mbuf mbuf;
    size_t size;
    int error;

    mbuf_init(&mbuf, mbuf_buf, sizeof(mbuf_buf), sizeof(buffer) - 1);

    memset(buffer, 0xab, sizeof(buffer));

    error = mbuf_push(&mbuf, buffer, sizeof(buffer), true);
    check(error == EINVAL);

    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(error == EAGAIN);
}

static void
test_peak(void)
{
    char mbuf_buf[16], buffer[8];
    struct mbuf mbuf;
    size_t size;
    int error;

    mbuf_init(&mbuf, mbuf_buf, sizeof(mbuf_buf), (size_t)-1);

#define STRING "abcdef"
    error = mbuf_push(&mbuf, STRING, STRLEN(STRING) + 1, true);
    check(!error);
#undef STRING

#define STRING "abcdef"
    size = 0;
    error = mbuf_pop(&mbuf, NULL, &size);
    check((error == EMSGSIZE) && (size == (STRLEN(STRING) + 1)));
    error = mbuf_pop(&mbuf, buffer, &size);
    check(!error && (size == (STRLEN(STRING) + 1)));
#undef STRING

    size = sizeof(buffer);
    error = mbuf_pop(&mbuf, buffer, &size);
    check(error == EAGAIN);
}

int
main(void)
{
    test_regular();
    test_write_full();
    test_overwrite();
    test_msg_size_power_of_two();
    test_msg_size_uint8_max();
    test_msg_too_big_to_fit();
    test_msg_bigger_than_max();
    test_peak();

    return EXIT_SUCCESS;
}
