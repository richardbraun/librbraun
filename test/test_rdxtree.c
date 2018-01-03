/*
 * Copyright (c) 2011-2017 Richard Braun.
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

#include <stdarg.h>
#include <stdio.h>

#define RDXTREE_ENABLE_NODE_CREATION_FAILURES

#include "../check.h"
#include "../error.h"
#include "../macros.h"
#include "../rdxtree.c"

#define TITLE(str) printf("%s: %s\n", __func__, str)

#if RDXTREE_RADIX < 6
#define BM_FORMAT "%lx"
#elif RDXTREE_RADIX == 6 /* RDXTREE_RADIX < 6 */
#define BM_FORMAT "%llx"
#endif /* RDXTREE_RADIX < 6 */

struct obj {
    rdxtree_key_t id;
};

static void print_subtree(struct rdxtree_node *node, int height, size_t index,
                          size_t level);

static struct obj *
obj_create(rdxtree_key_t id)
{
    struct obj *obj;

    obj = malloc(sizeof(*obj));
    check(obj != NULL);
    obj->id = id;
    return obj;
}

static void
obj_destroy(struct obj *obj)
{
    free(obj);
}

static void
print_value(void *ptr, size_t index, size_t level)
{
    struct obj *obj;
    int i;

    if (ptr == NULL) {
        return;
    }

    obj = ptr;

    for (i = level; i > 0; i--) {
        putchar(' ');
    }

    printf("%zu:%llu\n", index, (unsigned long long)obj->id);
}

static void
print_values(struct rdxtree_node *node, size_t index, size_t level)
{
    size_t i;

    for (i = level; i > 0; i--) {
        putchar(' ');
    }

    printf("%zu:n (bm: " BM_FORMAT ")\n", index, node->alloc_bm);

    for (i = 0; i < ARRAY_SIZE(node->entries); i++) {
        print_value(node->entries[i], i, level + 1);
    }
}

static void
print_node(struct rdxtree_node *node, int height, size_t index, size_t level)
{
    void *entry;
    size_t i;

    for (i = level; i > 0; i--) {
        putchar(' ');
    }

    printf("%zu:n (bm: " BM_FORMAT ")\n", index, node->alloc_bm);

    for (i = 0; i < ARRAY_SIZE(node->entries); i++) {
        entry = rdxtree_entry_addr(node->entries[i]);
        print_subtree(entry, height - 1, i, level + 1);
    }
}

static void
print_subtree(struct rdxtree_node *node, int height, size_t index, size_t level)
{
    if (node == NULL) {
        return;
    }

    if (height == 1) {
        print_values(node, index, level);
    } else {
        print_node(node, height, index, level);
    }
}

static void
print_tree(struct rdxtree *tree)
{
    void *root;

    root = rdxtree_entry_addr(tree->root);

    if (tree->height == 0) {
        print_value(root, 0, 0);
    } else {
        print_subtree(root, tree->height, 0, 0);
    }
}

static void
destroy_tree(struct rdxtree *tree)
{
    struct rdxtree_iter iter;
    struct obj *obj;

    rdxtree_for_each(tree, &iter, obj) {
        check(obj->id == rdxtree_iter_key(&iter));
        obj_destroy(obj);
    }

    rdxtree_remove_all(tree);
}

