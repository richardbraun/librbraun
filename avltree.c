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

#include <assert.h>
#include <stddef.h>

#include "macros.h"
#include "avltree.h"
#include "avltree_i.h"

/*
 * Convert an index of the children array (0 or 1) into a balance value
 * (-1 or 1).
 */
static inline int
avltree_i2b(int index)
{
    assert(avltree_check_index(index));
    return (index - 1) | 1;
}

/*
 * Return the index of a node in the children array of its parent.
 *
 * The parent parameter must not be NULL, and must be the parent of the
 * given node.
 */
static inline int
avltree_node_index(const struct avltree_node *node,
                   const struct avltree_node *parent)
{
    assert(parent != NULL);
    assert((node == NULL) || (avltree_node_parent(node) == parent));

    if (parent->children[AVLTREE_LEFT] == node)
        return AVLTREE_LEFT;

    assert(parent->children[AVLTREE_RIGHT] == node);

    return AVLTREE_RIGHT;
}

/*
 * Return the balance of a node.
 */
static inline int
avltree_node_balance(const struct avltree_node *node)
{
    int balance;

    balance = (node->parent & AVLTREE_BALANCE_MASK);
    assert(balance != AVLTREE_BALANCE_INVALID);

    return balance - 1;
}

/*
 * Set the parent of a node, retaining its current balance.
 */
static inline void
avltree_node_set_parent(struct avltree_node *node, struct avltree_node *parent)
{
    assert(avltree_node_check_alignment(node));
    assert(avltree_node_check_alignment(parent));
    node->parent = (unsigned long)parent
                   | (node->parent & AVLTREE_BALANCE_MASK);
}

/*
 * Set the balance of a node, retaining its current parent.
 */
static inline void
avltree_node_set_balance(struct avltree_node *node, int balance)
{
    assert((-1 <= balance) && (balance <= 1));
    node->parent = (node->parent & AVLTREE_PARENT_MASK) | (balance + 1);
}

/*
 * Return the left-most deepest child node of the given node.
 */
static struct avltree_node *
avltree_node_find_deepest(struct avltree_node *node)
{
    struct avltree_node *parent;

    assert(node != NULL);

    for (;;) {
        parent = node;
        node = node->children[AVLTREE_LEFT];

        if (node == NULL) {
            node = parent->children[AVLTREE_RIGHT];

            if (node == NULL)
                return parent;
        }
    }
}

/*
 * Rotate an unbalanced tree.
 *
 * This function is called after the insertion or the removal of a node made
 * a tree unbalanced. The node and balance parameters define the rotation root
 * (the balance being either -2 or 2).
 *
 * Return true if the overall height of the subtree rooted at node has
 * decreased, false otherwise.
 */
static int
avltree_rotate(struct avltree *tree, struct avltree_node *node, int balance)
{
    struct avltree_node *parent, *lnode, *lrnode, *lrlnode, *lrrnode;
    int left, right, lweight, rweight, lbalance;
    int index = 0; /* GCC */

    assert((balance == -2) || (balance == 2));

    left = avltree_d2i(balance);
    right = 1 - left;
    lweight = balance >> 1;
    rweight = -lweight;

    parent = avltree_node_parent(node);

    if (likely(parent != NULL))
        index = avltree_node_index(node, parent);

    lnode = node->children[left];
    assert(lnode != NULL);
    lbalance = avltree_node_balance(lnode);
    lrnode = lnode->children[right];

    /*
     * Left-left case. Note that only a removal can set the left node balance
     * to 0 when rotating.
     */
    if (lbalance != rweight) {
        node->children[left] = lrnode;

        if (lrnode != NULL)
            avltree_node_set_parent(lrnode, node);

        lbalance += rweight;

        lnode->children[right] = node;

        avltree_node_set_parent(node, lnode);
        avltree_node_set_balance(node, -lbalance);

        avltree_node_set_parent(lnode, parent);
        avltree_node_set_balance(lnode, lbalance);

        if (unlikely(parent == NULL))
            tree->root = lnode;
        else
            parent->children[index] = lnode;

        /*
         * If the adjusted balance is now 0, it means that the height of the
         * left subtree has decreased.
         */
        return (lbalance == 0);
    }

    /*
     * Left-right case.
     */

    assert(lrnode != NULL);
    lrlnode = lrnode->children[left];
    lrrnode = lrnode->children[right];

    node->children[left] = lrrnode;

    if (lrrnode != NULL)
        avltree_node_set_parent(lrrnode, node);

    lnode->children[right] = lrlnode;

    if (lrlnode != NULL)
        avltree_node_set_parent(lrlnode, lnode);

    balance = avltree_node_balance(lrnode);

    lrnode->children[left] = lnode;
    avltree_node_set_parent(lnode, lrnode);
    avltree_node_set_balance(lnode, ((balance == rweight) ? lweight : 0));

    lrnode->children[right] = node;
    avltree_node_set_parent(node, lrnode);
    avltree_node_set_balance(node, ((balance == lweight) ? rweight : 0));

    avltree_node_set_parent(lrnode, parent);
    avltree_node_set_balance(lrnode, 0);

    if (unlikely(parent == NULL))
        tree->root = lrnode;
    else
        parent->children[index] = lrnode;

    /*
     * The balance of the new subtree root is always 0 in this case, which
     * implies the overall subtree height has decreased.
     */
    return 1;
}

