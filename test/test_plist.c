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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../macros.h"
#include "../plist.h"

struct obj {
    struct plist_node node;
    unsigned int priority;
};

struct plist obj_list = PLIST_INITIALIZER(obj_list);

static void
die(const char *msg)
{
    perror(msg);
    exit(1);
}

static struct obj *
add_obj(struct plist *list, unsigned int priority)
{
    struct obj *obj;

    obj = malloc(sizeof(*obj));

    if (obj == NULL) {
        die("malloc");
    }

    obj->priority = priority;
    printf("add: %p:%u\n", obj, obj->priority);
    plist_add(list, &obj->node, priority);
    return obj;
}

static void
del_obj(struct plist *list, struct obj *obj)
{
    printf("del: %p:%u\n", obj, obj->priority);
    plist_remove(list, &obj->node);
    free(obj);
}

static void
print_list(const struct plist *list)
{
    struct obj *obj;

    plist_for_each_entry(list, obj, node) {
        printf("list: %p:%u\n", obj, obj->priority);
    }
}

int
main(int argc, char *argv[])
{
    struct obj *obj, *tmp;
    unsigned int prev_priority __attribute__((unused));

    (void)argc;
    (void)argv;

    add_obj(&obj_list, 1);
    add_obj(&obj_list, 3);
    obj = plist_first_entry(&obj_list, struct obj, node);
    printf("obj: %p:%u\n", obj, obj->priority);
    obj = plist_next_entry(obj, node);
    printf("obj: %p:%u\n", obj, obj->priority);
    add_obj(&obj_list, 5);
    obj = add_obj(&obj_list, 4);
    add_obj(&obj_list, 4);
    tmp = add_obj(&obj_list, 4);
    add_obj(&obj_list, 2);
    add_obj(&obj_list, 3);
    del_obj(&obj_list, obj);
    del_obj(&obj_list, tmp);
    add_obj(&obj_list, 6);
    add_obj(&obj_list, 6);
    add_obj(&obj_list, 3);
    add_obj(&obj_list, 2);
    print_list(&obj_list);

    prev_priority = 0;

    plist_for_each_entry_safe(&obj_list, obj, tmp, node) {
        assert(prev_priority <= obj->priority);
        del_obj(&obj_list, obj);
    }

    return 0;
}
