/*
 * Copyright (c) 2017 Richard Braun.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cbuf.h>
#include <check.h>
#include <macros.h>

#define TEST_BUF_SIZE 1024

static void
test_push(struct cbuf *cbuf, const char *s)
{
    size_t i, len;

    len = strlen(s) + 1;

    for (i = 0; i < len; i++) {
        cbuf_pushb(cbuf, s[i], true);
    }
}

static void
test_write(struct cbuf *cbuf, size_t index, char *s, size_t size)
{
    int error;

    error = cbuf_write(cbuf, index, s, size);
    check(!error);
}

static void
test_check(const struct cbuf *cbuf, size_t index, const char *s, size_t size)
{
    char buf[TEST_BUF_SIZE];
    int error;

    check(size <= sizeof(buf));
    error = cbuf_read(cbuf, index, buf, &size);
    check(!error);
    check(memcmp(s, buf, size) == 0);
}

static void
test_read_0(void)
{
    char cbuf_buf[TEST_BUF_SIZE], buf[TEST_BUF_SIZE];
    size_t index, size;
    struct cbuf cbuf;
    int error;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    index = cbuf_end(&cbuf);

    size = 0;
    error = cbuf_read(&cbuf, index, buf, &size);
    check(!error);
    check(size == 0);

    test_push(&cbuf, "a");
    size = 0;
    error = cbuf_read(&cbuf, index, buf, &size);
    check(!error);
    check(size == 0);
}

static void
test_read_regular(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    size_t index;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    cbuf.start = 0;
    cbuf.end = cbuf.start;
    index = cbuf_end(&cbuf);

#define STRING "abcdef"
    test_push(&cbuf, STRING);
    test_check(&cbuf, index, STRING, strlen(STRING) + 1);
#undef STRING
}

static void
test_read_overflow(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    size_t index;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    cbuf.start = TEST_BUF_SIZE - 1;
    cbuf.end = cbuf.start;
    index = cbuf_end(&cbuf);

#define STRING "abcdef"
    test_push(&cbuf, STRING);
    test_check(&cbuf, index, STRING, strlen(STRING) + 1);
#undef STRING
}

static void
test_read_short(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    size_t index;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    index = cbuf_end(&cbuf);

#define STRING "abcdef"
    test_push(&cbuf, STRING);
    test_check(&cbuf, index, STRING, strlen(STRING) + 10);
#undef STRING
}

static void
test_append_regular(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    size_t index;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    cbuf.start = 0;
    cbuf.end = cbuf.start;
    index = cbuf_end(&cbuf);

#define STRING "abcdef"
    test_write(&cbuf, index, STRING, STRLEN(STRING) + 1);
    test_check(&cbuf, index, STRING, STRLEN(STRING) + 1);
    check(cbuf_size(&cbuf) == (STRLEN(STRING) + 1));
#undef STRING
}

static void
test_append_overflow(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    size_t index;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    cbuf.start = TEST_BUF_SIZE - 1;
    cbuf.end = cbuf.start;
    index = cbuf_end(&cbuf);

#define STRING "abcdef"
    test_write(&cbuf, index, STRING, STRLEN(STRING) + 1);
    test_check(&cbuf, index, STRING, STRLEN(STRING) + 1);
    check(cbuf_size(&cbuf) == (STRLEN(STRING) + 1));
#undef STRING
}

static void
test_append_large(void)
{
    char cbuf_buf[TEST_BUF_SIZE], buf[TEST_BUF_SIZE * 3];
    struct cbuf cbuf;
    size_t index;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    cbuf.start = 0;
    cbuf.end = cbuf.start;
    index = cbuf_end(&cbuf);

    memset(buf, 0xaa, TEST_BUF_SIZE);
    memset(buf + TEST_BUF_SIZE, 0xab, TEST_BUF_SIZE);
    memset(buf + (TEST_BUF_SIZE * 2), 0xac, TEST_BUF_SIZE);

    test_write(&cbuf, index, buf, sizeof(buf));
    index += TEST_BUF_SIZE * 2;
    test_check(&cbuf, index, buf + (TEST_BUF_SIZE * 2), TEST_BUF_SIZE);
}

static void
test_append_overwrite(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    size_t index;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    cbuf.start = 0;
    cbuf.end = TEST_BUF_SIZE;
    index = cbuf_end(&cbuf);

#define STRING "abcdef"
    test_write(&cbuf, index, STRING, STRLEN(STRING) + 1);
    test_check(&cbuf, index, STRING, STRLEN(STRING) + 1);
    check(cbuf_size(&cbuf) == TEST_BUF_SIZE);
#undef STRING
}

static void
test_write_regular(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    size_t index;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    cbuf.start = 0;
    cbuf.end = TEST_BUF_SIZE;
    index = cbuf_start(&cbuf);

#define STRING "abcdef"
    test_write(&cbuf, index, STRING, STRLEN(STRING) + 1);
    test_check(&cbuf, index, STRING, STRLEN(STRING) + 1);
    check(cbuf_size(&cbuf) == TEST_BUF_SIZE);
#undef STRING
}

static void
test_write_overflow(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    size_t index;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    cbuf.start = TEST_BUF_SIZE - 2;
    cbuf.end = TEST_BUF_SIZE - 1;
    index = cbuf_start(&cbuf);

#define STRING "abcdef"
    test_write(&cbuf, index, STRING, STRLEN(STRING) + 1);
    test_check(&cbuf, index, STRING, STRLEN(STRING) + 1);
    check(cbuf_size(&cbuf) == (STRLEN(STRING) + 1));
#undef STRING
}

static void
test_push_buf(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    int error;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));

#define STRING "abcdef"
    error = cbuf_push(&cbuf, STRING, STRLEN(STRING) + 1, false);
    check(!error);
    test_check(&cbuf, cbuf_start(&cbuf), STRING, STRLEN(STRING) + 1);
    check(cbuf_size(&cbuf) == (STRLEN(STRING) + 1));
#undef STRING
}

static void
test_push_buf_overflow(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    int error;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));
    cbuf.start = 0;
    cbuf.end = TEST_BUF_SIZE - 1;

#define STRING "abcdef"
    error = cbuf_push(&cbuf, STRING, STRLEN(STRING) + 1, false);
    check(error);
#undef STRING
}

static void
test_pop_buf(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    size_t size;
    int error;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));

#define STRING "abcdef"
    error = cbuf_push(&cbuf, STRING, STRLEN(STRING) + 1, false);
    check(!error);
    size = sizeof(cbuf_buf);
    error = cbuf_pop(&cbuf, cbuf_buf, &size);
    check(!error && (size == (STRLEN(STRING) + 1)));
#undef STRING
}

static void
test_pop_buf_overflow(void)
{
    char cbuf_buf[TEST_BUF_SIZE];
    struct cbuf cbuf;
    size_t size;
    int error;

    cbuf_init(&cbuf, cbuf_buf, sizeof(cbuf_buf));

#define STRING "abcdef"
    size = sizeof(cbuf_buf);
    error = cbuf_pop(&cbuf, cbuf_buf, &size);
    check(error);
#undef STRING
}

int
main(void)
{
    test_read_0();
    test_read_regular();
    test_read_overflow();
    test_read_short();
    test_append_regular();
    test_append_overflow();
    test_append_large();
    test_append_overwrite();
    test_write_regular();
    test_write_overflow();
    test_push_buf();
    test_push_buf_overflow();
    test_pop_buf();
    test_pop_buf_overflow();

    return 0;
}
