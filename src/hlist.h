/*
 * Copyright (c) 2017 Richard Braun.
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
 * Doubly-linked list specialized for forward traversals and O(1) removals.
 */

#ifndef HLIST_H
#define HLIST_H

#include <stdbool.h>
#include <stddef.h>

#include "macros.h"

/*
 * List node.
 *
 * The pprev member points to another node member instead of another node,
 * so that it may safely refer to the first member of the list head. Its
 * main purpose is to allow O(1) removal.
 */
struct hlist_node {
    struct hlist_node *next;
    struct hlist_node **pprev;
};

/*
 * List head.
 */
struct hlist {
    struct hlist_node *first;
};

/*
 * Static list initializer.
 */
#define HLIST_INITIALIZER(list) { NULL }

/*
 * Initialize a list.
 */
static inline void
hlist_init(struct hlist *list)
{
    list->first = NULL;
}

/*
 * Initialize a list node.
 *
 * A node is in no list when its pprev member points to NULL.
 */
static inline void
hlist_node_init(struct hlist_node *node)
{
    node->pprev = NULL;
}

/*
 * Return true if node is in no list.
 */
static inline bool
hlist_node_unlinked(const struct hlist_node *node)
{
    return node->pprev == NULL;
}

/*
 * Return the first node of a list.
 */
static inline struct hlist_node *
hlist_first(const struct hlist *list)
{
    return list->first;
}

/*
 * Return the node next to the given node.
 */
static inline struct hlist_node *
hlist_next(const struct hlist_node *node)
{
    return node->next;
}

/*
 * Return true if node is invalid and denotes the end of the list.
 */
static inline bool
hlist_end(const struct hlist_node *node)
{
    return node == NULL;
}

/*
 * Return true if list is empty.
 */
static inline bool
hlist_empty(const struct hlist *list)
{
    return list->first == NULL;
}

/*
 * Return true if list contains exactly one node.
 */
static inline bool
hlist_singular(const struct hlist *list)
{
    return !hlist_empty(list) && hlist_end(list->first->next);
}

/*
 * Set the new head of a list.
 *
 * After completion, old_head is stale.
 */
static inline void
hlist_set_head(struct hlist *new_head, const struct hlist *old_head)
{
    *new_head = *old_head;

    if (!hlist_empty(new_head)) {
        new_head->first->pprev = &new_head->first;
    }
}

/*
 * Insert a node at the head of a list.
 */
static inline void
hlist_insert_head(struct hlist *list, struct hlist_node *node)
{
    struct hlist_node *first;

    first = list->first;
    node->next = first;
    node->pprev = &list->first;

    if (first != NULL) {
        first->pprev = &node->next;
    }

    list->first = node;
}

/*
 * Insert a node before another node.
 */
static inline void
hlist_insert_before(struct hlist_node *node, struct hlist_node *next)
{
    node->next = next;
    node->pprev = next->pprev;
    next->pprev = &node->next;
    *node->pprev = node;
}

/*
 * Insert a node after another node.
 */
static inline void
hlist_insert_after(struct hlist_node *node, struct hlist_node *prev)
{
    node->next = prev->next;
    node->pprev = &prev->next;

    if (node->next != NULL) {
        node->next->pprev = &node->next;
    }

    prev->next = node;
}

/*
 * Remove a node from a list.
 */
static inline void
hlist_remove(struct hlist_node *node)
{
    if (node->next != NULL) {
        node->next->pprev = node->pprev;
    }

    *node->pprev = node->next;
}

/*
 * Macro that evaluates to the address of the structure containing the
 * given node based on the given type and member.
 */
#define hlist_entry(node, type, member) structof(node, type, member)

/*
 * Get the first entry of a list.
 */
#define hlist_first_entry(list, type, member)                           \
MACRO_BEGIN                                                             \
    struct hlist_node *first_;                                          \
                                                                        \
    first_ = (list)->first;                                             \
    hlist_end(first_) ? NULL : hlist_entry(first_, type, member);       \
MACRO_END

/*
 * Get the entry next to the given entry.
 */
#define hlist_next_entry(entry, member)                                 \
MACRO_BEGIN                                                             \
    struct hlist_node *next_;                                           \
                                                                        \
    next_ = (entry)->member.next;                                       \
    hlist_end(next_)                                                    \
        ? NULL                                                          \
        : hlist_entry(next_, typeof(*entry), member);                   \
MACRO_END

/*
 * Forge a loop to process all nodes of a list.
 *
 * The node must not be altered during the loop.
 */
#define hlist_for_each(list, node)  \
for (node = hlist_first(list);      \
     !hlist_end(node);              \
     node = hlist_next(node))

/*
 * Forge a loop to process all nodes of a list.
 */
