/*
 * Copyright (c) 2009-2015 Richard Braun.
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
 * List sorting implementation.
 *
 * In addition to most common list operations, this implementation includes
 * a sort operation. The algorithm used is a variant of the bottom-up
 * mergesort algorithm. It has the following properties :
 *  - It is iterative (no recursion overhead).
 *  - It is stable (the relative order of equal entries is preserved).
 *  - It only requires constant additional space (as it works on linked lists).
 *  - It performs at O(n log n) for average and worst cases.
 *  - It is adaptive, performing faster on already sorted lists.
 *
 * Upstream site with license notes :
 * http://git.sceen.net/rbraun/librbraun.git/
 */

#include "list.h"

/*
 * Split input_list into two lists.
 *
 * The list_size first entries of input_list are moved into left_list while
 * the "right" list is actually the list_size first entries of input_list
 * after completion.
 */
static void
_list_divide(struct list *input_list, struct list *left_list,
             unsigned int list_size)
{
    struct list *node;
    unsigned int i;

    node = input_list->next;

    for (i = 0; (i < list_size) && !list_end(input_list, node); i++) {
        node = node->next;
    }

    list_split(left_list, input_list, node);
}

/*
 * Merge left_list and right_list at the tail of output_list.
 */
static void
_list_merge(struct list *left_list, struct list *right_list,
            struct list *output_list, unsigned int right_list_size,
            list_sort_cmp_fn_t cmp_fn)
{
    struct list *left, *right;

    left = left_list->prev;
    right = right_list->next;

    /*
     * Try to concatenate lists instead of merging first. This reduces
     * complexity for already sorted lists.
     */
    if (!list_end(left_list, left)
        && ((right_list_size > 0) && !list_end(right_list, right))
        && (cmp_fn(left, right) <= 0)) {
        struct list tmp_list;

        list_concat(output_list, left_list);
        list_init(left_list);

        while ((right_list_size > 0) && !list_end(right_list, right)) {
            right = right->next;
            right_list_size--;
        }

        list_split(&tmp_list, right_list, right);
        list_concat(output_list, &tmp_list);
        return;
    }

    left = left_list->next;

    while (!list_end(left_list, left)
           || ((right_list_size > 0) && !list_end(right_list, right)))
        if (((right_list_size == 0) || list_end(right_list, right))
            || (!list_end(left_list, left) && (cmp_fn(left, right) <= 0))) {
            list_remove(left);
            list_insert_tail(output_list, left);
            left = left_list->next;
        } else {
            list_remove(right);
            list_insert_tail(output_list, right);
            right = right_list->next;
            right_list_size--;
        }
}

void
list_sort(struct list *list, list_sort_cmp_fn_t cmp_fn)
{
    struct list left_list, output_list;
    unsigned int list_size, nr_merges;

    list_init(&left_list);
    list_init(&output_list);

    for (list_size = 1; /* no condition */; list_size <<= 1) {
        for (nr_merges = 0; /* no condition */; nr_merges++) {
            if (list_empty(list)) {
                break;
            }

            _list_divide(list, &left_list, list_size);
            _list_merge(&left_list, list, &output_list, list_size, cmp_fn);
        }

        list_concat(list, &output_list);
        list_init(&output_list);

        if (nr_merges <= 1) {
            return;
        }
    }
}
