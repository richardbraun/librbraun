/*
 * Copyright (c) 2010-2017 Richard Braun.
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
 * AVL tree.
 *
 * This type of tree is well suited for lookup intensive applications. In
 * the more common case where there can be as many insertions/removals as
 * lookups, or the amount of changes can't be determined, red-black trees
 * provide better average performance.
 */

#ifndef AVLTREE_H
#define AVLTREE_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "macros.h"

/*
 * Indexes of the left and right nodes in the children array of a node.
 */
#define AVLTREE_LEFT    0
#define AVLTREE_RIGHT   1

/*
 * AVL node.
 */
struct avltree_node;

/*
 * AVL tree.
 */
struct avltree;

/*
 * Insertion point identifier.
 */
typedef uintptr_t avltree_slot_t;

/*
 * Static tree initializer.
 */
#define AVLTREE_INITIALIZER { NULL }

#include "avltree_i.h"

/*
 * Initialize a tree.
 */
static inline void
avltree_init(struct avltree *tree)
{
    tree->root = NULL;
}

/*
 * Initialize a node.
 *
 * A node is in no tree when its parent points to itself.
 */
static inline void
avltree_node_init(struct avltree_node *node)
{
    assert(avltree_node_check_alignment(node));

    node->parent = (uintptr_t)node | AVLTREE_BALANCE_ZERO;
    node->children[AVLTREE_LEFT] = NULL;
    node->children[AVLTREE_RIGHT] = NULL;
}

/*
 * Return true if node is in no tree.
 */
static inline int
avltree_node_unlinked(const struct avltree_node *node)
{
    return avltree_node_parent(node) == node;
}

/*
 * Macro that evaluates to the address of the structure containing the
 * given node based on the given type and member.
 */
#define avltree_entry(node, type, member) structof(node, type, member)

/*
 * Return true if tree is empty.
 */
static inline int
avltree_empty(const struct avltree *tree)
{
    return tree->root == NULL;
}

/*
 * Look up a node in a tree.
 *
 * Note that implementing the lookup algorithm as a macro gives two benefits:
 * First, it avoids the overhead of a callback function. Next, the type of the
 * cmp_fn parameter isn't rigid. The only guarantee offered by this
 * implementation is that the key parameter is the first parameter given to
 * cmp_fn. This way, users can pass only the value they need for comparison
 * instead of e.g. allocating a full structure on the stack.
 */
#define avltree_lookup(tree, key, cmp_fn)                   \
MACRO_BEGIN                                                 \
    struct avltree_node *cur_;                              \
    int diff_;                                              \
                                                            \
    cur_ = (tree)->root;                                    \
                                                            \
    while (cur_ != NULL) {                                  \
        diff_ = cmp_fn(key, cur_);                          \
                                                            \
        if (diff_ == 0) {                                   \
            break;                                          \
        }                                                   \
                                                            \
        cur_ = cur_->children[avltree_d2i(diff_)];          \
    }                                                       \
                                                            \
    cur_;                                                   \
MACRO_END

/*
 * Look up a node or one of its nearest nodes in a tree.
 *
 * This macro essentially acts as avltree_lookup() but if no entry matched
 * the key, an additional step is performed to obtain the next or previous
 * node, depending on the direction (left or right).
 *
 * The constraints that apply to the key parameter are the same as for
 * avltree_lookup().
 */
#define avltree_lookup_nearest(tree, key, cmp_fn, dir)      \
MACRO_BEGIN                                                 \
    struct avltree_node *cur_, *prev_;                      \
    int diff_, index_;                                      \
                                                            \
    prev_ = NULL;                                           \
    index_ = 0;                                             \
    cur_ = (tree)->root;                                    \
                                                            \
    while (cur_ != NULL) {                                  \
        diff_ = cmp_fn(key, cur_);                          \
                                                            \
        if (diff_ == 0) {                                   \
            break;                                          \
        }                                                   \
                                                            \
        prev_ = cur_;                                       \
        index_ = avltree_d2i(diff_);                        \
        cur_ = cur_->children[index_];                      \
    }                                                       \
                                                            \
    if (cur_ == NULL) {                                     \
        cur_ = avltree_nearest(prev_, index_, dir);         \
    }                                                       \
                                                            \
    cur_;                                                   \
MACRO_END