void
avltree_insert_rebalance(struct avltree *tree, struct avltree_node *parent,
                         int index, struct avltree_node *node)
{
    int old_balance, new_balance;

    assert(avltree_node_check_alignment(parent));
    assert(avltree_node_check_alignment(node));

    node->parent = (unsigned long)parent | AVLTREE_BALANCE_ZERO;
    node->children[AVLTREE_LEFT] = NULL;
    node->children[AVLTREE_RIGHT] = NULL;

    /*
     * The node is the tree root, and is the single tree entry.
     */
    if (parent == NULL) {
        assert(tree->root == NULL);
        tree->root = node;
        return;
    }

    assert(avltree_check_index(index));
    assert(parent->children[index] == NULL);
    parent->children[index] = node;

    for (;;) {
        node = parent;

        old_balance = avltree_node_balance(node);
        new_balance = old_balance + avltree_i2b(index);

        /*
         * Perfect balance, stop now.
         */
        if (new_balance == 0) {
            avltree_node_set_balance(node, 0);
            return;
        }

        /*
         * Both previous and new balances are different from 0, which means the
         * new one has reached -2 or 2. Rebalance now.
         */
        if (old_balance != 0)
            break;

        /*
         * The new balance is either -1 or 1. Update the current node and
         * iterate again to propagate the height change.
         */
        avltree_node_set_balance(node, new_balance);
        parent = avltree_node_parent(node);

        /*
         * The tree root was reached.
         */
        if (parent == NULL)
            return;

        index = avltree_node_index(node, parent);
    }

    avltree_rotate(tree, node, new_balance);
}

