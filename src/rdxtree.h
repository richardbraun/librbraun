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
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 *
 *
 * Radix tree.
 *
 * In addition to the standard insertion operation, this implementation
 * can allocate keys for the caller at insertion time.
 */

#ifndef RDXTREE_H
#define RDXTREE_H

#include <stddef.h>
#include <stdint.h>

/*
 * These macros can be replaced by actual functions in an environment
 * that provides lockless synchronization such as RCU.
 */
#define rcu_store_ptr(ptr, value)       ((ptr) = (value))
#define rcu_load_ptr(ptr)               (ptr)

/*
 * This macro selects between 32 or 64-bits (the default) keys.
 */
#if 0
#define RDXTREE_KEY_32
#endif

#ifdef RDXTREE_KEY_32
typedef uint32_t rdxtree_key_t;
#else /* RDXTREE_KEY_32 */
typedef uint64_t rdxtree_key_t;
#endif /* RDXTREE_KEY_32 */

/*
 * Radix tree initialization flags.
 */
#define RDXTREE_KEY_ALLOC 0x1 /* Enable key allocation */

/*
 * Radix tree.
 */
struct rdxtree;

/*
 * Radix tree iterator.
 */
struct rdxtree_iter;

/*
 * Static tree initializer.
 */
#define RDXTREE_INITIALIZER { 0, NULL }

#include "rdxtree_i.h"

/*
 * Initialize a tree.
 */
static inline void
rdxtree_init(struct rdxtree *tree, unsigned short flags)
{
    assert((flags & ~RDXTREE_KEY_ALLOC) == 0);

    tree->height = 0;
    tree->flags = flags;
    tree->root = NULL;
}

/*
 * Insert a pointer in a tree.
 *
 * The ptr parameter must not be NULL.
 */
static inline int
rdxtree_insert(struct rdxtree *tree, rdxtree_key_t key, void *ptr)
{
    return rdxtree_insert_common(tree, key, ptr, NULL);
}

/*
 * Insert a pointer in a tree and obtain its slot.
 *
 * The ptr and slotp parameters must not be NULL. If successful, the slot of
 * the newly inserted pointer is stored at the address pointed to by the slotp
 * parameter.
 */
static inline int
rdxtree_insert_slot(struct rdxtree *tree, rdxtree_key_t key,
                    void *ptr, void ***slotp)
{
    return rdxtree_insert_common(tree, key, ptr, slotp);
}

/*
 * Insert a pointer in a tree, for which a new key is allocated.
 *
 * The ptr and keyp parameters must not be NULL. The newly allocated key is
 * stored at the address pointed to by the keyp parameter.
 */
static inline int
rdxtree_insert_alloc(struct rdxtree *tree, void *ptr, rdxtree_key_t *keyp)
{
    return rdxtree_insert_alloc_common(tree, ptr, keyp, NULL);
}

/*
 * Insert a pointer in a tree, for which a new key is allocated, and obtain
 * its slot.
 *
 * The ptr, keyp and slotp parameters must not be NULL. The newly allocated
 * key is stored at the address pointed to by the keyp parameter while the
 * slot of the inserted pointer is stored at the address pointed to by the
 * slotp parameter.
 */
static inline int
rdxtree_insert_alloc_slot(struct rdxtree *tree, void *ptr,
                          rdxtree_key_t *keyp, void ***slotp)
{
    return rdxtree_insert_alloc_common(tree, ptr, keyp, slotp);
}

/*
 * Remove a pointer from a tree.
 *
 * The matching pointer is returned if successful, NULL otherwise.
 */
void * rdxtree_remove(struct rdxtree *tree, rdxtree_key_t key);

/*
 * Look up a pointer in a tree.
 *
 * The matching pointer is returned if successful, NULL otherwise.
 */
static inline void *
rdxtree_lookup(const struct rdxtree *tree, rdxtree_key_t key)
{
    return rdxtree_lookup_common(tree, key, 0);
}

/*
 * Look up a slot in a tree.
 *
 * A slot is a pointer to a stored pointer in a tree. It can be used as
 * a placeholder for fast replacements to avoid multiple lookups on the same
 * key.
 *
 * A slot for the matching pointer is returned if successful, NULL otherwise.
 *
 * See rdxtree_replace_slot().
 */
static inline void **
rdxtree_lookup_slot(const struct rdxtree *tree, rdxtree_key_t key)
{
    return rdxtree_lookup_common(tree, key, 1);
}

static inline void *
rdxtree_load_slot(void **slot)
{
    return rcu_load_ptr(*slot);
}

/*
 * Replace a pointer in a tree.
 *
 * The ptr parameter must not be NULL. The previous pointer is returned.
 *
 * See rdxtree_lookup_slot().
 */
void * rdxtree_replace_slot(void **slot, void *ptr);

/*
 * Forge a loop to process all pointers of a tree.
 *
 * It is not safe to modify a tree from such a loop.
 */
#define rdxtree_for_each(tree, iter, ptr)                       \
for (rdxtree_iter_init(iter), ptr = rdxtree_walk(tree, iter);   \
     ptr != NULL;                                               \
     ptr = rdxtree_walk(tree, iter))

/*
 * Return the key of the current pointer from an iterator.
 */
static inline rdxtree_key_t
rdxtree_iter_key(const struct rdxtree_iter *iter)
{
    return iter->key;
}

/*
 * Remove all pointers from a tree.
 *
 * The common way to destroy a tree and its pointers is to loop over all
 * the pointers using rdxtree_for_each(), freeing them, then call this
 * function.
 */
void rdxtree_remove_all(struct rdxtree *tree);

#endif /* RDXTREE_H */