/*
 * Insert a node in a tree.
 *
 * This macro performs a standard lookup to obtain the insertion point of
 * the given node in the tree (it is assumed that the inserted node never
 * compares equal to any other entry in the tree) and links the node. It
 * then updates and checks balances of the parent nodes, and rebalances the
 * tree if necessary.
 *
 * Unlike avltree_lookup(), the cmp_fn parameter must compare two complete
 * entries, so it is suggested to use two different comparison inline
 * functions, such as myobj_cmp_lookup() and myobj_cmp_insert(). There is no
 * guarantee about the order of the nodes given to the comparison function.
 */
#define avltree_insert(tree, node, cmp_fn)                      \
MACRO_BEGIN                                                     \
    struct avltree_node *cur_, *prev_;                          \
    int diff_, index_;                                          \
                                                                \
    prev_ = NULL;                                               \
    index_ = 0;                                                 \
    cur_ = (tree)->root;                                        \
                                                                \
    while (cur_ != NULL) {                                      \
        diff_ = cmp_fn(node, cur_);                             \
        assert(diff_ != 0);                                     \
        prev_ = cur_;                                           \
        index_ = avltree_d2i(diff_);                            \
        cur_ = cur_->children[index_];                          \
    }                                                           \
                                                                \
    avltree_insert_rebalance(tree, prev_, index_, node);        \
MACRO_END

/*
 * Look up a node/slot pair in a tree.
 *
 * This macro essentially acts as avltree_lookup() but in addition to a node,
 * it also returns a slot, which identifies an insertion point in the tree.
 * If the returned node is NULL, the slot can be used by avltree_insert_slot()
 * to insert without the overhead of an additional lookup.
 *
 * The constraints that apply to the key parameter are the same as for
 * avltree_lookup().
 */
#define avltree_lookup_slot(tree, key, cmp_fn, slot)    \
MACRO_BEGIN                                             \
    struct avltree_node *cur_, *prev_;                  \
    int diff_, index_;                                  \
                                                        \
    prev_ = NULL;                                       \
    index_ = 0;                                         \
    cur_ = (tree)->root;                                \
                                                        \
    while (cur_ != NULL) {                              \
        diff_ = cmp_fn(key, cur_);                      \
                                                        \
        if (diff_ == 0) {                               \
            break;                                      \
        }                                               \
                                                        \
        prev_ = cur_;                                   \
        index_ = avltree_d2i(diff_);                    \
        cur_ = cur_->children[index_];                  \
    }                                                   \
                                                        \
    (slot) = avltree_slot(prev_, index_);               \
    cur_;                                               \
MACRO_END

/*
 * Insert a node at an insertion point in a tree.
 *
 * This macro essentially acts as avltree_insert() except that it doesn't
 * obtain the insertion point with a standard lookup. The insertion point
 * is obtained by calling avltree_lookup_slot(). In addition, the new node
 * must not compare equal to an existing node in the tree (i.e. the slot
 * must denote a NULL node).
 */
static inline void
avltree_insert_slot(struct avltree *tree, avltree_slot_t slot,
                    struct avltree_node *node)
{
    struct avltree_node *parent;
    int index;

    parent = avltree_slot_parent(slot);
    index = avltree_slot_index(slot);
    avltree_insert_rebalance(tree, parent, index, node);
}

/*
 * Replace a node at an insertion point in a tree.
 *
 * The given node must compare strictly equal to the previous node,
 * which is returned on completion.
 */
void * avltree_replace_slot(struct avltree *tree, avltree_slot_t slot,
                            struct avltree_node *node);

/*
 * Remove a node from a tree.
 *
 * After completion, the node is stale.
 */
void avltree_remove(struct avltree *tree, struct avltree_node *node);

/*
 * Return the first node of a tree.
 */
#define avltree_first(tree) avltree_firstlast(tree, AVLTREE_LEFT)

/*
 * Return the last node of a tree.
 */
#define avltree_last(tree) avltree_firstlast(tree, AVLTREE_RIGHT)

/*
 * Return the node previous to the given node.
 */
#define avltree_prev(node) avltree_walk(node, AVLTREE_LEFT)

/*
 * Return the node next to the given node.
 */
#define avltree_next(node) avltree_walk(node, AVLTREE_RIGHT)

/*
 * Forge a loop to process all nodes of a tree, removing them when visited.
 *
 * This macro can only be used to destroy a tree, so that the resources used
 * by the entries can be released by the user. It basically removes all nodes
 * without doing any balance checking.
 *
 * After completion, all nodes and the tree root member are stale.
 */
#define avltree_for_each_remove(tree, node, tmp)        \
for (node = avltree_postwalk_deepest(tree),             \
     tmp = avltree_postwalk_unlink(node);               \
     node != NULL;                                      \
     node = tmp, tmp = avltree_postwalk_unlink(node))

#endif /* AVLTREE_H */
