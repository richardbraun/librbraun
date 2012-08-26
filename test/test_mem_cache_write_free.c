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

#define _GNU_SOURCE
#include <sched.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../mem.h"
#include "../macros.h"

struct obj {
    unsigned long nr_refs;
    char name[16];
};

static void
obj_ctor(void *ptr)
{
    struct obj *obj;

    obj = ptr;
    obj->nr_refs = 0;
}

static void
obj_print(struct obj *obj)
{
    unsigned char *ptr, *end;

    printf("buffer content: ");

    for (ptr = (unsigned char *)obj, end = ptr + sizeof(*obj);
         ptr < end;
         ptr++)
        printf("%02x", *ptr);

    printf("\n");
}

int
main(int argc, char *argv[])
{
    struct mem_cache *obj_cache;
    struct obj *obj;
    cpu_set_t cpu_set;

    (void)argc;
    (void)argv;

    printf("binding to CPU 0\n");
    CPU_ZERO(&cpu_set);
    CPU_SET(0, &cpu_set);
    sched_setaffinity(0, sizeof(cpu_set), &cpu_set);

    mem_setup();

    obj_cache = mem_cache_create("obj", sizeof(struct obj), 0,
                                 obj_ctor, NULL, MEM_CACHE_VERIFY);

    printf("doing normal alloc+free:\n");
    obj = mem_cache_alloc(obj_cache);
    mem_cache_free(obj_cache, obj);

    mem_cache_info(obj_cache);

    obj_print(obj);

    printf("doing write on free object:\n");
    memset((void *)obj + 3, 0x89, 5);

    obj_print(obj);

    printf("trying normal alloc+free on same CPU (should fail):\n");

    obj = mem_cache_alloc(obj_cache);
    mem_cache_free(obj_cache, obj);

    printf("done\n");

    return 0;
}
