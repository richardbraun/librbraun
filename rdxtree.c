/*
 * Copyright (c) 2013 Richard Braun.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
#include "rdxtree.h"

/*
 * Global properties used to shape radix trees.
 */
#define RDXTREE_RADIX       6
#define RDXTREE_RADIX_SIZE  (1UL << RDXTREE_RADIX)
#define RDXTREE_RADIX_MASK  (RDXTREE_RADIX_SIZE - 1)

#if RDXTREE_RADIX < 6
typedef unsigned long rdxtree_bm_t;
#define rdxtree_ffs(x) __builtin_ffsl(x)
#elif RDXTREE_RADIX == 6 /* RDXTREE_RADIX < 6 */
#ifdef __LP64__
typedef unsigned long rdxtree_bm_t;
#define rdxtree_ffs(x) __builtin_ffsl(x)
#else /* __LP64__ */
typedef unsigned long long rdxtree_bm_t;
#define rdxtree_ffs(x) __builtin_ffsll(x)
#endif /* __LP64__ */
#else /* RDXTREE_RADIX < 6 */
#error "radix too high"
#endif /* RDXTREE_RADIX < 6 */

/*
 * Allocation bitmap size in bits.
 */
#define RDXTREE_BM_SIZE (sizeof(rdxtree_bm_t) * CHAR_BIT)

/*
 * Empty/full allocation bitmap words.
 */
#define RDXTREE_BM_EMPTY    ((rdxtree_bm_t)0)
#define RDXTREE_BM_FULL \
    ((~(rdxtree_bm_t)0) >> (RDXTREE_BM_SIZE - RDXTREE_RADIX_SIZE))

/*
 * Radix tree node.
 *
 * The index member can only be used if the parent isn't null. Concerning the
 * allocation bitmap, a bit is set when the node it denotes, or one of its
 * children, can be used to allocate a slot. Similarly, a bit is clear when
 * the matching node and all of its children have no free slot.
 */
struct rdxtree_node {
    struct rdxtree_node *parent;
    unsigned int index;
    unsigned int nr_slots;
    rdxtree_bm_t alloc_bm;
    void *slots[RDXTREE_RADIX_SIZE];
};

static struct rdxtree_node *
rdxtree_node_create(void)
{
    struct rdxtree_node *node;

    node = malloc(sizeof(*node));

    if (node == NULL)
        return NULL;

    node->parent = NULL;
    node->nr_slots = 0;
    node->alloc_bm = RDXTREE_BM_FULL;
    memset(node->slots, 0, sizeof(node->slots));
    return node;
}

static void
rdxtree_node_destroy(struct rdxtree_node *node)
{
    free(node);
}

static inline void
rdxtree_node_link(struct rdxtree_node *node, struct rdxtree_node *parent,
                  unsigned int index)
{
    node->parent = parent;
    node->index = index;
}

static inline void
rdxtree_node_unlink(struct rdxtree_node *node)
{
    assert(node->parent != NULL);
    node->parent = NULL;
}

static inline int
rdxtree_node_full(struct rdxtree_node *node)
{
    return (node->nr_slots == ARRAY_SIZE(node->slots));
}

static inline int
rdxtree_node_empty(struct rdxtree_node *node)
{
    return (node->nr_slots == 0);
}

static inline void
rdxtree_node_insert(struct rdxtree_node *node, unsigned int index, void *ptr)
{
    assert(node->slots[index] == NULL);

    node->nr_slots++;
    node->slots[index] = ptr;
}

static inline void
rdxtree_node_remove(struct rdxtree_node *node, unsigned int index)
{
    assert(node->slots[index] != NULL);

    node->nr_slots--;
    node->slots[index] = NULL;
}

static inline void **
rdxtree_node_find(struct rdxtree_node *node, unsigned int index)
{
    while (index < ARRAY_SIZE(node->slots)) {
        if (node->slots[index] != NULL)
            return &node->slots[index];

        index++;
    }

    return NULL;
}

