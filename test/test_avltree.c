/*
 * Copyright (c) 2010-2017 Richard Braun.
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

#include "../avltree.h"
#include "../check.h"
#include "../hash.h"
#include "../macros.h"

#define SIZE 28

struct obj {
    struct avltree_node node;
    int id;
};

static inline int
obj_cmp_lookup(int id, struct avltree_node *node)
{
    struct obj *obj;

    obj = avltree_entry(node, struct obj, node);
    return id - obj->id;
}

static void
print_subtree(struct avltree_node *node, int level)
{
    struct obj *obj;
    char balance;
    int i;

    if (node == NULL) {
        return;
    }

    print_subtree(node->children[AVLTREE_RIGHT], level + 1);

    for (i = level; i > 0; i--) {
        putchar(' ');
    }

    obj = avltree_entry(node, struct obj, node);

    switch(node->parent & AVLTREE_BALANCE_MASK) {
    case 0:
        balance = '-';
        break;
    case 1:
        balance = '0';
        break;
    case 2:
        balance = '+';
        break;
    default:
        printf("invalid balance for node %d\n", obj->id);
        return;
    }

    printf("%d:%c\n", obj->id, balance);

    print_subtree(node->children[AVLTREE_LEFT], level + 1);
}

static void
print_tree(struct avltree *tree)
{
    print_subtree(tree->root, 0);
}

static int
get_id(int i)
{
    return hash_int32(i, 6);
}

int
main(int argc, char *argv[])
{
    struct avltree tree;
    struct avltree_node *node, *tmp;
    struct obj *obj, *prev;
    avltree_slot_t slot;
    int i, id;

    (void)argc;
    (void)argv;

    avltree_init(&tree);

    for (i = 0; i < SIZE; i++) {
        id = get_id(i);
        node = avltree_lookup_slot(&tree, id, obj_cmp_lookup, slot);

        if (node != NULL) {
            continue;
        }

        obj = malloc(sizeof(*obj));
        obj->id = id;
        printf("%d ", obj->id);
        avltree_insert_slot(&tree, slot, &obj->node);
    }

    id = get_id(0);
    node = avltree_lookup_slot(&tree, id, obj_cmp_lookup, slot);
    check(node);
    obj = malloc(sizeof(*obj));
    check(obj);
    obj->id = id;
    printf("replacing: %d ", obj->id);
    node = avltree_replace_slot(&tree, slot, &obj->node);
    check(node != &obj->node);
    prev = avltree_entry(node, struct obj, node);
    free(prev);
    node = avltree_lookup(&tree, id, obj_cmp_lookup);
    check(node == &obj->node);

    printf("\n");
    print_tree(&tree);

    avltree_for_each_remove(&tree, node, tmp) {
        obj = avltree_entry(node, struct obj, node);
        printf("freeing obj %d\n", obj->id);
        free(obj);
    }

    return 0;
}
