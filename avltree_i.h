/*
 * Copyright (c) 2010-2015 Richard Braun.
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
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#ifndef _AVLTREE_I_H
#define _AVLTREE_I_H

#include <assert.h>
#include <stddef.h>

#include "macros.h"

/*
 * AVL node structure.
 *
 * To reduce the number of branches and the instruction cache footprint,
 * the left and right child pointers are stored in an array, and the symmetry
 * of most tree operations is exploited by using left/right variables when
 * referring to children.
 *
 * In addition, this implementation assumes that all nodes are at least 4-byte
 * aligned, so that the two least significant bits of the parent member can
 * be used to store the balance of the node. This is true for all modern 32
 * and 64 bits architectures, as long as the nodes aren't embedded in
 * structures with special alignment constraints such as member packing.
 *
 * To avoid issues with signed operations when handling a negative balance,
 * the raw values stored in the parent member are always positive, and are
 * actually one more than the value they represent. It is then easy to obtain
 * a balance of -1 from a raw value of 0.
 */
struct avltree_node {
    unsigned long parent;
    struct avltree_node *children[2];
};

/*
 * AVL tree structure.
 */
struct avltree {
    struct avltree_node *root;
};

/*
 * Masks applied on the parent member of a node to obtain either the
 * balance or the parent address.
 */
#define AVLTREE_BALANCE_MASK    0x3UL
#define AVLTREE_PARENT_MASK     (~AVLTREE_BALANCE_MASK)

/*
 * Special raw balance values.
 */
#define AVLTREE_BALANCE_ZERO    1UL
#define AVLTREE_BALANCE_INVALID AVLTREE_BALANCE_MASK

/*
 * Masks applied on slots to obtain either the child index or the parent
 * address.
 */
#define AVLTREE_SLOT_INDEX_MASK     0x1UL
#define AVLTREE_SLOT_PARENT_MASK    (~AVLTREE_SLOT_INDEX_MASK)

/*
 * Return true if the given index is a valid child index.
 */
static inline int
avltree_check_index(int index)
{
    return index == (index & 1);
}

/*
 * Convert the result of a comparison (or a balance) into an index in the
 * children array (0 or 1).
 *
 * This function is mostly used when looking up a node.
 */
static inline int
avltree_d2i(int diff)
{
    return !(diff <= 0);
}

/*
 * Return true if the given pointer is suitably aligned.
 */
static inline int
avltree_node_check_alignment(const struct avltree_node *node)
{
    return ((unsigned long)node & AVLTREE_BALANCE_MASK) == 0;
}

/*
 * Return the parent of a node.
 */
static inline struct avltree_node *
avltree_node_parent(const struct avltree_node *node)
{
    return (struct avltree_node *)(node->parent & AVLTREE_PARENT_MASK);
}

/*
 * Translate an insertion point into a slot.
 */
static inline unsigned long
avltree_slot(struct avltree_node *parent, int index)
{
    assert(avltree_node_check_alignment(parent));
    assert(avltree_check_index(index));
    return (unsigned long)parent | index;
}

/*
 * Extract the parent address from a slot.
 */
static inline struct avltree_node *
avltree_slot_parent(unsigned long slot)
{
    return (struct avltree_node *)(slot & AVLTREE_SLOT_PARENT_MASK);
}

/*
 * Extract the index from a slot.
 */
static inline int
avltree_slot_index(unsigned long slot)
{
    return slot & AVLTREE_SLOT_INDEX_MASK;
}

/*
 * Insert a node in a tree, rebalancing it if necessary.
 *
 * The index parameter is the index in the children array of the parent where
 * the new node is to be inserted. It is ignored if the parent is NULL.
 *
 * This function is intended to be used by the avltree_insert() macro only.
 */
void avltree_insert_rebalance(struct avltree *tree, struct avltree_node *parent,
                              int index, struct avltree_node *node);

/*
 * Return the previous or next node relative to a location in a tree.
 *
 * The parent and index parameters define the location, which can be empty.
 * The direction parameter is either AVLTREE_LEFT (to obtain the previous
 * node) or AVLTREE_RIGHT (to obtain the next one).
 */
struct avltree_node * avltree_nearest(struct avltree_node *parent, int index,
                                      int direction);

/*
 * Return the first or last node of a tree.
 *
 * The direction parameter is either AVLTREE_LEFT (to obtain the first node)
 * or AVLTREE_RIGHT (to obtain the last one).
 */
struct avltree_node * avltree_firstlast(const struct avltree *tree,
                                        int direction);

/*
 * Return the node next to, or previous to the given node.
 *
 * The direction parameter is either AVLTREE_LEFT (to obtain the previous node)
 * or AVLTREE_RIGHT (to obtain the next one).
 */
struct avltree_node * avltree_walk(struct avltree_node *node, int direction);

/*
 * Return the left-most deepest node of a tree, which is the starting point of
 * the postorder traversal performed by avltree_for_each_remove().
 */
struct avltree_node * avltree_postwalk_deepest(const struct avltree *tree);

/*
 * Unlink a node from its tree and return the next (right) node in postorder.
 */
struct avltree_node * avltree_postwalk_unlink(struct avltree_node *node);

#endif /* _AVLTREE_I_H */
