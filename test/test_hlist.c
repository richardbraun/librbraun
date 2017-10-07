/*
 * Copyright (c) 2017 Richard Braun.
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../check.h"
#include "../macros.h"
#include "../hlist.h"

struct obj {
    struct hlist_node node;
};

static struct obj *
obj_create(void)
{
    struct obj *obj;

    obj = malloc(sizeof(struct obj));
    hlist_node_init(&obj->node);
    return obj;
}

static void
obj_destroy(struct obj *obj)
{
    free(obj);
}

static void
add_obj_head(struct hlist *list)
{
    struct obj *obj;

    obj = obj_create();
    check(hlist_node_unlinked(&obj->node));
    hlist_insert_head(list, &obj->node);
}

static void
add_obj_head2(struct hlist *list)
{
    struct hlist_node *node;
    struct obj *obj;

    obj = obj_create();
    check(hlist_node_unlinked(&obj->node));
    node = hlist_first(list);
    hlist_insert_before(node, &obj->node);
}

static void
add_obj_second(struct hlist *list)
{
    struct hlist_node *node;
    struct obj *obj;

    obj = obj_create();
    check(hlist_node_unlinked(&obj->node));
    node = hlist_first(list);
    hlist_insert_after(node, &obj->node);
}

static void
add_obj_head_llsync(struct hlist *list)
{
    struct obj *obj;

    obj = obj_create();
    hlist_llsync_insert_head(list, &obj->node);
}

static void
add_obj_head2_llsync(struct hlist *list)
{
    struct hlist_node *node;
    struct obj *obj;

    obj = obj_create();
    check(hlist_node_unlinked(&obj->node));
    node = hlist_first(list);
    hlist_llsync_insert_before(node, &obj->node);
}

static void
add_obj_second_llsync(struct hlist *list)
{
    struct hlist_node *node;
    struct obj *obj;

    obj = obj_create();
    check(hlist_node_unlinked(&obj->node));
    node = hlist_first(list);
    hlist_llsync_insert_after(node, &obj->node);
}

static void
walk_all_objs1(struct hlist *list)
{
    struct hlist_node *node;

    hlist_for_each(list, node);
}

static void
walk_all_objs1_llsync(struct hlist *list)
{
    struct hlist_node *node;

    hlist_llsync_for_each(list, node);
}

static void
walk_all_objs2(struct hlist *list)
{
    struct hlist_node *node, *tmp;

    hlist_for_each_safe(list, node, tmp);
}

static void
walk_all_objs3(struct hlist *list)
{
    struct obj *obj;

    hlist_for_each_entry(list, obj, node);
}

static void
walk_all_objs3_llsync(struct hlist *list)
{
    struct obj *obj;

    hlist_llsync_for_each_entry(list, obj, node);
}

static void
walk_all_objs4(struct hlist *list)
{
    struct obj *obj, *tmp;

    hlist_for_each_entry_safe(list, obj, tmp, node);
}

static void
del_all_objs(struct hlist *list)
{
    struct obj *obj;

    while (!hlist_empty(list)) {
        obj = hlist_first_entry(list, struct obj, node);
        hlist_remove(&obj->node);
        obj_destroy(obj);
    }
}

static void
del_all_objs_llsync(struct hlist *list)
{
    struct obj *obj;

    while (!hlist_empty(list)) {
        obj = hlist_first_entry(list, struct obj, node);
        hlist_llsync_remove(&obj->node);
        obj_destroy(obj);
    }
}

int
main(void)
{
    struct hlist list, list2;

    hlist_init(&list);
    hlist_init(&list2);

    check(!hlist_llsync_first_entry(&list, struct obj, node));

    check(!hlist_singular(&list));
    add_obj_head(&list);
    check(hlist_singular(&list));
    add_obj_head(&list);
    check(!hlist_singular(&list));

    walk_all_objs1(&list);
    walk_all_objs1_llsync(&list);
    walk_all_objs2(&list);
    walk_all_objs3(&list);
    walk_all_objs3_llsync(&list);
    walk_all_objs4(&list);

    del_all_objs(&list);

    add_obj_head(&list);
    add_obj_head2(&list);
    add_obj_second(&list);
    add_obj_head_llsync(&list);
    add_obj_head2_llsync(&list);
    add_obj_second_llsync(&list);
    add_obj_head(&list);
    add_obj_head(&list);
    hlist_set_head(&list2, &list);
    hlist_set_head(&list, &list2);

    del_all_objs_llsync(&list);

    return 0;
}
