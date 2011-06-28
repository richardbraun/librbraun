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

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "macros.h"
#include "rdatree.h"

/*
 * Global properties used to shape radix trees.
 */
#define RDATREE_RADIX       6
#define RDATREE_RADIX_SIZE  (1UL << RDATREE_RADIX)
#define RDATREE_RADIX_MASK  (RDATREE_RADIX_SIZE - 1)

#if RDATREE_RADIX < 6
typedef unsigned long rdatree_bm_t;
#define rdatree_ffs(x) __builtin_ffsl(x)
#elif RDATREE_RADIX == 6 /* RDATREE_RADIX < 6 */
#ifdef __LP64__
typedef unsigned long rdatree_bm_t;
#define rdatree_ffs(x) __builtin_ffsl(x)
#else /* __LP64__ */
typedef unsigned long long rdatree_bm_t;
#define rdatree_ffs(x) __builtin_ffsll(x)
#endif /* __LP64__ */
#else /* RDATREE_RADIX < 6 */
#error "radix too high"
#endif /* RDATREE_RADIX < 6 */

/*
 * Allocation bitmap size in bits.
 */
#define RDATREE_BM_SIZE (sizeof(rdatree_bm_t) * CHAR_BIT)

/*
 * Empty/full allocation bitmap words.
 */
#define RDATREE_BM_EMPTY    ((rdatree_bm_t)0)
#define RDATREE_BM_FULL \
    ((~(rdatree_bm_t)0) >> (RDATREE_BM_SIZE - RDATREE_RADIX_SIZE))

/*
 * Radix tree node.
 *
 * The index member can only be used if the parent isn't null. Concerning the
 * allocation bitmap, a bit is set when the node it denotes, or one of its
 * children, can be used to allocate a slot. Similarly, a bit is clear when
 * the matching node and all of its children have no free slot.
 */
struct rdatree_node {
    struct rdatree_node *parent;
    unsigned int index;
    unsigned int nr_slots;
    rdatree_bm_t alloc_bm;
    void *slots[RDATREE_RADIX_SIZE];
};

static struct rdatree_node * rdatree_node_create(void)
{
    struct rdatree_node *node;

    node = malloc(sizeof(*node));

    if (node == NULL)
        return NULL;

    node->parent = NULL;
    node->nr_slots = 0;
    node->alloc_bm = RDATREE_BM_FULL;
    memset(node->slots, 0, sizeof(node->slots));
    return node;
}

static void rdatree_node_destroy(struct rdatree_node *node)
{
    free(node);
}

static inline void rdatree_node_link(struct rdatree_node *node,
                                     struct rdatree_node *parent,
                                     unsigned int index)
{
    node->parent = parent;
    node->index = index;
}

static inline void rdatree_node_unlink(struct rdatree_node *node)
{
    assert(node->parent != NULL);
    node->parent = NULL;
}

static inline int rdatree_node_full(struct rdatree_node *node)
{
    return (node->nr_slots == ARRAY_SIZE(node->slots));
}

static inline int rdatree_node_empty(struct rdatree_node *node)
{
    return (node->nr_slots == 0);
}

static inline void rdatree_node_insert(struct rdatree_node *node,
                                       unsigned int index, void *ptr)
{
    assert(node->slots[index] == NULL);

    node->nr_slots++;
    node->slots[index] = ptr;
}

static inline void rdatree_node_remove(struct rdatree_node *node,
                                       unsigned int index)
{
    assert(node->slots[index] != NULL);

    node->nr_slots--;
    node->slots[index] = NULL;
}

static inline void ** rdatree_node_find(struct rdatree_node *node,
                                        unsigned int index)
{
    while (index < ARRAY_SIZE(node->slots)) {
        if (node->slots[index] != NULL)
            return &node->slots[index];

        index++;
    }

    return NULL;
}

static inline void rdatree_node_bm_set(struct rdatree_node *node,
                                       unsigned int index)
{
    node->alloc_bm |= (rdatree_bm_t)1 << index;
}

static inline void rdatree_node_bm_clear(struct rdatree_node *node,
                                         unsigned int index)
{
    node->alloc_bm &= ~((rdatree_bm_t)1 << index);
}

static inline int rdatree_node_bm_is_set(struct rdatree_node *node,
                                         unsigned int index)
{
    return (node->alloc_bm & ((rdatree_bm_t)1 << index));
}

static inline int rdatree_node_bm_empty(struct rdatree_node *node)
{
    return (node->alloc_bm == RDATREE_BM_EMPTY);
}

static inline int rdatree_node_bm_first(struct rdatree_node *node)
{
    return rdatree_ffs(node->alloc_bm) - 1;
}

static inline unsigned long rdatree_max_key(int height)
{
    unsigned int shift;

    shift = RDATREE_RADIX * height;

    if (shift >= (sizeof(unsigned long) * CHAR_BIT))
        return ~0UL;
    else
        return (1 << shift) - 1;
}

