/*
 * Copyright (c) 2017 Richard Braun.
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
#include <stdlib.h>
#include <string.h>

#include "../cbuf.h"
#include "../check.h"
#include "../error.h"
#include "../macros.h"

#define TEST_BUF_SIZE 1024

static void
test_push(struct cbuf *cbuf, const char *s)
{
    size_t i, len;

    len = strlen(s) + 1;

    for (i = 0; i < len; i++) {
        cbuf_push(cbuf, s[i]);
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
    check(error == ERR_INVAL);
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

int
main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

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

    return 0;
}