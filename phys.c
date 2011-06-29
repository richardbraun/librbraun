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
 *
 *
 * Page allocator.
 *
 * This implementation uses the binary buddy system to manage its heap.
 * Descriptions of the buddy system can be found in the following works :
 * - "UNIX Internals: The New Frontiers", by Uresh Vahalia.
 * - "Dynamic Storage Allocation: A Survey and Critical Review",
 *    by Paul R. Wilson, Mark S. Johnstone, Michael Neely, and David Boles.
 *
 * In addition, this allocator uses per-cpu pools of pages for level 0
 * (i.e. single page) allocations. These pools act as caches (but are named
 * differently to avoid confusion with CPU caches) that reduce contention on
 * multiprocessor systems. When a pool is empty and cannot provide a page,
 * it is filled by transferring multiple pages from the backend buddy system.
 * The symmetric case is handled likewise.
 */

#include <sched.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>

#include "cpu.h"
#include "list.h"
#include "phys.h"
#include "error.h"
#include "macros.h"

/*
 * The system page size.
 *
 * This macro actually expands to a global variable that is set on
 * initialization.
 */
#define PAGE_SIZE ((unsigned long)_pagesize)

/*
 * Maximum number of segments.
 */
#define PHYS_MAX_SEGMENTS 2

/*
 * Segment boundaries.
 */
#define PHYS_ISADMA_LIMIT   0x1000000
#define PHYS_NORMAL_LIMIT   0x30000000

/*
 * Number of segment lists.
 */
#define PHYS_NR_SEG_LISTS 2

/*
 * Segment list priorities.
 *
 * Higher priorities have lower numerical values.
 */
#define PHYS_SEGLIST_NORMAL 1
#define PHYS_SEGLIST_ISADMA 0

/*
 * Number of free block lists per segment.
 */
#define PHYS_NR_FREE_LISTS 11

/*
 * The size of a CPU pool is computed by dividing the number of pages in its
 * containing segment by this value.
 */
#define PHYS_CPU_POOL_RATIO 1024

/*
 * Maximum number of pages in a CPU pool.
 */
#define PHYS_CPU_POOL_MAX_SIZE 128

/*
 * The transfer size of a CPU pool is computed by dividing the pool size by
 * this value.
 */
#define PHYS_CPU_POOL_TRANSFER_RATIO 2

/*
 * Per-processor cache of pages.
 */
struct phys_cpu_pool {
    pthread_mutex_t lock;
    int size;
    int transfer_size;
    int nr_pages;
    struct list pages;
} __aligned(CPU_L1_SIZE);

/*
 * Special level value.
 *
 * When a page is free, its level is the index of its free list.
 */
#define PHYS_LEVEL_ALLOCATED PHYS_NR_FREE_LISTS

/*
 * Doubly-linked list of free blocks.
 */
struct phys_free_list {
    unsigned long size;
    struct list blocks;
};

/*
 * Segment name buffer size.
 */
#define PHYS_NAME_SIZE 16

/*
 * Segment of contiguous memory.
 */
struct phys_seg {
    struct phys_cpu_pool cpu_pools[NR_CPUS];

    struct list node;
    phys_paddr_t start;
    phys_paddr_t end;
    struct phys_page *pages;
    struct phys_page *pages_end;
    pthread_mutex_t lock;
    struct phys_free_list free_lists[PHYS_NR_FREE_LISTS];
    unsigned long nr_free_pages;
    char name[PHYS_NAME_SIZE];
};

/*
 * See PAGE_SIZE.
 */
static long _pagesize;

/*
 * Segment lists, ordered by priority (higher priority lists have lower
 * numerical priorities).
 */
static struct list phys_seg_lists[PHYS_NR_SEG_LISTS];

/*
 * Segment table.
 */
static struct phys_seg phys_segs[PHYS_MAX_SEGMENTS];

/*
 * Number of loaded segments.
 */
static unsigned int phys_segs_size;

/*
 * Page/address conversion macros.
 */
#define phys_atop(addr) ((addr) / PAGE_SIZE)
#define phys_ptoa(pfn)  ((pfn) * PAGE_SIZE)