static void rdatree_shrink(struct rdatree *tree)
{
    struct rdatree_node *node, *child;

    while (tree->height > 0) {
        node = tree->root;

        if (node->nr_slots != 1)
            break;

        child = node->slots[0];

        if (child == NULL)
            break;

        rdatree_node_destroy(node);
        tree->height--;

        if (tree->height > 0)
            rdatree_node_unlink(child);

        tree->root = child;
    }
}

static int rdatree_grow(struct rdatree *tree, unsigned long key)
{
    struct rdatree_node *node;
    int new_height;

    new_height = tree->height + 1;

    while (key > rdatree_max_key(new_height))
        new_height++;

    if (tree->root == NULL) {
        tree->height = new_height;
        return ERR_SUCCESS;
    }

    do {
        node = rdatree_node_create();

        if (node == NULL) {
            rdatree_shrink(tree);
            return ERR_NOMEM;
        }

        rdatree_node_insert(node, 0, tree->root);

        if (tree->height == 0)
            rdatree_node_bm_clear(node, 0);
        else {
            rdatree_node_link(tree->root, node, 0);

            if (rdatree_node_bm_empty(tree->root))
                rdatree_node_bm_clear(node, 0);
        }

        tree->root = node;
        tree->height++;
    } while (new_height > tree->height);

    return ERR_SUCCESS;
}

static void rdatree_cleanup(struct rdatree *tree, struct rdatree_node *node)
{
    struct rdatree_node *prev;

    for (;;) {
        if (!rdatree_node_empty(node)) {
            if (node->parent == NULL)
                rdatree_shrink(tree);

            break;
        }

        if (node->parent == NULL) {
            rdatree_node_destroy(node);
            tree->height = 0;
            tree->root = NULL;
            break;
        }

        prev = node;
        node = node->parent;
        rdatree_node_remove(node, prev->index);
        rdatree_node_destroy(prev);
    }
}

static void rdatree_insert_bm_clear(struct rdatree_node *node,
                                    unsigned int index)
{
    for (;;) {
        rdatree_node_bm_clear(node, index);

        if ((node->parent == NULL) || !rdatree_node_full(node))
            break;

        index = node->index;
        node = node->parent;
    }
}

int rdatree_insert(struct rdatree *tree, unsigned long key, void *ptr)
{
    struct rdatree_node *node, *prev;
    unsigned int index = index;
    int error, height, shift;

    assert(ptr != NULL);

    if (key > rdatree_max_key(tree->height)) {
        error = rdatree_grow(tree, key);

        if (error)
            return error;
    }

    height = tree->height;

    if (height == 0) {
        if (tree->root != NULL)
            return ERR_BUSY;

        tree->root = ptr;
        return ERR_SUCCESS;
    }

    node = tree->root;
    shift = (height - 1) * RDATREE_RADIX;
    prev = NULL;

    do {
        if (node == NULL) {
            node = rdatree_node_create();

            if (node == NULL) {
                if (prev == NULL)
                    tree->height = 0;
                else
                    rdatree_cleanup(tree, prev);

                return ERR_NOMEM;
            }

            if (prev == NULL)
                tree->root = node;
            else {
                rdatree_node_insert(prev, index, node);
                rdatree_node_link(node, prev, index);
            }
        }

        prev = node;
        index = (key >> shift) & RDATREE_RADIX_MASK;
        node = prev->slots[index];
        shift -= RDATREE_RADIX;
        height--;
    } while (height > 0);

    if (node != NULL)
        return ERR_BUSY;

    rdatree_node_insert(prev, index, ptr);
    rdatree_insert_bm_clear(prev, index);
    return ERR_SUCCESS;
}

int rdatree_insert_alloc(struct rdatree *tree, void *ptr, unsigned long *keyp)
{
    struct rdatree_node *node, *prev;
    unsigned long key;
    int error, height, shift, index = index;

    assert(ptr != NULL);

    height = tree->height;

    if (height == 0) {
        if (tree->root == NULL) {
            tree->root = ptr;
            *keyp = 0;
            return ERR_SUCCESS;
        }

        goto grow;
    }

    node = tree->root;
    key = 0;
    shift = (height - 1) * RDATREE_RADIX;
    prev = NULL;

    do {
        if (node == NULL) {
            node = rdatree_node_create();

            if (node == NULL) {
                rdatree_cleanup(tree, prev);
                return ERR_NOMEM;
            }

            rdatree_node_insert(prev, index, node);
            rdatree_node_link(node, prev, index);
        }

        prev = node;
        index = rdatree_node_bm_first(node);

        if (index == -1)
            goto grow;

        key |= ((unsigned long)index << shift);
        node = node->slots[index];
        shift -= RDATREE_RADIX;
        height--;
    } while (height > 0);

    rdatree_node_insert(prev, index, ptr);
    rdatree_insert_bm_clear(prev, index);
    goto out;

grow:
    key = rdatree_max_key(height) + 1;
    error = rdatree_insert(tree, key, ptr);

    if (error)
        return error;

out:
    *keyp = key;
    return ERR_SUCCESS;
}

