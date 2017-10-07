/*
 * Copyright (c) 2009-2015 Richard Braun.
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