static void phys_page_init(struct phys_page *page, struct phys_seg *seg,
                           phys_paddr_t pa)
{
    page->seg = seg;
    page->phys_addr = pa;
    page->level = PHYS_LEVEL_ALLOCATED;
}

static inline struct phys_page * phys_page_lookup(phys_paddr_t pa)
{
    struct phys_seg *seg;
    unsigned int i;

    for (i = 0; i < phys_segs_size; i++) {
        seg = &phys_segs[i];

        if ((pa >= seg->start) && (pa < seg->end))
            return &seg->pages[phys_atop(pa - seg->start)];
    }

    return NULL;
}

static void phys_free_list_init(struct phys_free_list *free_list)
{
    free_list->size = 0;
    list_init(&free_list->blocks);
}

static inline void phys_free_list_insert(struct phys_free_list *free_list,
                                         struct phys_page *page)
{
    assert(page->level == PHYS_LEVEL_ALLOCATED);

    free_list->size++;
    list_insert(&free_list->blocks, &page->node);
}

static inline void phys_free_list_remove(struct phys_free_list *free_list,
                                         struct phys_page *page)
{
    assert(free_list->size != 0);
    assert(!list_empty(&free_list->blocks));
    assert(page->level < PHYS_NR_FREE_LISTS);

    free_list->size--;
    list_remove(&page->node);
}

static struct phys_page * phys_seg_alloc_from_buddy(struct phys_seg *seg,
                                                    unsigned int level)
{
    struct phys_free_list *free_list;
    struct phys_page *page, *buddy;
    unsigned int i;

    assert(level < PHYS_NR_FREE_LISTS);

    for (i = level; i < PHYS_NR_FREE_LISTS; i++) {
        free_list = &seg->free_lists[i];

        if (free_list->size != 0)
            break;
    }

    if (i == PHYS_NR_FREE_LISTS)
        return NULL;

    page = list_first_entry(&free_list->blocks, struct phys_page, node);
    phys_free_list_remove(free_list, page);
    page->level = PHYS_LEVEL_ALLOCATED;

    while (i > level) {
        i--;
        buddy = &page[1 << i];
        phys_free_list_insert(&seg->free_lists[i], buddy);
        buddy->level = i;
    }

    seg->nr_free_pages -= (1 << level);
    return page;
}

static void phys_seg_free_to_buddy(struct phys_seg *seg,
                                   struct phys_page *page,
                                   unsigned int level)
{
    struct phys_page *buddy;
    phys_paddr_t pa, buddy_pa;
    unsigned int nr_pages;

    assert(page >= seg->pages);
    assert(page < seg->pages_end);
    assert(page->level == PHYS_LEVEL_ALLOCATED);
    assert(level < PHYS_NR_FREE_LISTS);

    nr_pages = (1 << level);
    pa = page->phys_addr;

    while (level < (PHYS_NR_FREE_LISTS - 1)) {
        buddy_pa = pa ^ phys_ptoa(1 << level);

        if ((buddy_pa < seg->start) || (buddy_pa >= seg->end))
            break;

        buddy = &seg->pages[phys_atop(buddy_pa - seg->start)];

        if (buddy->level != level)
            break;

        phys_free_list_remove(&seg->free_lists[level], buddy);
        buddy->level = PHYS_LEVEL_ALLOCATED;
        level++;
        pa &= -phys_ptoa(1 << level);
        page = &seg->pages[phys_atop(pa - seg->start)];
    }

    phys_free_list_insert(&seg->free_lists[level], page);
    page->level = level;
    seg->nr_free_pages += nr_pages;
}

static void phys_cpu_pool_init(struct phys_cpu_pool *cpu_pool, int size)
{
    cpu_pool->size = size;
    cpu_pool->transfer_size = (size + PHYS_CPU_POOL_TRANSFER_RATIO - 1)
                              / PHYS_CPU_POOL_TRANSFER_RATIO;
    cpu_pool->nr_pages = 0;
    list_init(&cpu_pool->pages);
}