static inline void
rdxtree_node_bm_set(struct rdxtree_node *node, unsigned int index)
{
    node->alloc_bm |= (rdxtree_bm_t)1 << index;
}

static inline void
rdxtree_node_bm_clear(struct rdxtree_node *node, unsigned int index)
{
    node->alloc_bm &= ~((rdxtree_bm_t)1 << index);
}

static inline int
rdxtree_node_bm_is_set(struct rdxtree_node *node, unsigned int index)
{
    return (node->alloc_bm & ((rdxtree_bm_t)1 << index));
}

static inline int
rdxtree_node_bm_empty(struct rdxtree_node *node)
{
    return (node->alloc_bm == RDXTREE_BM_EMPTY);
}

static inline int
rdxtree_node_bm_first(struct rdxtree_node *node)
{
    return rdxtree_ffs(node->alloc_bm) - 1;
}

static inline unsigned long
rdxtree_max_key(int height)
{
    unsigned int shift;

    shift = RDXTREE_RADIX * height;

    if (shift >= (sizeof(unsigned long) * CHAR_BIT))
        return ~0UL;
    else
        return (1 << shift) - 1;
}

static void
rdxtree_shrink(struct rdxtree *tree)
{
    struct rdxtree_node *node, *child;

    while (tree->height > 0) {
        node = tree->root;

        if (node->nr_slots != 1)
            break;

        child = node->slots[0];

        if (child == NULL)
            break;

        rdxtree_node_destroy(node);
        tree->height--;

        if (tree->height > 0)
            rdxtree_node_unlink(child);

        tree->root = child;
    }
}

static int
rdxtree_grow(struct rdxtree *tree, unsigned long key)
{
    struct rdxtree_node *node;
    int new_height;

    new_height = tree->height + 1;

    while (key > rdxtree_max_key(new_height))
        new_height++;

    if (tree->root == NULL) {
        tree->height = new_height;
        return ERR_SUCCESS;
    }

    do {
        node = rdxtree_node_create();

        if (node == NULL) {
            rdxtree_shrink(tree);
            return ERR_NOMEM;
        }

        rdxtree_node_insert(node, 0, tree->root);

        if (tree->height == 0)
            rdxtree_node_bm_clear(node, 0);
        else {
            rdxtree_node_link(tree->root, node, 0);

            if (rdxtree_node_bm_empty(tree->root))
                rdxtree_node_bm_clear(node, 0);
        }

        tree->root = node;
        tree->height++;
    } while (new_height > tree->height);

    return ERR_SUCCESS;
}

static void
rdxtree_cleanup(struct rdxtree *tree, struct rdxtree_node *node)
{
    struct rdxtree_node *prev;

    for (;;) {
        if (!rdxtree_node_empty(node)) {
            if (node->parent == NULL)
                rdxtree_shrink(tree);

            break;
        }

        if (node->parent == NULL) {
            rdxtree_node_destroy(node);
            tree->height = 0;
            tree->root = NULL;
            break;
        }

        prev = node;
        node = node->parent;
        rdxtree_node_remove(node, prev->index);
        rdxtree_node_destroy(prev);
    }
}

static void
rdxtree_insert_bm_clear(struct rdxtree_node *node, unsigned int index)
{
    for (;;) {
        rdxtree_node_bm_clear(node, index);

        if ((node->parent == NULL) || !rdxtree_node_full(node))
            break;

        index = node->index;
        node = node->parent;
    }
}

