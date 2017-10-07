/*
 * Copyright (c) 2017 Richard Braun.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