/*
 * Return a CPU pool.
 *
 * This function will generally return the pool matching the CPU running the
 * calling thread. Because of context switches and thread migration, the
 * caller might be running on another processor after this function returns.
 * Although not optimal, this should rarely happen, and it doesn't affect the
 * allocator operations in any other way, as CPU pools are always valid, and
 * their access is serialized by a lock.
 */
static inline struct phys_cpu_pool * phys_cpu_pool_get(struct phys_seg *seg)
{
    return &seg->cpu_pools[cpu_id()];
}

static inline struct phys_page *
phys_cpu_pool_pop(struct phys_cpu_pool *cpu_pool)
{
    struct phys_page *page;

    assert(cpu_pool->nr_pages != 0);
    cpu_pool->nr_pages--;
    page = list_first_entry(&cpu_pool->pages, struct phys_page, node);
    list_remove(&page->node);
    return page;
}

static inline void phys_cpu_pool_push(struct phys_cpu_pool *cpu_pool,
                                      struct phys_page *page)
{
    assert(cpu_pool->nr_pages < cpu_pool->size);
    cpu_pool->nr_pages++;
    list_insert(&cpu_pool->pages, &page->node);
}

static int phys_cpu_pool_fill(struct phys_cpu_pool *cpu_pool,
                              struct phys_seg *seg)
{
    struct phys_page *page;
    int i;

    assert(cpu_pool->nr_pages == 0);

    pthread_mutex_lock(&seg->lock);

    for (i = 0; i < cpu_pool->transfer_size; i++) {
        page = phys_seg_alloc_from_buddy(seg, 0);

        if (page == NULL)
            break;

        phys_cpu_pool_push(cpu_pool, page);
    }

    pthread_mutex_unlock(&seg->lock);

    return i;
}

static void phys_cpu_pool_drain(struct phys_cpu_pool *cpu_pool,
                                struct phys_seg *seg)
{
    struct phys_page *page;
    int i;

    assert(cpu_pool->nr_pages == cpu_pool->size);

    pthread_mutex_lock(&seg->lock);

    for (i = cpu_pool->transfer_size; i > 0; i--) {
        page = phys_cpu_pool_pop(cpu_pool);
        phys_seg_free_to_buddy(seg, page, 0);
    }

    pthread_mutex_unlock(&seg->lock);
}

static inline phys_paddr_t phys_seg_start(struct phys_seg *seg)
{
    return seg->start;
}

static inline phys_paddr_t phys_seg_end(struct phys_seg *seg)
{
    return seg->end;
}

static inline phys_paddr_t phys_seg_size(struct phys_seg *seg)
{
    return phys_seg_end(seg) - phys_seg_start(seg);
}

static int phys_seg_compute_pool_size(struct phys_seg *seg)
{
    phys_paddr_t size;

    size = phys_atop(phys_seg_size(seg)) / PHYS_CPU_POOL_RATIO;

    if (size == 0)
        size = 1;
    else if (size > PHYS_CPU_POOL_MAX_SIZE)
        size = PHYS_CPU_POOL_MAX_SIZE;

    return size;
}

static void phys_seg_init(struct phys_seg *seg, struct phys_page *pages)
{
    phys_paddr_t pa;
    int pool_size;
    unsigned int i;

    pool_size = phys_seg_compute_pool_size(seg);

    for (i = 0; i < ARRAY_SIZE(seg->cpu_pools); i++)
        phys_cpu_pool_init(&seg->cpu_pools[i], pool_size);

    seg->pages = pages;
    seg->pages_end = pages + phys_atop(phys_seg_size(seg));
    pthread_mutex_init(&seg->lock, NULL);

    for (i = 0; i < ARRAY_SIZE(seg->free_lists); i++)
        phys_free_list_init(&seg->free_lists[i]);

    seg->nr_free_pages = 0;

    for (pa = phys_seg_start(seg); pa < phys_seg_end(seg); pa += PAGE_SIZE)
        phys_page_init(&pages[phys_atop(pa - phys_seg_start(seg))], seg, pa);
}

/*
 * Return the level (i.e. the index in the free lists array) matching the
 * given size.
 */