static int
rdxtree_insert_prim(struct rdxtree *tree, unsigned long key, void *ptr,
                    void ***slotp)
{
    struct rdxtree_node *node, *prev;
    unsigned int index = index;
    int error, height, shift;

    assert(ptr != NULL);

    if (key > rdxtree_max_key(tree->height)) {
        error = rdxtree_grow(tree, key);

        if (error)
            return error;
    }

    height = tree->height;

    if (height == 0) {
        if (tree->root != NULL)
            return ERR_BUSY;

        tree->root = ptr;

        if (slotp != NULL)
            *slotp = &tree->root;

        return ERR_SUCCESS;
    }

    node = tree->root;
    shift = (height - 1) * RDXTREE_RADIX;
    prev = NULL;

    do {
        if (node == NULL) {
            node = rdxtree_node_create();

            if (node == NULL) {
                if (prev == NULL)
                    tree->height = 0;
                else
                    rdxtree_cleanup(tree, prev);

                return ERR_NOMEM;
            }

            if (prev == NULL)
                tree->root = node;
            else {
                rdxtree_node_insert(prev, index, node);
                rdxtree_node_link(node, prev, index);
            }
        }

        prev = node;
        index = (key >> shift) & RDXTREE_RADIX_MASK;
        node = prev->slots[index];
        shift -= RDXTREE_RADIX;
        height--;
    } while (height > 0);

    if (node != NULL)
        return ERR_BUSY;

    rdxtree_node_insert(prev, index, ptr);
    rdxtree_insert_bm_clear(prev, index);

    if (slotp != NULL)
        *slotp = &prev->slots[index];

    return ERR_SUCCESS;
}

int
rdxtree_insert(struct rdxtree *tree, unsigned long key, void *ptr)
{
    return rdxtree_insert_prim(tree, key, ptr, NULL);
}

int
rdxtree_insert_slot(struct rdxtree *tree, unsigned long key, void *ptr,
                    void ***slotp)
{
    return rdxtree_insert_prim(tree, key, ptr, slotp);
}

static int
rdxtree_insert_alloc_prim(struct rdxtree *tree, void *ptr, unsigned long *keyp,
                          void ***slotp)
{
    struct rdxtree_node *node, *prev;
    unsigned long key;
    int error, height, shift, index = index;

    assert(ptr != NULL);

    height = tree->height;

    if (height == 0) {
        if (tree->root == NULL) {
            tree->root = ptr;
            *keyp = 0;

            if (slotp != NULL)
                *slotp = &tree->root;

            return ERR_SUCCESS;
        }

        goto grow;
    }

    node = tree->root;
    key = 0;
    shift = (height - 1) * RDXTREE_RADIX;
    prev = NULL;

    do {
        if (node == NULL) {
            node = rdxtree_node_create();

            if (node == NULL) {
                rdxtree_cleanup(tree, prev);
                return ERR_NOMEM;
            }

            rdxtree_node_insert(prev, index, node);
            rdxtree_node_link(node, prev, index);
        }

        prev = node;
        index = rdxtree_node_bm_first(node);

        if (index == -1)
            goto grow;

        key |= ((unsigned long)index << shift);
        node = node->slots[index];
        shift -= RDXTREE_RADIX;
        height--;
    } while (height > 0);

    rdxtree_node_insert(prev, index, ptr);
    rdxtree_insert_bm_clear(prev, index);

    if (slotp != NULL)
        *slotp = &prev->slots[index];

    goto out;

grow:
    key = rdxtree_max_key(height) + 1;
    error = rdxtree_insert_prim(tree, key, ptr, slotp);

    if (error)
        return error;

out:
    *keyp = key;
    return ERR_SUCCESS;
}

int
rdxtree_insert_alloc(struct rdxtree *tree, void *ptr, unsigned long *keyp)
{
    return rdxtree_insert_alloc_prim(tree, ptr, keyp, NULL);
}

int
rdxtree_insert_alloc_slot(struct rdxtree *tree, void *ptr, unsigned long *keyp,
                          void ***slotp)
{
    return rdxtree_insert_alloc_prim(tree, ptr, keyp, slotp);
}

static void
rdxtree_remove_bm_set(struct rdxtree_node *node, unsigned int index)
{
    do {
        rdxtree_node_bm_set(node, index);

        if (node->parent == NULL)
            break;

        index = node->index;
        node = node->parent;
    } while (!rdxtree_node_bm_is_set(node, index));
}

