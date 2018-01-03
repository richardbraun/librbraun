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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../check.h"
#include "../macros.h"
#include "../slist.h"

struct obj {
    struct slist_node node;
};

static struct obj *
obj_create(void)
{
    return malloc(sizeof(struct obj));
}

static void
obj_destroy(struct obj *obj)
{
    free(obj);
}

static void
add_obj_head(struct slist *list)
{
    struct obj *obj;

    obj = obj_create();
    slist_insert_head(list, &obj->node);
}

static void
add_obj_head_llsync(struct slist *list)
{
    struct obj *obj;

    obj = obj_create();
    slist_llsync_insert_head(list, &obj->node);
}

static void
add_obj_tail(struct slist *list)
{
    struct obj *obj;

    obj = obj_create();
    slist_insert_tail(list, &obj->node);
}

static void
add_obj_tail_llsync(struct slist *list)
{
    struct obj *obj;

    obj = obj_create();
    slist_llsync_insert_tail(list, &obj->node);
}

static void
add_obj_tail2(struct slist *list)
{
    struct obj *obj;

    obj = obj_create();
    slist_insert_after(list, slist_last(list), &obj->node);
}

static void
add_obj_tail2_llsync(struct slist *list)
{
    struct obj *obj;

    obj = obj_create();
    slist_llsync_insert_after(list, slist_last(list), &obj->node);
}

static void
add_obj_second(struct slist *list)
{
    struct slist_node *first;
    struct obj *obj;

    obj = obj_create();
    first = slist_first(list);
    slist_insert_after(list, first, &obj->node);
}

static void
del_obj_head(struct slist *list)
{
    struct obj *obj;

    obj = slist_first_entry(list, struct obj, node);
    slist_remove(list, NULL);
    obj_destroy(obj);
}

static void
del_obj_head_llsync(struct slist *list)
{
    struct obj *obj;

    obj = slist_first_entry(list, struct obj, node);
    slist_llsync_remove(list, NULL);
    obj_destroy(obj);
}

static void
del_obj_tail(struct slist *list)
{
    struct obj *obj;

    obj = slist_last_entry(list, struct obj, node);
    slist_remove(list, slist_first(list));
    obj_destroy(obj);
}

static void
del_obj_tail_llsync(struct slist *list)
{
    struct obj *obj;

    obj = slist_last_entry(list, struct obj, node);
    slist_llsync_remove(list, slist_first(list));
    obj_destroy(obj);
}

static void
del_obj_second(struct slist *list)
{
    struct obj *obj;

    obj = slist_entry(slist_next(slist_first(list)), struct obj, node);
    slist_remove(list, slist_first(list));
    obj_destroy(obj);
}

static void
del_obj_second_llsync(struct slist *list)
{
    struct obj *obj;

    obj = slist_entry(slist_next(slist_first(list)), struct obj, node);
    slist_llsync_remove(list, slist_first(list));
    obj_destroy(obj);
}

static void
walk_all_objs1(struct slist *list)
{
    struct slist_node *node;

    slist_for_each(list, node);
}

static void
walk_all_objs1_llsync(struct slist *list)
{
    struct slist_node *node;

    slist_llsync_for_each(list, node);
}

static void
walk_all_objs2(struct slist *list)
{
    struct slist_node *node, *tmp;

    slist_for_each_safe(list, node, tmp);
}

static void
walk_all_objs3(struct slist *list)
{
    struct obj *obj;

    slist_for_each_entry(list, obj, node);
}

static void
walk_all_objs3_llsync(struct slist *list)
{
    struct obj *obj;

    slist_llsync_for_each_entry(list, obj, node);
}

static void
walk_all_objs4(struct slist *list)
{
    struct obj *obj, *tmp;

    slist_for_each_entry_safe(list, obj, tmp, node);
}

static void
del_all_objs(struct slist *list)
{
    struct obj *obj;

    while (!slist_empty(list)) {
        obj = slist_first_entry(list, struct obj, node);
        slist_remove(list, NULL);
        obj_destroy(obj);
    }
}

static void
del_all_objs_llsync(struct slist *list)
{
    struct obj *obj;

    while (!slist_empty(list)) {
        obj = slist_first_entry(list, struct obj, node);
        slist_llsync_remove(list, NULL);
        obj_destroy(obj);
    }
}

int
main(void)
{
    struct slist list, list2;

    slist_init(&list);
    slist_init(&list2);

    check(!slist_llsync_first_entry(&list, struct obj, node));

    add_obj_head(&list);
    check(slist_llsync_first_entry(&list, struct obj, node));
    check(slist_singular(&list));
    del_all_objs(&list);

    add_obj_tail(&list);
    add_obj_tail(&list);
    del_obj_tail(&list);
    add_obj_tail(&list);
    del_obj_tail_llsync(&list);

    slist_concat(&list, &list2);
    slist_set_head(&list2, &list);
    slist_init(&list);
    slist_concat(&list, &list2);
    slist_init(&list2);
    add_obj_tail(&list);
    add_obj_tail(&list);
    add_obj_tail(&list2);
    add_obj_tail(&list2);
    slist_concat(&list, &list2);

    add_obj_tail(&list);
    add_obj_tail(&list);
    add_obj_tail(&list);
    add_obj_tail(&list);
    add_obj_head(&list);
    add_obj_head(&list);

    del_all_objs(&list);
    slist_init(&list);

    add_obj_head(&list);
    add_obj_head_llsync(&list);
    slist_init(&list2);
    add_obj_head_llsync(&list2);
    del_all_objs(&list2);
    add_obj_head(&list);
    add_obj_head(&list);
    add_obj_tail(&list);
    add_obj_tail_llsync(&list);
    slist_init(&list2);
    add_obj_tail_llsync(&list2);
    del_all_objs(&list2);
    add_obj_tail(&list);
    add_obj_tail(&list);
    add_obj_tail(&list);
    add_obj_tail(&list);
    add_obj_tail(&list);
    add_obj_head(&list);
    add_obj_head(&list);
    add_obj_head(&list);
    add_obj_tail2(&list);
    add_obj_tail2_llsync(&list);

    walk_all_objs1(&list);
    walk_all_objs2(&list);
    walk_all_objs3(&list);
    walk_all_objs4(&list);
    walk_all_objs1_llsync(&list);
    walk_all_objs3_llsync(&list);

    del_obj_head(&list);
    del_obj_head_llsync(&list);
    del_obj_second(&list);
    del_obj_second_llsync(&list);
    add_obj_second(&list);

    del_all_objs_llsync(&list);

    return 0;
}