#define hlist_for_each_safe(list, node, tmp)            \
for (node = hlist_first(list),                          \
     tmp = hlist_end(node) ? NULL : hlist_next(node);   \
     !hlist_end(node);                                  \
     node = tmp,                                        \
     tmp = hlist_end(node) ? NULL : hlist_next(node))

/*
 * Forge a loop to process all entries of a list.
 *
 * The entry node must not be altered during the loop.
 */
#define hlist_for_each_entry(list, entry, member)               \
for (entry = hlist_first_entry(list, typeof(*entry), member);   \
     entry != NULL;                                             \
     entry = hlist_next_entry(entry, member))

/*
 * Forge a loop to process all entries of a list.
 */
#define hlist_for_each_entry_safe(list, entry, tmp, member)             \
for (entry = hlist_first_entry(list, typeof(*entry), member),           \
     tmp = (entry == NULL) ? NULL : hlist_next_entry(entry, member);    \
     entry != NULL;                                                     \
     entry = tmp,                                                       \
     tmp = (entry == NULL) ? NULL : hlist_next_entry(entry, member))    \

/*
 * Lockless variants
 *
 * The hlist_end() function may be used from read-side critical sections.
 */

/*
 * These macros can be replaced by actual functions in an environment
 * that provides lockless synchronization such as RCU.
 */
#define rcu_store_ptr(ptr, value)       ((ptr) = (value))
#define rcu_load_ptr(ptr)               (ptr)

/*
 * Return the first node of a list.
 */
static inline struct hlist_node *
hlist_rcu_first(const struct hlist *list)
{
    return rcu_load_ptr(list->first);
}

/*
 * Return the node next to the given node.
 */
static inline struct hlist_node *
hlist_rcu_next(const struct hlist_node *node)
{
    return rcu_load_ptr(node->next);
}

/*
 * Insert a node at the head of a list.
 */
static inline void
hlist_rcu_insert_head(struct hlist *list, struct hlist_node *node)
{
    struct hlist_node *first;

    first = list->first;
    node->next = first;
    node->pprev = &list->first;

    if (first != NULL) {
        first->pprev = &node->next;
    }

    rcu_store_ptr(list->first, node);
}

/*
 * Insert a node before another node.
 */
static inline void
hlist_rcu_insert_before(struct hlist_node *node, struct hlist_node *next)
{
    node->next = next;
    node->pprev = next->pprev;
    next->pprev = &node->next;
    rcu_store_ptr(*node->pprev, node);
}

/*
 * Insert a node after another node.
 */
static inline void
hlist_rcu_insert_after(struct hlist_node *node, struct hlist_node *prev)
{
    node->next = prev->next;
    node->pprev = &prev->next;

    if (node->next != NULL) {
        node->next->pprev = &node->next;
    }

    rcu_store_ptr(prev->next, node);
}

/*
 * Remove a node from a list.
 */
static inline void
hlist_rcu_remove(struct hlist_node *node)
{
    if (node->next != NULL) {
        node->next->pprev = node->pprev;
    }

    rcu_store_ptr(*node->pprev, node->next);
}

/*
 * Macro that evaluates to the address of the structure containing the
 * given node based on the given type and member.
 */
#define hlist_rcu_entry(node, type, member) \
    structof(rcu_load_ptr(node), type, member)

/*
 * Get the first entry of a list.
 */
#define hlist_rcu_first_entry(list, type, member)                       \
MACRO_BEGIN                                                             \
    struct hlist_node *first_;                                          \
                                                                        \
    first_ = hlist_rcu_first(list);                                     \
    hlist_end(first_) ? NULL : hlist_entry(first_, type, member);       \
MACRO_END

/*
 * Get the entry next to the given entry.
 */
#define hlist_rcu_next_entry(entry, member)                             \
MACRO_BEGIN                                                             \
    struct hlist_node *next_;                                           \
                                                                        \
    next_ = hlist_rcu_next(&entry->member);                             \
    hlist_end(next_)                                                    \
        ? NULL                                                          \
        : hlist_entry(next_, typeof(*entry), member);                   \
MACRO_END

/*
 * Forge a loop to process all nodes of a list.
 */
#define hlist_rcu_for_each(list, node)      \
for (node = hlist_rcu_first(list);          \
     !hlist_end(node);                      \
     node = hlist_rcu_next(node))

/*
 * Forge a loop to process all entries of a list.
 */
#define hlist_rcu_for_each_entry(list, entry, member)                   \
for (entry = hlist_rcu_first_entry(list, typeof(*entry), member);       \
     entry != NULL;                                                     \
     entry = hlist_rcu_next_entry(entry, member))

#endif /* HLIST_H */