void *
rdxtree_remove(struct rdxtree *tree, unsigned long key)
{
    struct rdxtree_node *node, *prev;
    unsigned int index;
    int height, shift;

    height = tree->height;

    if (key > rdxtree_max_key(height))
        return NULL;

    node = tree->root;

    if (height == 0) {
        tree->root = NULL;
        return node;
    }

    shift = (height - 1) * RDXTREE_RADIX;

    do {
        if (node == NULL)
            return NULL;

        prev = node;
        index = (key >> shift) & RDXTREE_RADIX_MASK;
        node = node->slots[index];
        shift -= RDXTREE_RADIX;
        height--;
    } while (height > 0);

    if (node == NULL)
        return NULL;

    rdxtree_node_remove(prev, index);
    rdxtree_remove_bm_set(prev, index);
    rdxtree_cleanup(tree, prev);
    return node;
}

static void **
rdxtree_lookup_prim(struct rdxtree *tree, unsigned long key)
{
    struct rdxtree_node *node, *prev;
    unsigned int index;
    int height, shift;

    height = tree->height;

    if (key > rdxtree_max_key(height))
        return NULL;

    if (height == 0) {
        if (tree->root == NULL)
            return NULL;

        return &tree->root;
    }

    node = tree->root;
    shift = (height - 1) * RDXTREE_RADIX;

    do {
        if (node == NULL)
            return NULL;

        prev = node;
        index = (key >> shift) & RDXTREE_RADIX_MASK;
        node = node->slots[index];
        shift -= RDXTREE_RADIX;
        height--;
    } while (height > 0);

    if (node == NULL)
        return NULL;

    return &prev->slots[index];
}

void *
rdxtree_lookup(struct rdxtree *tree, unsigned long key)
{
    void **slot;

    slot = rdxtree_lookup_prim(tree, key);
    return (slot == NULL) ? NULL : *slot;
}

void **
rdxtree_lookup_slot(struct rdxtree *tree, unsigned long key)
{
    return rdxtree_lookup_prim(tree, key);
}

void *
rdxtree_replace_slot(void **slot, void *ptr)
{
    void *old;

    assert(ptr != NULL);

    old = *slot;
    assert(old != NULL);
    *slot = ptr;
    return old;
}

static struct rdxtree_node *
rdxtree_walk(struct rdxtree *tree, struct rdxtree_node *node)
{
    struct rdxtree_node *prev;
    void **slot;
    unsigned int index;
    int height;

    if (node == NULL) {
        height = tree->height;
        node = tree->root;

        while (height > 1) {
            slot = rdxtree_node_find(node, 0);
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
        slot = rdxtree_node_find(prev, index + 1);

        if (slot != NULL)
            break;

        height++;
        node = prev;
    }

    node = *slot;

    while (height > 0) {
        slot = rdxtree_node_find(node, 0);
        node = *slot;
        height--;
    }

    return node;
}

void *
rdxtree_iter_next(struct rdxtree *tree, struct rdxtree_iter *iter)
{
    unsigned int index;

    if (tree->height == 0) {
        if (iter->slot != NULL)
            return NULL;

        iter->slot = &tree->root;
        return *iter->slot;
    }

    if (iter->node != NULL) {
        index = iter->slot - ((struct rdxtree_node *)iter->node)->slots;
        iter->slot = rdxtree_node_find(iter->node, index + 1);
    }

    if (iter->slot == NULL) {
        iter->node = rdxtree_walk(tree, iter->node);

        if (iter->node != NULL)
            iter->slot = rdxtree_node_find(iter->node, 0);
    }

    if (iter->slot == NULL)
        return NULL;

    return *iter->slot;
}

void
rdxtree_remove_all(struct rdxtree *tree)
{
    struct rdxtree_node *node, *next;
    int height;

    height = tree->height;

    if (height == 0) {
        tree->root = NULL;
        return;
    }

    node = rdxtree_walk(tree, NULL);

    do {
        next = rdxtree_walk(tree, node);
        node->nr_slots = 0;
        rdxtree_cleanup(tree, node);
        node = next;
    } while (node != NULL);
}
