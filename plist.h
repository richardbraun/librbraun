/*
 * Copyright (c) 2017 Richard Braun.
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
 * Priority list.
 *
 * This container acts as a doubly-linked list sorted by priority in
 * ascending order.
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#ifndef _PLIST_H
#define _PLIST_H

#include "list.h"

struct plist {
    struct list list;
    struct list prio_list;
};

struct plist_node {
    unsigned int priority;
    struct list node;
    struct list prio_node;
};

/*
 * Static priority list initializer.
 */
#define PLIST_INITIALIZER(plist) \
    { LIST_INITIALIZER((plist).list), LIST_INITIALIZER((plist).prio_list) }

/*
 * Initialize a priority list.
 */
static inline void
plist_init(struct plist *plist)
{
    list_init(&plist->list);
    list_init(&plist->prio_list);
}

/*
 * Initialize a priority list node.
 */
static inline void
plist_node_init(struct plist_node *pnode, unsigned int priority)
{
    pnode->priority = priority;
    list_node_init(&pnode->node);
    list_node_init(&pnode->prio_node);
}

/*
 * Return true if pnode is in no priority lists.
 */
static inline bool
plist_node_unlinked(const struct plist_node *pnode)
{
    return list_node_unlinked(&pnode->node);
}

/*
 * Macro that evaluates to the address of the structure containing the
 * given node based on the given type and member.
 */
#define plist_entry(pnode, type, member) structof(pnode, type, member)

/*
 * Return the first node of a priority list.
 */
static inline struct plist_node *
plist_first(const struct plist *plist)
{
    return list_first_entry(&plist->list, struct plist_node, node);
}

/*
 * Return the last node of a priority list.
 */
static inline struct plist_node *
plist_last(const struct plist *plist)
{
    return list_last_entry(&plist->list, struct plist_node, node);
}

/*
 * Return the node next to the given node.
 */
static inline struct plist_node *
plist_next(const struct plist_node *pnode)
{
    return (struct plist_node *)list_next_entry(pnode, node);
}

/*
 * Return the node previous to the given node.
 */
static inline struct plist_node *
plist_prev(const struct plist_node *pnode)
{
    return (struct plist_node *)list_prev_entry(pnode, node);
}

/*
 * Get the first entry of a priority list.
 */
#define plist_first_entry(plist, type, member) \
    plist_entry(plist_first(plist), type, member)

/*
 * Get the last entry of a priority list.
 */
#define plist_last_entry(plist, type, member) \
    plist_entry(plist_last(plist), type, member)

/*
 * Get the entry next to the given entry.
 */
#define plist_next_entry(entry, member) \
    list_next_entry(&(entry)->member, node)

/*
 * Get the entry previous to the given entry.
 */
#define plist_prev_entry(entry, member) \
    list_prev_entry(&(entry)->member, node)

/*
 * Return true if node is after the last or before the first node of
 * a priority list.
 */
static inline bool
plist_end(const struct plist *plist, const struct plist_node *pnode)
{
    return list_end(&plist->list, &pnode->node);
}

/*
 * Return true if plist is empty.
 */
static inline bool
plist_empty(const struct plist *plist)
{
    return list_empty(&plist->list);
}

/*
 * Return true if plist contains exactly one node.
 */
static inline bool
plist_singular(const struct plist *plist)
{
    return list_singular(&plist->list);
}

/*
 * Add a node to a priority list.
 *
 * The node must be initialized before calling this function.
 */
void plist_add(struct plist *plist, struct plist_node *pnode);

/*
 * Remove a node from a priority list.
 *
 * After completion, the node is stale.
 */
void plist_remove(struct plist *plist, struct plist_node *pnode);

/*
 * Forge a loop to process all nodes of a priority list.
 *
 * The node must not be altered during the loop.
 */
#define plist_for_each(plist, pnode)    \
for (pnode = plist_first(plist);        \
     !plist_end(plist, pnode);          \
     pnode = plist_next(pnode))

/*
 * Forge a loop to process all nodes of a priority list.
 */
#define plist_for_each_safe(plist, pnode, tmp)              \
for (pnode = plist_first(plist), tmp = plist_next(pnode);   \
     !plist_end(plist, pnode);                              \
     pnode = tmp, tmp = plist_next(pnode))

/*
 * Version of plist_for_each() that processes nodes backward.
 */
#define plist_for_each_reverse(plist, pnode)    \
for (pnode = plist_last(plist);                 \
     !plist_end(plist, pnode);                  \
     pnode = plist_prev(pnode))

/*
 * Version of plist_for_each_safe() that processes nodes backward.
 */
#define plist_for_each_reverse_safe(plist, pnode, tmp)      \
for (pnode = plist_last(plist), tmp = plist_prev(pnode);    \
     !plist_end(plist, pnode);                              \
     pnode = tmp, tmp = plist_prev(pnode))

/*
 * Forge a loop to process all entries of a priority list.
 *
 * The entry node must not be altered during the loop.
 */
#define plist_for_each_entry(plist, entry, member)      \
    list_for_each_entry(&(plist)->list, entry, member)

/*
 * Forge a loop to process all entries of a priority list.
 */
#define plist_for_each_entry_safe(plist, entry, tmp, member)        \
    list_for_each_entry_safe(&(plist)->list, entry, tmp, member)

/*
 * Version of plist_for_each_entry() that processes entries backward.
 */
#define plist_for_each_entry_reverse(plist, entry, member)      \
    list_for_each_entry_reverse(&(plist)->list, entry, member)

/*
 * Version of plist_for_each_entry_safe() that processes entries backward.
 */
#define plist_for_each_entry_reverse_safe(plist, entry, tmp, member)  \
    list_for_each_entry_reverse_safe(&(plist)->list, entry, tmp, member)

#endif /* _PLIST_H */
