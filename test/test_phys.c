/*
 * Copyright (c) 2011 Richard Braun.
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
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "../phys.c"

int main(int argc, char *argv[])
{
    struct phys_page *page;

    (void)argc;
    (void)argv;

    phys_init();

    phys_info();
    printf("sizeof(struct phys_cpu_pool) = %zu\n",
           sizeof(struct phys_cpu_pool));
    printf("sizeof(struct phys_free_list) = %zu\n",
           sizeof(struct phys_free_list));
    printf("sizeof(struct phys_page): %zu\n", sizeof(struct phys_page));
    printf("sizeof(struct phys_seg): %zu\n", sizeof(struct phys_seg));
    printf("allocating two pages\n");
    page = phys_alloc_pages(PAGE_SIZE * 2);

    if (page == NULL) {
        fprintf(stderr, "unable to allocate memory\n");
        return 1;
    }

    phys_info();

    printf("freeing the allocated pages\n");
    phys_free_pages(page, PAGE_SIZE * 2);
    phys_info();

    return 0;
}