static void rdatree_remove_bm_set(struct rdatree_node *node,
                                  unsigned int index)
{
    do {
        rdatree_node_bm_set(node, index);

        if (node->parent == NULL)
            break;

        index = node->index;
        node = node->parent;
    } while (!rdatree_node_bm_is_set(node, index));
}

void * rdatree_remove(struct rdatree *tree, unsigned long key)
{
    struct rdatree_node *node, *prev;
    unsigned int index;
    int height, shift;

    height = tree->height;

    if (key > rdatree_max_key(height))
        return NULL;

    node = tree->root;

    if (height == 0) {
        tree->root = NULL;
        return node;
    }

    shift = (height - 1) * RDATREE_RADIX;

    do {
        if (node == NULL)
            return NULL;

        prev = node;
        index = (key >> shift) & RDATREE_RADIX_MASK;
        node = node->slots[index];
        shift -= RDATREE_RADIX;
        height--;
    } while (height > 0);

    if (node == NULL)
        return NULL;

    rdatree_node_remove(prev, index);
    rdatree_remove_bm_set(prev, index);
    rdatree_cleanup(tree, prev);
    return node;
}

static void ** rdatree_lookup_prim(struct rdatree *tree, unsigned long key)
{
    struct rdatree_node *node, *prev;
    unsigned int index;
    int height, shift;

    height = tree->height;

    if (key > rdatree_max_key(height))
        return NULL;

    if (height == 0) {
        if (tree->root == NULL)
            return NULL;

        return &tree->root;
    }

    node = tree->root;
    shift = (height - 1) * RDATREE_RADIX;

    do {
        if (node == NULL)
            return NULL;

        prev = node;
        index = (key >> shift) & RDATREE_RADIX_MASK;
        node = node->slots[index];
        shift -= RDATREE_RADIX;
        height--;
    } while (height > 0);

    if (node == NULL)
        return NULL;

    return &prev->slots[index];
}

void * rdatree_lookup(struct rdatree *tree, unsigned long key)
{
    void **slot;

    slot = rdatree_lookup_prim(tree, key);
    return (slot == NULL) ? NULL : *slot;
}

void ** rdatree_lookup_slot(struct rdatree *tree, unsigned long key)
{
    return rdatree_lookup_prim(tree, key);
}

void * rdatree_replace_slot(void **slot, void *ptr)
{
    void *old;

    assert(ptr != NULL);

    old = *slot;
    assert(old != NULL);
    *slot = ptr;
    return old;
}

static struct rdatree_node * rdatree_walk(struct rdatree *tree,
                                          struct rdatree_node *node)
{
    struct rdatree_node *prev;
    void **slot;
    unsigned int index;
    int height;

    if (node == NULL) {
        height = tree->height;
        node = tree->root;

        while (height > 1) {
            slot = rdatree_node_find(node, 0);
            node = *slot;
            height--;
        }

        return node;
    }

    height = 0;

    for (;;) {
        prev = node->parent;

        if (prev == NULL)
            return NULL;

        index = node->index;
        slot = rdatree_node_find(prev, index + 1);

        if (slot != NULL)
            break;

        height++;
        node = prev;
    }

    node = *slot;

    while (height > 0) {
        slot = rdatree_node_find(node, 0);
        node = *slot;
        height--;
    }

    return node;
}

void * rdatree_iter_next(struct rdatree *tree, struct rdatree_iter *iter)
{
    unsigned int index;

    if (tree->height == 0) {
        if (iter->slot != NULL)
            return NULL;

        iter->slot = &tree->root;
        return *iter->slot;
    }

    if (iter->node != NULL) {
        index = iter->slot - ((struct rdatree_node *)iter->node)->slots;
        iter->slot = rdatree_node_find(iter->node, index + 1);
    }

    if (iter->slot == NULL) {
        iter->node = rdatree_walk(tree, iter->node);

        if (iter->node != NULL)
            iter->slot = rdatree_node_find(iter->node, 0);
    }

    if (iter->slot == NULL)
        return NULL;

    return *iter->slot;
}

void rdatree_remove_all(struct rdatree *tree)
{
    struct rdatree_node *node, *next;
    int height;

    height = tree->height;

    if (height == 0) {
        tree->root = NULL;
        return;
    }

    node = rdatree_walk(tree, NULL);

    do {
        next = rdatree_walk(tree, node);
        node->nr_slots = 0;
        rdatree_cleanup(tree, node);
        node = next;
    } while (node != NULL);
}
