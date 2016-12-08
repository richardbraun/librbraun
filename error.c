/*
 * Copyright (c) 2009-2015 Richard Braun.
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
    "memory limit exceeded",
    "operation timed out",
    "operation would block",
    "entry not found",
    "internal memory allocator failure",
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
        return ERR_SUCCESS;
    case EINVAL:
        return ERR_INVAL;
    case ENOMEM:
        return ERR_NOMEM;
    case EAGAIN:
        return ERR_NORES;
    case EPERM:
        return ERR_PERM;
    case EBUSY:
        return ERR_BUSY;
    case ETIMEDOUT:
        return ERR_TIMEDOUT;
    default:
        fprintf(stderr, "unable to translate errno code (%d)\n", errno_code);
        return ERR_UNKNOWN;
    }
}

void
error_die(unsigned int error)
{
    fprintf(stderr, "process terminating, reason: %s\n", error_str(error));
    abort();
}
