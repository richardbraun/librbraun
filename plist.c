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
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#include "list.h"
#include "plist.h"

void
plist_add(struct plist *plist, struct plist_node *pnode)
{
    struct plist_node *next;

    if (plist_empty(plist)) {
        list_insert_head(&plist->list, &pnode->node);
        list_insert_head(&plist->prio_list, &pnode->prio_node);
        return;
    }

    list_for_each_entry(&plist->prio_list, next, prio_node) {
        if (pnode->priority < next->priority) {
            break;
        }
    }

    if (list_end(&plist->prio_list, &next->prio_node)
        || (pnode->priority != next->priority)) {
        list_insert_before(&next->prio_node, &pnode->prio_node);
    } else {
        list_init(&pnode->prio_node);
    }

    list_insert_before(&next->node, &pnode->node);
}

void
plist_remove(struct plist *plist, struct plist_node *pnode)
{
    struct plist_node *next;

    if (!list_node_unlinked(&pnode->prio_node)) {
        next = list_next_entry(pnode, node);

        if (!list_end(&plist->list, &next->node)
            && list_node_unlinked(&next->prio_node)) {
            list_insert_after(&pnode->prio_node, &next->prio_node);
        }

        list_remove(&pnode->prio_node);
    }

    list_remove(&pnode->node);
}
