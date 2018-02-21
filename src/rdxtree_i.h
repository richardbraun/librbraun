/*
 * Copyright (c) 2011-2017 Richard Braun.
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
 */

#ifndef RDXTREE_I_H
#define RDXTREE_I_H

/*
 * Radix tree.
 */
struct rdxtree {
    unsigned short height;
    unsigned short flags;
    void *root;
};

/*
 * Radix tree iterator.
 *
 * The node member refers to the node containing the current pointer, if any.
 * The key member refers to the current pointer, and is valid if and only if
 * rdxtree_walk() has been called at least once on the iterator.
 */
struct rdxtree_iter {
    void *node;
    rdxtree_key_t key;
};

/*
 * Initialize an iterator.
 */
static inline void
rdxtree_iter_init(struct rdxtree_iter *iter)
{
    iter->node = NULL;
    iter->key = (rdxtree_key_t)-1;
}

int rdxtree_insert_common(struct rdxtree *tree, rdxtree_key_t key,
                          void *ptr, void ***slotp);

int rdxtree_insert_alloc_common(struct rdxtree *tree, void *ptr,
                                rdxtree_key_t *keyp, void ***slotp);

void * rdxtree_lookup_common(const struct rdxtree *tree, rdxtree_key_t key,
                             int get_slot);

void * rdxtree_walk(struct rdxtree *tree, struct rdxtree_iter *iter);

#endif /* RDXTREE_I_H */
