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

#ifndef _ERROR_H
#define _ERROR_H

#include <stdnoreturn.h>

#include "macros.h"

/*
 * List of errors this library can return.
 *
 * ERROR_SUCCESS is guaranteed to be 0, allowing code such as :
 *
 * error = do_smth();
 *
 * if (error) {
 *     ...;
 * }
 */
enum {
    ERROR_SUCCESS,
    ERROR_UNKNOWN,
    ERROR_INVAL,
    ERROR_NOMEM,
    ERROR_FORMAT,
    ERROR_NORES,
    ERROR_PERM,
    ERROR_BUSY,
    ERROR_MEMLIM,
    ERROR_TIMEDOUT,
    ERROR_WOULDBLOCK,
    ERROR_LOOKUP,
    ERROR_MEM_CACHE,
    ERROR_AGAIN,
    ERROR_EXIST,
};

/*
 * Return the message matching the given error.
 *
 * The returned address points to a statically allocated, read only,
 * null-terminated string literal. The caller must not attempt to use it
 * for anything else than error reporting.
 */
const char * error_str(unsigned int error);

/*
 * Map standard error codes to error values.
 *
 * This function accepts a subset of the standard error codes in errno.h.
 * When called, and if the errno value is handled, it will return the
 * corresponding ERROR_xxx code. Otherwise ERROR_UNKNOWN is returned.
 */
unsigned int error_from_errno(int errno_code);

/*
 * Exit the current process, reporting an error.
 *
 * This function will report the given error and make the process exit,
 * using the error code as the exit() parameter.
 */
noreturn void error_die(unsigned int error);

#endif /* _ERROR_H */
