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
 *
 *
 * Radix tree.
 *
 * In addition to the standard insertion operation, this implementation
 * can allocate keys for the caller at insertion time.
 */

#ifndef _RDXTREE_H
#define _RDXTREE_H

#include <stddef.h>

/*
 * Radix tree.
 */
struct rdxtree {
    int height;
    void *root;
};

/*
 * Radix tree iterator.
 *
 * Don't use directly - use rdxtree_for_each() instead.
 */
struct rdxtree_iter {
    void *node;
    void **slot;
};

/*
 * Static tree initializer.
 */
#define RDXTREE_INITIALIZER { 0, NULL }

/*
 * Initialize a tree.
 */
static inline void rdxtree_init(struct rdxtree *tree)
{
    tree->height = 0;
    tree->root = NULL;
}

/*
 * Initialize an iterator.
 */
static inline void rdxtree_iter_init(struct rdxtree_iter *iter)
{
    iter->node = NULL;
    iter->slot = NULL;
}

/*
 * Insert a pointer in a tree.
 *
 * The ptr parameter must not be null.
 */
int rdxtree_insert(struct rdxtree *tree, unsigned long key, void *ptr);

/*
 * Insert a pointer in a tree, for which a new key is allocated.
 *
 * The ptr and keyp parameters must not be null. The newly allocated key is
 * stored at the address pointed to by the keyp parameter.
 */
int rdxtree_insert_alloc(struct rdxtree *tree, void *ptr, unsigned long *keyp);

/*
 * Remove a pointer from a tree.
 *
 * The matching pointer is returned if successful, null otherwise.
 */
void * rdxtree_remove(struct rdxtree *tree, unsigned long key);

/*
 * Look up a pointer in a tree.
 *
 * The matching pointer is returned if successful, null otherwise.
 */
void * rdxtree_lookup(struct rdxtree *tree, unsigned long key);

/*
 * Look up a slot in a tree.
 *
 * A slot is a pointer to a contained pointer in a tree. It can be used as
 * a placeholder for fast replacements to avoid multiple lookups on the same
 * key.
 *
 * A slot for the matching pointer is returned if successful, null otherwise.
 *
 * See rdxtree_replace_slot().
 */
void ** rdxtree_lookup_slot(struct rdxtree *tree, unsigned long key);

/*
 * Replace a pointer in a tree.
 *
 * The ptr parameter must not be null. The previous pointer is returned.
 *
 * See rdxtree_lookup_slot().
 */
void * rdxtree_replace_slot(void **slot, void *ptr);

/*
 * Walk pointers in a tree.
 *
 * Move the iterator to the next pointer in the given tree.
 *
 * The next pointer is returned if there is one, null otherwise.
 */
void * rdxtree_iter_next(struct rdxtree *tree, struct rdxtree_iter *iter);

/*
 * Forge a loop to process all pointers of a tree.
 */
#define rdxtree_for_each(tree, iter, ptr)                           \
for (rdxtree_iter_init(iter), ptr = rdxtree_iter_next(tree, iter);  \
     ptr != NULL;                                                   \
     ptr = rdxtree_iter_next(tree, iter))

/*
 * Remove all pointers from a tree.
 *
 * The common way to destroy a tree and its pointers is to loop over all
 * the pointers using rdxtree_for_each(), freeing them, then call this
 * function.
 */
void rdxtree_remove_all(struct rdxtree *tree);

#endif /* _RDXTREE_H */