static void
test_1(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 0");

    rdxtree_init(&tree, 0);
    obj = obj_create(0);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void
test_2(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 1");

    rdxtree_init(&tree, 0);
    obj = obj_create(1);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void
test_3(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 0 and 1");

    rdxtree_init(&tree, 0);
    obj = obj_create(0);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create(1);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void
test_4(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 1 and 0");

    rdxtree_init(&tree, 0);
    obj = obj_create(1);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create(0);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void
test_5(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 0 and 4096");

    rdxtree_init(&tree, 0);
    obj = obj_create(0);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create(4096);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void
test_5_1(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 0, 256 and 4096");

    rdxtree_init(&tree, 0);
    obj = obj_create(0);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create(256);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create(4096);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void
test_5_2(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("insert [0..78], remove 77");

    rdxtree_init(&tree, 0);

    for (i = 0; i <= 78; i++) {
        obj = obj_create(i);
        error = rdxtree_insert(&tree, obj->id, obj);
        check(!error);
    }

    obj = rdxtree_remove(&tree, 77);
    check(obj->id == 77);
    obj_destroy(obj);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void
test_6(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 4096 and 0");

    rdxtree_init(&tree, 0);
    obj = obj_create(4096);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create(0);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void
test_7(void)
{
    struct rdxtree tree;
    struct obj *obj;
    void *ptr;
    int error;

    TITLE("insert and remove 0");

    rdxtree_init(&tree, 0);
    obj = obj_create(0);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    ptr = rdxtree_remove(&tree, obj->id);
    check(ptr == obj);
    obj_destroy(obj);
    print_tree(&tree);
}

static void
test_8(void)
{
    struct rdxtree tree;
    struct obj *obj;
    void *ptr;
    int error;

    TITLE("insert and remove 4096");

    rdxtree_init(&tree, 0);
    obj = obj_create(4096);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    ptr = rdxtree_remove(&tree, obj->id);
    check(ptr == obj);
    obj_destroy(obj);
    print_tree(&tree);
}

static void
test_9(void)
{
    struct rdxtree tree;
    struct obj *obj1, *obj2;
    void *ptr;
    int error;

    TITLE("insert 0 and 4096 and remove in reverse order");

    rdxtree_init(&tree, 0);
    obj1 = obj_create(0);
    error = rdxtree_insert(&tree, obj1->id, obj1);
    check(!error);
    obj2 = obj_create(4096);
    error = rdxtree_insert(&tree, obj2->id, obj2);
    check(!error);
    ptr = rdxtree_remove(&tree, obj2->id);
    check(ptr == obj2);
    obj_destroy(obj2);
    ptr = rdxtree_remove(&tree, obj1->id);
    check(ptr == obj1);
    obj_destroy(obj1);
    print_tree(&tree);
}

static void
test_10(void)
{
    struct rdxtree tree;
    struct obj *obj1, *obj2;
    void *ptr;
    int error;

    TITLE("insert 0 and 4096 and remove in same order");

    rdxtree_init(&tree, 0);
    obj1 = obj_create(0);
    error = rdxtree_insert(&tree, obj1->id, obj1);
    check(!error);
    obj2 = obj_create(4096);
    error = rdxtree_insert(&tree, obj2->id, obj2);
    check(!error);
    ptr = rdxtree_remove(&tree, obj1->id);
    check(ptr == obj1);
    obj_destroy(obj1);
    ptr = rdxtree_remove(&tree, obj2->id);
    check(ptr == obj2);
    obj_destroy(obj2);
    print_tree(&tree);
}

static void
test_11(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("insert [0..4096] and remove in reverse order");

    rdxtree_init(&tree, 0);

    for (i = 0; i <= 4096; i++) {
        obj = obj_create(i);
        error = rdxtree_insert(&tree, i, obj);
        check(!error);
    }

    for (i = 4096; i <= 4096; i--) {
        obj = rdxtree_remove(&tree, i);
        obj_destroy(obj);
    }

    print_tree(&tree);
}

static void
test_12(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("insert [0..4096] and remove in same order");

    rdxtree_init(&tree, 0);

    for (i = 0; i <= 4096; i++) {
        obj = obj_create(i);
        error = rdxtree_insert(&tree, i, obj);
        check(!error);
    }

    for (i = 0; i <= 4096; i++) {
        obj = rdxtree_remove(&tree, i);
        obj_destroy(obj);
    }

    print_tree(&tree);
}

static void
test_13(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("allocate");

    rdxtree_init(&tree, RDXTREE_KEY_ALLOC);
    obj = obj_create(0);
    error = rdxtree_insert_alloc(&tree, obj, &obj->id);
    check(!error);
    check(obj->id == 0);
    print_tree(&tree);
    i = obj->id;
    obj = rdxtree_lookup(&tree, i);
    check(obj->id == i);
    destroy_tree(&tree);
}

static void
test_14(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("insert 0, allocate");

    rdxtree_init(&tree, RDXTREE_KEY_ALLOC);
    obj = obj_create(0);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create(0);
    error = rdxtree_insert_alloc(&tree, obj, &obj->id);
    check(!error);
    check(obj->id == 1);
    print_tree(&tree);
    i = obj->id;
    obj = rdxtree_lookup(&tree, i);
    check(obj->id == i);
    destroy_tree(&tree);
}

static void
test_15(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("insert [0..4095], remove 0, allocate");

    rdxtree_init(&tree, RDXTREE_KEY_ALLOC);

    for (i = 0; i < 4096; i++) {
        obj = obj_create(i);
        error = rdxtree_insert(&tree, i, obj);
        check(!error);
    }

    obj = rdxtree_remove(&tree, 0);
    check(obj->id == 0);
    error = rdxtree_insert_alloc(&tree, obj, &obj->id);
    check(!error);
    check(obj->id == 0);
    destroy_tree(&tree);
}

static void
test_16(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("insert [0..4095], remove 1, allocate");

    rdxtree_init(&tree, RDXTREE_KEY_ALLOC);

    for (i = 0; i < 4096; i++) {
        obj = obj_create(i);
        error = rdxtree_insert(&tree, i, obj);
        check(!error);
    }

    obj = rdxtree_remove(&tree, 1);
    check(obj->id == 1);
    error = rdxtree_insert_alloc(&tree, obj, &obj->id);
    check(!error);
    check(obj->id == 1);
    destroy_tree(&tree);
}

static void
test_17(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("insert [0..63] and [128..191], allocate x65");

    rdxtree_init(&tree, RDXTREE_KEY_ALLOC);

    for (i = 0; i < 64; i++) {
        obj = obj_create(i);
        error = rdxtree_insert(&tree, i, obj);
        check(!error);
    }

    for (i = 128; i < 192; i++) {
        obj = obj_create(i);
        error = rdxtree_insert(&tree, i, obj);
        check(!error);
    }

    for (i = 64; i < 128; i++) {
        obj = obj_create(0);
        error = rdxtree_insert_alloc(&tree, obj, &obj->id);
        check(!error);
        check(obj->id == i);
    }

    obj = obj_create(0);
    error = rdxtree_insert_alloc(&tree, obj, &obj->id);
    check(!error);
    check(obj->id == 192);
    destroy_tree(&tree);
}

static void
test_18(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("insert [0..4095], allocate");

    rdxtree_init(&tree, RDXTREE_KEY_ALLOC);

    for (i = 0; i < 4096; i++) {
        obj = obj_create(i);
        error = rdxtree_insert(&tree, i, obj);
        check(!error);
    }

    obj = obj_create(0);
    error = rdxtree_insert_alloc(&tree, obj, &obj->id);
    check(!error);
    check(obj->id == 4096);
    destroy_tree(&tree);
}

static void
test_19(void)
{
    struct rdxtree tree;
    struct obj *obj1, *obj2, *tmp;
    void **slot;
    int error;

    TITLE("insert 0, replace");

    rdxtree_init(&tree, 0);
    obj1 = obj_create(0);
    error = rdxtree_insert(&tree, 0, obj1);
    check(!error);
    slot = rdxtree_lookup_slot(&tree, 0);
    check(slot != NULL);
    obj2 = obj_create(0);
    tmp = rdxtree_replace_slot(slot, obj2);
    check(obj1 == tmp);
    obj_destroy(obj1);
    print_tree(&tree);
    tmp = rdxtree_lookup(&tree, 0);
    check(obj2 == tmp);
    destroy_tree(&tree);
}

static void
test_20(void)
{
    struct rdxtree tree;
    struct obj *obj1, *obj2, *tmp;
    void **slot;
    int error;

    TITLE("insert 4096, replace");

    rdxtree_init(&tree, 0);
    obj1 = obj_create(4096);
    error = rdxtree_insert(&tree, 4096, obj1);
    check(!error);
    slot = rdxtree_lookup_slot(&tree, 4096);
    check(slot != NULL);
    obj2 = obj_create(4096);
    tmp = rdxtree_replace_slot(slot, obj2);
    check(obj1 == tmp);
    obj_destroy(obj1);
    print_tree(&tree);
    tmp = rdxtree_lookup(&tree, 4096);
    check(obj2 == tmp);
    destroy_tree(&tree);
}

static void
test_21(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 0, insert again");

    rdxtree_init(&tree, 0);
    obj = obj_create(0);
    error = rdxtree_insert(&tree, 0, obj);
    check(!error);
    error = rdxtree_insert(&tree, 0, obj);
    check(error == ERROR_BUSY);
    destroy_tree(&tree);
}

static void
test_22(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 123, insert again");

    rdxtree_init(&tree, 0);
    obj = obj_create(123);
    error = rdxtree_insert(&tree, 123, obj);
    check(!error);
    error = rdxtree_insert(&tree, 123, obj);
    check(error == ERROR_BUSY);
    destroy_tree(&tree);
}

static void
test_23(void)
{
    struct rdxtree tree;
    struct obj *obj;
    void **slot;
    int error;

    TITLE("insert_slot 0, check slot");

    rdxtree_init(&tree, 0);
    obj = obj_create(0);
    error = rdxtree_insert_slot(&tree, obj->id, obj, &slot);
    check(!error);
    check(*slot == obj);
    destroy_tree(&tree);
}

static void
test_24(void)
{
    struct rdxtree tree;
    struct obj *obj;
    void **slot;
    int error;

    TITLE("insert_slot 321, check slot");

    rdxtree_init(&tree, 0);
    obj = obj_create(321);
    error = rdxtree_insert_slot(&tree, obj->id, obj, &slot);
    check(!error);
    check(*slot == obj);
    destroy_tree(&tree);
}

static void
test_25(void)
{
    struct rdxtree tree;
    struct obj *obj;
    void **slot;
    rdxtree_key_t i;
    int error;

    TITLE("insert_alloc_slot x3");

    rdxtree_init(&tree, RDXTREE_KEY_ALLOC);

    for (i = 0; i < 3; i++) {
        obj = obj_create(0);
        error = rdxtree_insert_alloc_slot(&tree, obj, &obj->id, &slot);
        check(!error);
        check(obj->id == i);
        check(*slot == obj);
    }

    destroy_tree(&tree);
}

static void
test_26(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("insert [0..62], remove 63");

    rdxtree_init(&tree, 0);

    for (i = 0; i < 62; i++) {
        obj = obj_create(i);
        error = rdxtree_insert(&tree, obj->id, obj);
        check(!error);
    }

    obj = rdxtree_remove(&tree, 63);
    check(obj == NULL);

    destroy_tree(&tree);
}

static void
test_27(void)
{
    struct rdxtree tree;
    struct obj *obj;
    rdxtree_key_t i;
    int error;

    TITLE("insert [0..63], remove 64");

    rdxtree_init(&tree, 0);

    for (i = 0; i < 64; i++) {
        obj = obj_create(i);
        error = rdxtree_insert(&tree, obj->id, obj);
        check(!error);
    }

    obj = rdxtree_remove(&tree, 64);
    check(obj == NULL);

    destroy_tree(&tree);
}

static void
test_28(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 60000, remove 1");

    rdxtree_init(&tree, 0);
    obj = obj_create(60000);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = rdxtree_remove(&tree, 1);
    check(obj == NULL);
    destroy_tree(&tree);
}

static void
test_29(void)
{
    struct rdxtree tree;
    struct obj *obj;

    TITLE("empty tree, lookup 0");

    rdxtree_init(&tree, 0);
    obj = rdxtree_lookup(&tree, 0);
    check(obj == NULL);
}

static void
test_30(void)
{
    struct rdxtree tree;
    struct obj *obj;

    TITLE("empty tree, lookup 10");

    rdxtree_init(&tree, 0);
    obj = rdxtree_lookup(&tree, 10);
    check(obj == NULL);
}

static void
test_31(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 60000, lookup 1");

    rdxtree_init(&tree, 0);
    obj = obj_create(60000);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = rdxtree_lookup(&tree, 1);
    check(obj == NULL);
    destroy_tree(&tree);
}

static void
test_32(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 60001, lookup 60000");

    rdxtree_init(&tree, 0);
    obj = obj_create(60001);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = rdxtree_lookup(&tree, 60000);
    check(obj == NULL);
    destroy_tree(&tree);
}

extern unsigned int rdxtree_fail_node_creation_threshold;
extern unsigned int rdxtree_nr_node_creations;

static void
test_33(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("prepare node creation failure at 1, insert 1");

    rdxtree_fail_node_creation_threshold = 1;
    rdxtree_nr_node_creations = 0;

    rdxtree_init(&tree, 0);
    obj = obj_create(1);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(error == ERROR_NOMEM);
    obj_destroy(obj);
    print_tree(&tree);
}

static void
test_34(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("prepare node creation failure at 2, insert 64");

    rdxtree_fail_node_creation_threshold = 2;
    rdxtree_nr_node_creations = 0;

    rdxtree_init(&tree, 0);
    obj = obj_create(64);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(error == ERROR_NOMEM);
    obj_destroy(obj);
    print_tree(&tree);
}

static void
test_35(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("prepare node creation failure at 2, insert 0 and 64");

    rdxtree_fail_node_creation_threshold = 2;
    rdxtree_nr_node_creations = 0;

    rdxtree_init(&tree, 0);
    obj = obj_create(0);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create(64);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(error == ERROR_NOMEM);
    obj_destroy(obj);
    print_tree(&tree);
    destroy_tree(&tree);
}

static void
test_36(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("prepare node creation failure at 2, insert 1 and 64");

    rdxtree_fail_node_creation_threshold = 2;
    rdxtree_nr_node_creations = 0;

    rdxtree_init(&tree, 0);
    obj = obj_create(1);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create(64);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(error == ERROR_NOMEM);
    obj_destroy(obj);
    print_tree(&tree);
    destroy_tree(&tree);
}

#ifndef RDXTREE_KEY_32
static void
test_37(void)
{
    struct rdxtree tree;
    struct obj *obj;
    void *ptr;
    int error;

    TITLE("insert and remove 4294967296");

    rdxtree_init(&tree, 0);
    obj = obj_create(4294967296);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    ptr = rdxtree_remove(&tree, obj->id);
    check(ptr == obj);
    obj_destroy(obj);
    print_tree(&tree);
}
#endif /* RDXTREE_KEY_32 */

static void
test_38(void)
{
    struct rdxtree tree;
    struct obj *obj;
    int error;

    TITLE("insert 1, 3 and max_key");

    rdxtree_init(&tree, 0);
    obj = obj_create(1);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create(3);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    obj = obj_create((rdxtree_key_t)-1);
    error = rdxtree_insert(&tree, obj->id, obj);
    check(!error);
    print_tree(&tree);
    destroy_tree(&tree);
}

int
main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    test_1();
    test_2();
    test_3();
    test_4();
    test_5();
    test_5_1();
    test_5_2();
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
    test_21();
    test_22();
    test_23();
    test_24();
    test_25();
    test_26();
    test_27();
    test_28();
    test_29();
    test_30();
    test_31();
    test_32();
    test_33();
    test_34();
    test_35();
    test_36();
#ifndef RDXTREE_KEY_32
    test_37();
#endif /* RDXTREE_KEY_32 */
    test_38();
    return 0;
}
