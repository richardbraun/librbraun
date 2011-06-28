/*
 * Copyright (c) 2011 Richard Braun.
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
#include <stdarg.h>

#include "../macros.h"
#include "../rdatree.c"

#define TITLE(str) printf("%s: %s\n", __func__, str)

#if RDATREE_RADIX < 6
#define BM_FORMAT "%lx"
#elif RDATREE_RADIX == 6 /* RDATREE_RADIX < 6 */
#ifdef __LP64__
#define BM_FORMAT "%lx"
#else /* __LP64__ */
#define BM_FORMAT "%llx"
#endif /* __LP64__ */
#endif /* RDATREE_RADIX < 6 */

struct obj {
    unsigned long id;
};

static struct obj * obj_create(unsigned long id)
{
    struct obj *obj;

    obj = malloc(sizeof(*obj));
    assert(obj != NULL);
    obj->id = id;
    return obj;
}

static void obj_destroy(struct obj *obj)
{
    free(obj);
}

static void print_subtree(struct rdatree_node *node, int height, size_t index,
                          size_t level);

static void print_value(void *ptr, size_t index, size_t level)
{
    struct obj *obj;
    int i;

    if (ptr == NULL)
        return;

    obj = ptr;

    for (i = level; i > 0; i--)
        putchar(' ');

    printf("%zu:%lu\n", index, obj->id);
}

static void print_values(struct rdatree_node *node, size_t index, size_t level)
{
    size_t i;

    for (i = level; i > 0; i--)
        putchar(' ');

    printf("%zu:n (bm: " BM_FORMAT ")\n", index, node->alloc_bm);

    for (i = 0; i < ARRAY_SIZE(node->slots); i++)
        print_value(node->slots[i], i, level + 1);
}

static void print_node(struct rdatree_node *node, int height, size_t index,
                       size_t level)
{
    size_t i;

    for (i = level; i > 0; i--)
        putchar(' ');

    printf("%zu:n (bm: " BM_FORMAT ")\n", index, node->alloc_bm);

    for (i = 0; i < ARRAY_SIZE(node->slots); i++)
        print_subtree(node->slots[i], height - 1, i, level + 1);
}

static void print_subtree(struct rdatree_node *node, int height, size_t index,
                          size_t level)
{
    if (node == NULL)
        return;

    if (height == 1)
        print_values(node, index, level);
    else
        print_node(node, height, index, level);
}

static void print_tree(struct rdatree *tree)
{
    if (tree->height == 0)
        print_value(tree->root, 0, 0);
    else
        print_subtree(tree->root, tree->height, 0, 0);
}

static void destroy_tree(struct rdatree *tree)
{
    struct rdatree_iter iter;
    struct obj *obj;

    rdatree_for_each(tree, &iter, obj)
        obj_destroy(obj);

    rdatree_remove_all(tree);
}

