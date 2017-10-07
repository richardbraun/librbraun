/*
 * Copyright (c) 2009-2015 Richard Braun.
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
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "macros.h"

/*
 * Error message table.
 *
 * This table must be consistent with the enum defined in error.h.
 */
static const char *errormsg_table[] = {
    "success",
    "unknown error",
    "invalid argument",
    "not enough space",
    "invalid format",
    "not enough resources",
    "operation not permitted",
    "resource busy",
    "operation timed out",
    "resource temporarily unavailable",
    "entry exist",
};

#define ERRORMSG_TABLE_SIZE ARRAY_SIZE(errormsg_table)

const char *
error_str(unsigned int error)
{
    if (error >= ERRORMSG_TABLE_SIZE) {
        return "invalid error code";
    }

    return errormsg_table[error];
}

unsigned int
error_from_errno(int errno_code)
{
    switch (errno_code) {
    case 0:
        return ERROR_SUCCESS;
    case EINVAL:
        return ERROR_INVAL;
    case ENOMEM:
        return ERROR_NOMEM;
    case EAGAIN:
        return ERROR_NORES;
    case EPERM:
        return ERROR_PERM;
    case EBUSY:
        return ERROR_BUSY;
    case ETIMEDOUT:
        return ERROR_TIMEDOUT;
    default:
        fprintf(stderr, "unable to translate errno code (%d)\n", errno_code);
        return ERROR_UNKNOWN;
    }
}

void
error_die(unsigned int error)
{
    fprintf(stderr, "process terminating, reason: %s\n", error_str(error));
    abort();
}