void
avltree_remove(struct avltree *tree, struct avltree_node *node)
{
    struct avltree_node *child, *parent;
    int left, right, index, old_balance, new_balance;

    if (node->children[AVLTREE_LEFT] == NULL)
        child = node->children[AVLTREE_RIGHT];
    else if (node->children[AVLTREE_RIGHT] == NULL)
        child = node->children[AVLTREE_LEFT];
    else {
        struct avltree_node *successor;

        /*
         * Two-children case: replace the node with one of its successors. The
         * choice of the successor depends on the balance of the node to remove,
         * as swapping with a node in a heavier subtree can reduce the number of
         * rotations needed to rebalance the tree.
         */

        old_balance = avltree_node_balance(node);
        right = avltree_d2i(old_balance);
        left = 1 - right;

        successor = node->children[right];

        while (successor->children[left] != NULL)
            successor = successor->children[left];

        child = successor->children[right];
        parent = avltree_node_parent(node);

        if (unlikely(parent == NULL))
            tree->root = successor;
        else
            parent->children[avltree_node_index(node, parent)] = successor;

        parent = avltree_node_parent(successor);
        index = avltree_node_index(successor, parent);

        /*
         * Set parent directly to keep the original balance.
         */
        successor->parent = node->parent;
        successor->children[left] = node->children[left];
        avltree_node_set_parent(successor->children[left], successor);

        if (node == parent)
            parent = successor;
        else {
            successor->children[right] = node->children[right];
            avltree_node_set_parent(successor->children[right], successor);
            parent->children[left] = child;

            if (child != NULL)
                avltree_node_set_parent(child, parent);
        }

        goto update_balance;
    }

    /*
     * Node has at most one child.
     */

    parent = avltree_node_parent(node);

    if (child != NULL)
        avltree_node_set_parent(child, parent);

    if (parent == NULL) {
        tree->root = child;
        return;
    } else {
        index = avltree_node_index(node, parent);
        parent->children[index] = child;
    }

    /*
     * The node has been removed, update the balance factors. At this point,
     * the node parent and the index of the node in its parent children array
     * must be valid.
     */
update_balance:
    for (;;) {
        node = parent;

        old_balance = avltree_node_balance(node);
        new_balance = old_balance - avltree_i2b(index);

        /*
         * The overall subtree height hasn't decreased, stop now.
         */
        if (old_balance == 0) {
            avltree_node_set_balance(node, new_balance);
            break;
        }

        parent = avltree_node_parent(node);

        if (parent != NULL)
            index = avltree_node_index(node, parent);

        /*
         * The overall subtree height has decreased. Update the current node and
         * iterate again to propagate the change.
         */
        if (new_balance == 0)
            avltree_node_set_balance(node, new_balance);
        else {
            int decreased;

            /*
             * The new balance is either -2 or 2. Rebalance the tree and exit
             * the loop if the rotation hasn't decreased the overall tree
             * height.
             */

            decreased = avltree_rotate(tree, node, new_balance);

            if (!decreased)
                break;
        }

        /*
         * The tree root was reached.
         */
        if (parent == NULL)
            break;
    }
}

struct avltree_node *
avltree_nearest(struct avltree_node *parent, int index, int direction)
{
    assert(avltree_check_index(direction));

    if (parent == NULL)
        return NULL;

    assert(avltree_check_index(index));

    if (index != direction)
        return parent;

    return avltree_walk(parent, direction);
}

struct avltree_node *
avltree_firstlast(const struct avltree *tree, int direction)
{
    struct avltree_node *prev, *cur;

    assert(avltree_check_index(direction));

    prev = NULL;

    for (cur = tree->root; cur != NULL; cur = cur->children[direction])
        prev = cur;

    return prev;
}

struct avltree_node *
avltree_walk(struct avltree_node *node, int direction)
{
    int left, right;

    assert(avltree_check_index(direction));

    left = direction;
    right = 1 - left;

    if (node == NULL)
        return NULL;

    if (node->children[left] != NULL) {
        node = node->children[left];

        while (node->children[right] != NULL)
            node = node->children[right];
    } else {
        struct avltree_node *parent;
        int index;

        for (;;) {
            parent = avltree_node_parent(node);

            if (parent == NULL)
                return NULL;

            index = avltree_node_index(node, parent);
            node = parent;

            if (index == right)
                break;
        }
    }

    return node;
}

struct avltree_node *
avltree_postwalk_deepest(const struct avltree *tree)
{
    struct avltree_node *node;

    node = tree->root;

    if (node == NULL)
        return NULL;

    return avltree_node_find_deepest(node);
}

struct avltree_node *
avltree_postwalk_unlink(struct avltree_node *node)
{
    struct avltree_node *parent;
    int index;

    if (node == NULL)
        return NULL;

    assert(node->children[AVLTREE_LEFT] == NULL);
    assert(node->children[AVLTREE_RIGHT] == NULL);

    parent = avltree_node_parent(node);

    if (parent == NULL)
        return NULL;

    index = avltree_node_index(node, parent);
    parent->children[index] = NULL;
    node = parent->children[AVLTREE_RIGHT];

    if (node == NULL)
        return parent;

    return avltree_node_find_deepest(node);
}