static void test_1(void)
{
    struct rdatree tree;
    struct obj *obj;
    int error;

    TITLE("insert 0");

    rdatree_init(&tree);
    obj = obj_create(0);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void test_2(void)
{
    struct rdatree tree;
    struct obj *obj;
    int error;

    TITLE("insert 1");

    rdatree_init(&tree);
    obj = obj_create(1);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void test_3(void)
{
    struct rdatree tree;
    struct obj *obj;
    int error;

    TITLE("insert 0 and 1");

    rdatree_init(&tree);
    obj = obj_create(0);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    obj = obj_create(1);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void test_4(void)
{
    struct rdatree tree;
    struct obj *obj;
    int error;

    TITLE("insert 1 and 0");

    rdatree_init(&tree);
    obj = obj_create(1);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    obj = obj_create(0);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void test_5(void)
{
    struct rdatree tree;
    struct obj *obj;
    int error;

    TITLE("insert 0 and 4096");

    rdatree_init(&tree);
    obj = obj_create(4096);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    obj = obj_create(0);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void test_6(void)
{
    struct rdatree tree;
    struct obj *obj;
    int error;

    TITLE("insert 4096 and 0");

    rdatree_init(&tree);
    obj = obj_create(4096);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    obj = obj_create(0);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void test_7(void)
{
    struct rdatree tree;
    struct obj *obj;
    void *ptr;
    int error;

    TITLE("insert and remove 0");

    rdatree_init(&tree);
    obj = obj_create(0);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    ptr = rdatree_remove(&tree, obj->id);
    assert(ptr == obj);
    obj_destroy(obj);
    print_tree(&tree);
}

static void test_8(void)
{
    struct rdatree tree;
    struct obj *obj;
    void *ptr;
    int error;

    TITLE("insert and remove 4096");

    rdatree_init(&tree);
    obj = obj_create(4096);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    ptr = rdatree_remove(&tree, obj->id);
    assert(ptr == obj);
    obj_destroy(obj);
    print_tree(&tree);
}

static void test_9(void)
{
    struct rdatree tree;
    struct obj *obj1, *obj2;
    void *ptr;
    int error;

    TITLE("insert 0 and 4096 and remove in reverse order");

    rdatree_init(&tree);
    obj1 = obj_create(0);
    error = rdatree_insert(&tree, obj1->id, obj1);
    assert(!error);
    obj2 = obj_create(4096);
    error = rdatree_insert(&tree, obj2->id, obj2);
    assert(!error);
    ptr = rdatree_remove(&tree, obj2->id);
    assert(ptr == obj2);
    obj_destroy(obj2);
    ptr = rdatree_remove(&tree, obj1->id);
    assert(ptr == obj1);
    obj_destroy(obj1);
    print_tree(&tree);
}

static void test_10(void)
{
    struct rdatree tree;
    struct obj *obj1, *obj2;
    void *ptr;
    int error;

    TITLE("insert 0 and 4096 and remove in same order");

    rdatree_init(&tree);
    obj1 = obj_create(0);
    error = rdatree_insert(&tree, obj1->id, obj1);
    assert(!error);
    obj2 = obj_create(4096);
    error = rdatree_insert(&tree, obj2->id, obj2);
    assert(!error);
    ptr = rdatree_remove(&tree, obj1->id);
    assert(ptr == obj1);
    obj_destroy(obj1);
    ptr = rdatree_remove(&tree, obj2->id);
    assert(ptr == obj2);
    obj_destroy(obj2);
    print_tree(&tree);
}

static void test_11(void)
{
    struct rdatree tree;
    struct obj *obj;
    unsigned long i;
    int error;

    TITLE("insert [0..4096] and remove in reverse order");

    rdatree_init(&tree);

    for (i = 0; i <= 4096; i++) {
        obj = obj_create(i);
        error = rdatree_insert(&tree, i, obj);
        assert(!error);
    }

    for (i = 4096; i <= 4096; i--) {
        obj = rdatree_remove(&tree, i);
        obj_destroy(obj);
    }

    print_tree(&tree);
}

static void test_12(void)
{
    struct rdatree tree;
    struct obj *obj;
    unsigned long i;
    int error;

    TITLE("insert [0..4096] and remove in same order");

    rdatree_init(&tree);

    for (i = 0; i <= 4096; i++) {
        obj = obj_create(i);
        error = rdatree_insert(&tree, i, obj);
        assert(!error);
    }

    for (i = 0; i <= 4096; i++) {
        obj = rdatree_remove(&tree, i);
        obj_destroy(obj);
    }

    print_tree(&tree);
}

static void test_13(void)
{
    struct rdatree tree;
    struct obj *obj;
    unsigned long i;
    int error;

    TITLE("allocate");

    rdatree_init(&tree);
    obj = obj_create(0);
    error = rdatree_insert_alloc(&tree, obj, &obj->id);
    assert(!error);
    assert(obj->id == 0);
    print_tree(&tree);
    i = obj->id;
    obj = rdatree_lookup(&tree, i);
    assert(obj->id == i);
    destroy_tree(&tree);
}

static void test_14(void)
{
    struct rdatree tree;
    struct obj *obj;
    unsigned long i;
    int error;

    TITLE("insert 0, allocate");

    rdatree_init(&tree);
    obj = obj_create(0);
    error = rdatree_insert(&tree, obj->id, obj);
    assert(!error);
    obj = obj_create(0);
    error = rdatree_insert_alloc(&tree, obj, &obj->id);
    assert(!error);
    assert(obj->id == 1);
    print_tree(&tree);
    i = obj->id;
    obj = rdatree_lookup(&tree, i);
    assert(obj->id == i);
    destroy_tree(&tree);
}

static void test_15(void)
{
    struct rdatree tree;
    struct obj *obj;
    unsigned long i;
    int error;

    TITLE("insert [0..4095], remove 0, allocate");

    rdatree_init(&tree);

    for (i = 0; i < 4096; i++) {
        obj = obj_create(i);
        error = rdatree_insert(&tree, i, obj);
        assert(!error);
    }

    obj = rdatree_remove(&tree, 0);
    assert(obj->id == 0);
    error = rdatree_insert_alloc(&tree, obj, &obj->id);
    assert(!error);
    assert(obj->id == 0);
    destroy_tree(&tree);
}

static void test_16(void)
{
    struct rdatree tree;
    struct obj *obj;
    unsigned long i;
    int error;

    TITLE("insert [0..4095], remove 1, allocate");

    rdatree_init(&tree);

    for (i = 0; i < 4096; i++) {
        obj = obj_create(i);
        error = rdatree_insert(&tree, i, obj);
        assert(!error);
    }

    obj = rdatree_remove(&tree, 1);
    assert(obj->id == 1);
    error = rdatree_insert_alloc(&tree, obj, &obj->id);
    assert(!error);
    assert(obj->id == 1);
    destroy_tree(&tree);
}

static void test_17(void)
{
    struct rdatree tree;
    struct obj *obj;
    unsigned long i;
    int error;

    TITLE("insert [0..63] and [128..191], allocate x65");

    rdatree_init(&tree);

    for (i = 0; i < 64; i++) {
        obj = obj_create(i);
        error = rdatree_insert(&tree, i, obj);
        assert(!error);
    }

    for (i = 128; i < 192; i++) {
        obj = obj_create(i);
        error = rdatree_insert(&tree, i, obj);
        assert(!error);
    }

    for (i = 64; i < 128; i++) {
        obj = obj_create(0);
        error = rdatree_insert_alloc(&tree, obj, &obj->id);
        assert(!error);
        assert(obj->id == i);
    }

    obj = obj_create(0);
    error = rdatree_insert_alloc(&tree, obj, &obj->id);
    assert(!error);
    assert(obj->id == 192);
    destroy_tree(&tree);
}

static void test_18(void)
{
    struct rdatree tree;
    struct obj *obj;
    unsigned long i;
    int error;

    TITLE("insert [0..4095], allocate");

    rdatree_init(&tree);

    for (i = 0; i < 4096; i++) {
        obj = obj_create(i);
        error = rdatree_insert(&tree, i, obj);
        assert(!error);
    }

    obj = obj_create(0);
    error = rdatree_insert_alloc(&tree, obj, &obj->id);
    assert(!error);
    assert(obj->id == 4096);
    destroy_tree(&tree);
}

static void test_19(void)
{
    struct rdatree tree;
    struct obj *obj1, *obj2, *tmp;
    void **slot;
    int error;

    TITLE("insert 0, replace");

    rdatree_init(&tree);
    obj1 = obj_create(0);
    error = rdatree_insert(&tree, 0, obj1);
    assert(!error);
    slot = rdatree_lookup_slot(&tree, 0);
    assert(slot != NULL);
    obj2 = obj_create(0);
    tmp = rdatree_replace_slot(slot, obj2);
    assert(obj1 == tmp);
    obj_destroy(obj1);
    print_tree(&tree);
    tmp = rdatree_lookup(&tree, 0);
    assert(obj2 == tmp);
    destroy_tree(&tree);
}

static void test_20(void)
{
    struct rdatree tree;
    struct obj *obj1, *obj2, *tmp;
    void **slot;
    int error;

    TITLE("insert 4096, replace");

    rdatree_init(&tree);
    obj1 = obj_create(4096);
    error = rdatree_insert(&tree, 4096, obj1);
    assert(!error);
    slot = rdatree_lookup_slot(&tree, 4096);
    assert(slot != NULL);
    obj2 = obj_create(4096);
    tmp = rdatree_replace_slot(slot, obj2);
    assert(obj1 == tmp);
    obj_destroy(obj1);
    print_tree(&tree);
    tmp = rdatree_lookup(&tree, 4096);
    assert(obj2 == tmp);
    destroy_tree(&tree);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
    test_6();
    test_7();
    test_8();
    test_9();
    test_10();
    test_11();
    test_12();
    test_13();
    test_14();
    test_15();
    test_16();
    test_17();
    test_18();
    test_19();
    test_20();
    return 0;
}