static inline unsigned int phys_get_level(phys_size_t size)
{
    size = P2ROUND(size, PAGE_SIZE) / PAGE_SIZE;
    assert(size != 0);
    size--;

    if (size == 0)
        return 0;
    else
        return (sizeof(size) * CHAR_BIT) - __builtin_clzl(size);
}

static struct phys_page * phys_seg_alloc(struct phys_seg *seg, phys_size_t size)
{
    struct phys_cpu_pool *cpu_pool;
    struct phys_page *page;
    unsigned int level;
    int filled;

    level = phys_get_level(size);

    if (level == 0) {
        cpu_pool = phys_cpu_pool_get(seg);

        pthread_mutex_lock(&cpu_pool->lock);

        if (cpu_pool->nr_pages == 0) {
            filled = phys_cpu_pool_fill(cpu_pool, seg);

            if (!filled) {
                pthread_mutex_unlock(&cpu_pool->lock);
                return NULL;
            }
        }

        page = phys_cpu_pool_pop(cpu_pool);
        pthread_mutex_unlock(&cpu_pool->lock);
    } else {
        pthread_mutex_lock(&seg->lock);
        page = phys_seg_alloc_from_buddy(seg, level);
        pthread_mutex_unlock(&seg->lock);
    }

    return page;
}

static void phys_seg_free(struct phys_seg *seg, struct phys_page *page,
                          phys_size_t size)
{
    struct phys_cpu_pool *cpu_pool;
    unsigned int level;

    level = phys_get_level(size);

    if (level == 0) {
        cpu_pool = phys_cpu_pool_get(seg);

        pthread_mutex_lock(&cpu_pool->lock);

        if (cpu_pool->nr_pages == cpu_pool->size)
            phys_cpu_pool_drain(cpu_pool, seg);

        phys_cpu_pool_push(cpu_pool, page);
        pthread_mutex_unlock(&cpu_pool->lock);
    } else {
        pthread_mutex_lock(&seg->lock);
        phys_seg_free_to_buddy(seg, page, level);
        pthread_mutex_unlock(&seg->lock);
    }
}

/*
 * Load memory during initialization.
 *
 * This function partially initializes a segment.
 */
static void phys_load_segment(const char *name, phys_paddr_t start,
                              phys_paddr_t end, unsigned int seg_list_prio)
{
    static int initialized = 0;
    struct phys_seg *seg;
    struct list *seg_list;
    unsigned int i;

    assert(name != NULL);
    assert(start < end);
    assert(seg_list_prio < ARRAY_SIZE(phys_seg_lists));

    if (!initialized) {
        for (i = 0; i < ARRAY_SIZE(phys_seg_lists); i++)
            list_init(&phys_seg_lists[i]);

        phys_segs_size = 0;
        initialized = 1;
    }

    if (phys_segs_size >= ARRAY_SIZE(phys_segs))
        error_die(ERR_NORES);

    seg_list = &phys_seg_lists[seg_list_prio];
    seg = &phys_segs[phys_segs_size];

    list_insert_tail(seg_list, &seg->node);
    seg->start = start;
    seg->end = end;
    strncpy(seg->name, name, PHYS_NAME_SIZE);
    seg->name[sizeof(seg->name) - 1] = '\0';

    phys_segs_size++;
}

/*
 * Loading segments is normally done by architecture-specific code. In
 * this implementation, an Intel machine with two segments of RAM is
 * virtualized.
 */
static void phys_load_segments(void)
{
    phys_paddr_t start, end;
    size_t size;
    void *addr;

    /*
     * Load the ISADMA segment.
     */
    size = PHYS_ISADMA_LIMIT;
    addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (addr == MAP_FAILED)
        error_die(ERR_NOMEM);

    start = (phys_paddr_t)addr;
    end = start + size;
    phys_load_segment("isadma", start, end, PHYS_SEGLIST_ISADMA);

    /*
     * Load the normal segment.
     */
    size = PHYS_NORMAL_LIMIT - PHYS_ISADMA_LIMIT;
    addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (addr == MAP_FAILED)
        error_die(ERR_NOMEM);

    start = (phys_paddr_t)addr;
    end = start + size;
    phys_load_segment("normal", start, end, PHYS_SEGLIST_NORMAL);
}

