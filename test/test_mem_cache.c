/*
 * Copyright (c) 2010, 2011 Richard Braun.
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

#include <assert.h>
#include <stddef.h>
#include <pthread.h>

#include <stdio.h>

#include "../cpu.h"
#include "../mem.c"
#include "../error.c"
#include "../avltree.c"

#ifdef CONFIG_MEM_USE_PHYS
#include "../phys.c"
#endif /* CONFIG_MEM_USE_PHYS */

#define NTHREADS    4
#define TEST_TIME   60
#define OBJSPERLOOP 100

struct result {
    unsigned long allocs;
    unsigned long frees;
} __aligned(CPU_L1_SIZE);

struct obj {
    unsigned long nr_refs;
    char name[16];
};

static void obj_ctor(void *ptr)
{
    struct obj *obj;

    obj = ptr;
    obj->nr_refs = 0;
}

static struct mem_cache *obj_cache;
static volatile int work;
static struct result results[NTHREADS];

static void * run(void *arg)
{
    struct obj *objs[OBJSPERLOOP];
    struct result *result;
    int i, id;

    result = arg;
    id = result - results;

    mem_print("started");

    while (work) {
        for (i = 0; i < OBJSPERLOOP; i++) {
            objs[i] = mem_cache_alloc(obj_cache);
            result->allocs++;
        }

        for (i = 0; i < OBJSPERLOOP; i++) {
            mem_cache_free(obj_cache, objs[i]);
            result->frees++;
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t threads[NTHREADS];
    unsigned long ops;
    int i;

    (void)argc;
    (void)argv;

    mem_init();

    mem_info();

    mem_print("Selected cache line size: %u", CPU_L1_SIZE);
    mem_print("sizeof(pthread_mutex_t): %zu", sizeof(pthread_mutex_t));
    mem_print("sizeof(struct mem_cpu_pool): %zu", sizeof(struct mem_cpu_pool));
    mem_print("sizeof(union mem_bufctl): %zu", sizeof(union mem_bufctl));
    mem_print("sizeof(struct mem_buftag): %zu", sizeof(struct mem_buftag));
    mem_print("sizeof(struct mem_slab): %zu", sizeof(struct mem_slab));
    mem_print("sizeof(struct mem_cache): %zu", sizeof(struct mem_cache));
    mem_print("sizeof(struct obj): %zu", sizeof(struct obj));

    obj_cache = mem_cache_create("obj", sizeof(struct obj), 0,
                                 obj_ctor, NULL, 0);

    memset(results, 0, sizeof(results));
    work = 1;

    for (i = 0; i < NTHREADS; i++)
        pthread_create(&threads[i], NULL, run, &results[i]);

    sleep(TEST_TIME);
    work = 0;

    for (i = 0; i < NTHREADS; i++)
        pthread_join(threads[i], NULL);

    ops = 0;

    for (i = 0; i < NTHREADS; i++)
        ops += results[i].allocs + results[i].frees;

    mem_info();
    mem_cache_info(obj_cache);
    mem_print("total: %lu ops in %d secs", ops, TEST_TIME);

    mem_cache_destroy(obj_cache);

    return 0;
}
