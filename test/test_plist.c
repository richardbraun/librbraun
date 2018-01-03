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
    plist_node_init(&obj->node, priority);
    printf("add: %p:%u\n", obj, obj->priority);
    plist_add(list, &obj->node);
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
