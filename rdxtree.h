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
rdxtree_init(struct rdxtree *tree)
{
    tree->height = 0;
    tree->root = NULL;
}

/*
 * Insert a pointer in a tree.
 *
 * The ptr parameter must not be NULL.
 */
static inline int
rdxtree_insert(struct rdxtree *tree, unsigned long key, void *ptr)
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
rdxtree_insert_slot(struct rdxtree *tree, unsigned long key, void *ptr,
                    void ***slotp)
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
rdxtree_insert_alloc(struct rdxtree *tree, void *ptr, unsigned long *keyp)
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
                          unsigned long *keyp, void ***slotp)
{
    return rdxtree_insert_alloc_common(tree, ptr, keyp, slotp);
}

/*
 * Remove a pointer from a tree.
 *
 * The matching pointer is returned if successful, NULL otherwise.
 */
void * rdxtree_remove(struct rdxtree *tree, unsigned long key);

/*
 * Look up a pointer in a tree.
 *
 * The matching pointer is returned if successful, NULL otherwise.
 */
static inline void *
rdxtree_lookup(struct rdxtree *tree, unsigned long key)
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
rdxtree_lookup_slot(struct rdxtree *tree, unsigned long key)
{
    return rdxtree_lookup_common(tree, key, 1);
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
