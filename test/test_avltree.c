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

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../hash.h"
#include "../macros.h"
#include "../avltree.h"

#define SIZE 28

struct obj {
    struct avltree_node node;
    int id;
};

static inline int obj_cmp_lookup(int id, struct avltree_node *node)
{
    struct obj *obj;

    obj = avltree_entry(node, struct obj, node);
    return id - obj->id;
}

static void print_subtree(struct avltree_node *node, int level)
{
    struct obj *obj;
    char balance;
    int i;

    if (node == NULL)
        return;

    print_subtree(node->children[AVLTREE_RIGHT], level + 1);

    for (i = level; i > 0; i--)
        putchar(' ');

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

static void print_tree(struct avltree *tree)
{
    print_subtree(tree->root, 0);
}

static int get_id(int i)
{
    return hash_int32(i, 6);
}

int main(int argc, char *argv[])
{
    struct avltree tree;
    struct avltree_node *node, *tmp;
    struct obj *obj;
    unsigned long slot;
    int i, id;

    (void)argc;
    (void)argv;

    avltree_init(&tree);

    for (i = 0; i < SIZE; i++) {
        id = get_id(i);
        node = avltree_lookup_slot(&tree, id, obj_cmp_lookup, slot);

        if (node != NULL)
            continue;

        obj = malloc(sizeof(*obj));
        obj->id = id;
        printf("%d ", obj->id);
        avltree_insert_slot(&tree, slot, &obj->node);
    }

    printf("\n");
    print_tree(&tree);

    avltree_for_each_remove(&tree, node, tmp) {
        obj = avltree_entry(node, struct obj, node);
        printf("freeing obj %d\n", obj->id);
        free(obj);
    }

    return 0;
}