void phys_init(void)
{
    struct phys_seg *seg, *map_seg;
    struct phys_page *page, *map;
    struct list *seg_list;
    phys_paddr_t map_size;
    unsigned int i;

    _pagesize = sysconf(_SC_PAGESIZE);
    assert(ISP2(_pagesize));

    phys_load_segments();

    /*
     * Compute the memory map size.
     */
    map_size = 0;

    for (i = 0; i < phys_segs_size; i++)
        map_size += phys_atop(phys_seg_size(&phys_segs[i]));

    map_size = P2ROUND(map_size * sizeof(struct phys_page), PAGE_SIZE);

    /*
     * Find a segment from which to allocate the memory map.
     */
    for (seg_list = &phys_seg_lists[ARRAY_SIZE(phys_seg_lists) - 1];
         seg_list >= phys_seg_lists;
         seg_list--)
        list_for_each_entry(seg_list, map_seg, node)
            if (map_size <= phys_seg_size(map_seg))
                goto found;

    error_die(ERR_NOMEM);

found:
    /*
     * Allocate the memory map.
     */
    map = (struct phys_page *)phys_seg_start(map_seg);

    /*
     * Initialize the segments, associating them to the memory map. When
     * the segments are initialized, all their pages are set allocated,
     * with a block size of one (level 0). They are then released, which
     * populates the free lists.
     */
    for (i = 0; i < phys_segs_size; i++) {
        seg = &phys_segs[i];
        phys_seg_init(seg, map);

        /*
         * Don't release the memory map pages.
         *
         * XXX The memory map pages normally don't need descriptors, as they
         * are never released. This implementation however can be used in
         * cases where some memory is reserved at the start of all segments.
         * In order not to require descriptors for the memory map, the segment
         * where the map resides should be split. As it is quite cumbersome,
         * no effort is made here to avoid wasting descriptors and the pages
         * containing them.
         */
        if (seg == map_seg)
            page = seg->pages + phys_atop(map_size);
        else
            page = seg->pages;

        while (page < seg->pages_end) {
            phys_seg_free_to_buddy(seg, page, 0);
            page++;
        }

        map += phys_atop(phys_seg_size(seg));
    }
}

struct phys_page * phys_alloc_pages(phys_size_t size)
{
    struct list *seg_list;
    struct phys_seg *seg;
    struct phys_page *page;

    for (seg_list = &phys_seg_lists[ARRAY_SIZE(phys_seg_lists) - 1];
         seg_list >= phys_seg_lists;
         seg_list--)
        list_for_each_entry(seg_list, seg, node) {
            page = phys_seg_alloc(seg, size);

            if (page != NULL)
                return page;
        }

    return NULL;
}

void phys_free_pages(struct phys_page *page, phys_size_t size)
{
    phys_seg_free(page->seg, page, size);
}

phys_paddr_t phys_alloc(phys_size_t size)
{
    struct phys_page *page;

    page = phys_alloc_pages(size);

    /*
     * XXX Rely on the system to never provide a virtual memory area
     * starting at 0.
     */
    if (page == NULL)
        return 0;

    return page->phys_addr;
}

void phys_free(phys_paddr_t pa, phys_size_t size)
{
    struct phys_page *page;

    page = phys_page_lookup(pa);
    assert(page != NULL);
    phys_free_pages(page, size);
}

void phys_info(void)
{
    struct phys_seg *seg;
    unsigned int i, j;
    char name[16];

    printf("    name");

    for (i = 0; i < PHYS_NR_FREE_LISTS; i++) {
        snprintf(name, sizeof(name), "#%u", (1 << i));
        printf(" %5s", name);
    }

    printf("\n");

    for (i = 0; i < phys_segs_size; i++) {
        seg = &phys_segs[i];

        printf("%8s", seg->name);

        pthread_mutex_lock(&seg->lock);

        for (j = 0; j < ARRAY_SIZE(seg->free_lists); j++)
            printf(" %5lu", seg->free_lists[j].size);

        pthread_mutex_unlock(&seg->lock);

        printf("\n");
    }
}
