/*
 * Copyright (c) 2010, 2011 Richard Braun.
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
 * Object caching and general purpose memory allocator.
 *
 * This allocator is based on the following works :
 * - "The Slab Allocator: An Object-Caching Kernel Memory Allocator",
 *   by Jeff Bonwick.
 *
 * It allows the allocation of objects (i.e. fixed-size typed buffers) from
 * caches and is efficient in both space and time. This implementation follows
 * many of the indications from the paper mentioned. The most notable
 * differences are outlined below.
 *
 * The per-cache self-scaling hash table for buffer-to-bufctl conversion,
 * described in 3.2.3 "Slab Layout for Large Objects", has been replaced by
 * an AVL tree storing slabs, sorted by address. The use of a self-balancing
 * tree for buffer-to-slab conversions provides a few advantages over a hash
 * table. Unlike a hash table, a BST provides a "lookup nearest" operation,
 * so obtaining the slab data (whether it is embedded in the slab or off
 * slab) from a buffer address simply consists of a "lookup nearest towards
 * 0" tree search. Storing slabs instead of buffers also considerably reduces
 * the number of elements to retain. Finally, a self-balancing tree is a true
 * self-scaling data structure, whereas a hash table requires periodic
 * maintenance and complete resizing, which is expensive. The only drawback is
 * that releasing a buffer to the slab layer takes logarithmic time instead of
 * constant time. But as the data set size is kept reasonable (because slabs
 * are stored instead of buffers) and because the CPU pool layer services most
 * requests, avoiding many accesses to the slab layer, it is considered an
 * acceptable tradeoff.
 *
 * This implementation uses per-cpu pools of objects, which service most
 * allocation requests. These pools act as caches (but are named differently
 * to avoid confusion with CPU caches) that reduce contention on multiprocessor
 * systems. When a pool is empty and cannot provide an object, it is filled by
 * transferring multiple objects from the slab layer. The symmetric case is
 * handled likewise.
 */

#include <time.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>

#include "cpu.h"
#include "mem.h"
#include "list.h"
#include "error.h"
#include "macros.h"
#include "avltree.h"

/*
 * The system page size.
 *
 * This macro actually expands to a global variable that is set on
 * initialization.
 */
#define PAGE_SIZE ((unsigned long)_pagesize)

/*
 * Minimum required alignment.
 */
#define MEM_ALIGN_MIN 8

/*
 * Minimum number of buffers per slab.
 *
 * This value is ignored when the slab size exceeds a threshold.
 */
#define MEM_MIN_BUFS_PER_SLAB 8

/*
 * Special slab size beyond which the minimum number of buffers per slab is
 * ignored when computing the slab size of a cache.
 */
#define MEM_SLAB_SIZE_THRESHOLD (8 * PAGE_SIZE)

/*
 * Special buffer size under which slab data is unconditionnally allocated
 * from its associated slab.
 */
#define MEM_BUF_SIZE_THRESHOLD (PAGE_SIZE / 8)

/*
 * Time (in seconds) between two garbage collection operations.
 */
#define MEM_GC_INTERVAL 15

/*
 * The transfer size of a CPU pool is computed by dividing the pool size by
 * this value.
 */
#define MEM_CPU_POOL_TRANSFER_RATIO 2

/*
 * Shift for the first general cache size.
 */
#define MEM_CACHES_FIRST_SHIFT 5

/*
 * Number of caches backing general purpose allocations.
 */
#define MEM_NR_MEM_CACHES 13

/*
 * Per-processor cache of pre-constructed objects.
 *
 * The flags member is a read-only CPU-local copy of the parent cache flags.
 */
struct mem_cpu_pool {
    pthread_mutex_t lock;
    int flags;
    int size;
    int transfer_size;
    int nr_objs;
    void **array;
} __aligned(CPU_L1_SIZE);

/*
 * When a cache is created, its CPU pool type is determined from the buffer
 * size. For small buffer sizes, many objects can be cached in a CPU pool.
 * Conversely, for large buffer sizes, this would incur much overhead, so only
 * a few objects are stored in a CPU pool.
 */
struct mem_cpu_pool_type {
    size_t buf_size;
    int array_size;
    size_t array_align;
    struct mem_cache *array_cache;
};

/*
 * Buffer descriptor.
 *
 * For normal caches (i.e. without MEM_CF_VERIFY), bufctls are located at the
 * end of (but inside) each buffer. If MEM_CF_VERIFY is set, bufctls are located
 * after each buffer.
 *
 * When an object is allocated to a client, its bufctl isn't used. This memory
 * is instead used for redzoning if cache debugging is in effect.
 */
union mem_bufctl {
    union mem_bufctl *next;
    unsigned long redzone;
};

/*
 * Redzone guard word.
 */
#ifdef __LP64__
#if _HOST_BIG_ENDIAN
#define MEM_REDZONE_WORD 0xfeedfacefeedfaceUL
#else /* _HOST_BIG_ENDIAN */
#define MEM_REDZONE_WORD 0xcefaedfecefaedfeUL
#endif /* _HOST_BIG_ENDIAN */
#else /* __LP64__ */
#if _HOST_BIG_ENDIAN
#define MEM_REDZONE_WORD 0xfeedfaceUL
#else /* _HOST_BIG_ENDIAN */
#define MEM_REDZONE_WORD 0xcefaedfeUL
#endif /* _HOST_BIG_ENDIAN */
#endif /* __LP64__ */

/*
 * Redzone byte for padding.
 */
#define MEM_REDZONE_BYTE 0xbb

/*
 * Buffer tag.
 *
 * This structure is only used for MEM_CF_VERIFY caches. It is located after
 * the bufctl and includes information about the state of the buffer it
 * describes (allocated or not). It should be thought of as a debugging
 * extension of the bufctl.
 */
struct mem_buftag {
    unsigned long state;
};

/*
 * Values the buftag state member can take.
 */
#ifdef __LP64__
#if _HOST_BIG_ENDIAN
#define MEM_BUFTAG_ALLOC    0xa110c8eda110c8edUL
#define MEM_BUFTAG_FREE     0xf4eeb10cf4eeb10cUL
#else /* _HOST_BIG_ENDIAN */
#define MEM_BUFTAG_ALLOC    0xedc810a1edc810a1UL
#define MEM_BUFTAG_FREE     0x0cb1eef40cb1eef4UL
#endif /* _HOST_BIG_ENDIAN */
#else /* __LP64__ */
#if _HOST_BIG_ENDIAN
#define MEM_BUFTAG_ALLOC    0xa110c8edUL
#define MEM_BUFTAG_FREE     0xf4eeb10cUL
#else /* _HOST_BIG_ENDIAN */
#define MEM_BUFTAG_ALLOC    0xedc810a1UL
#define MEM_BUFTAG_FREE     0x0cb1eef4UL
#endif /* _HOST_BIG_ENDIAN */
#endif /* __LP64__ */

/*
 * Free and uninitialized patterns.
 *
 * These values are unconditionnally 64-bit wide since buffers are at least
 * 8-byte aligned.
 */
#if _HOST_BIG_ENDIAN
#define MEM_FREE_PATTERN    0xdeadbeefdeadbeefULL
#define MEM_UNINIT_PATTERN  0xbaddcafebaddcafeULL
#else /* _HOST_BIG_ENDIAN */
#define MEM_FREE_PATTERN    0xefbeaddeefbeaddeULL
#define MEM_UNINIT_PATTERN  0xfecaddbafecaddbaULL
#endif /* _HOST_BIG_ENDIAN */

/*
 * Page-aligned collection of unconstructed buffers.
 */
struct mem_slab {
    struct list list_node;
    struct avltree_node tree_node;
    unsigned long nr_refs;
    union mem_bufctl *first_free;
    void *addr;
};

/*
 * Private cache creation flags.
 */
#define MEM_CREATE_INTERNAL     0x0100  /* Prevent off slab data        */

/*
 * Cache name buffer size.
 */
#define MEM_NAME_SIZE 32

/*
 * Cache flags.
 *
 * The flags don't change once set and can be tested without locking.
 */
#define MEM_CF_DIRECT           0x0001  /* No buf-to-slab tree lookup   */
#define MEM_CF_SLAB_EXTERNAL    0x0002  /* Slab data is off slab        */

/*
 * Debugging flags
 */
#define MEM_CF_VERIFY           0x0100  /* Use debugging facilities     */

/*
 * Cache of objects.
 *
 * Locking order : cpu_pool -> cache. CPU pools locking is ordered by CPU ID.
 *
 * The partial slabs list is sorted by slab references. Slabs with a high
 * number of references are placed first on the list to reduce fragmentation.
 * Sorting occurs at insertion/removal of buffers in a slab. As the list
 * is maintained sorted, and the number of references only changes by one,
 * this is a very cheap operation in the average case and the worst (linear)
 * case is very unlikely.
 */
struct mem_cache {
    /* CPU pool layer */
    struct mem_cpu_pool cpu_pools[NR_CPUS];
    struct mem_cpu_pool_type *cpu_pool_type;

    /* Slab layer */
    pthread_mutex_t lock;
    struct list node;   /* Cache list linkage   */
    struct list partial_slabs;
    struct list free_slabs;
    struct avltree active_slabs;
    int flags;
    size_t obj_size;    /* User-provided size   */
    size_t align;
    size_t buf_size;    /* Aligned object size  */
    size_t bufctl_dist; /* Distance from buffer to bufctl   */
    size_t slab_size;
    size_t color;
    size_t color_max;
    unsigned long bufs_per_slab;
    unsigned long nr_objs;  /* Number of allocated objects  */
    unsigned long nr_bufs;  /* Total number of buffers      */
    unsigned long nr_slabs;
    unsigned long nr_free_slabs;
    mem_cache_ctor_t ctor;
    struct mem_source source;
    char name[MEM_NAME_SIZE];
    size_t buftag_dist; /* Distance from buffer to buftag  */
    size_t redzone_pad; /* Bytes from end of object to redzone word */
};

/*
 * Options for mem_cache_alloc_verify().
 */
#define MEM_AV_NOCONSTRUCT  0
#define MEM_AV_CONSTRUCT    1

/*
 * Error codes for mem_cache_error().
 */
#define MEM_ERR_INVALID     0   /* Invalid address being freed  */
#define MEM_ERR_DOUBLEFREE  1   /* Freeing already free address */
#define MEM_ERR_BUFTAG      2   /* Invalid buftag content       */
#define MEM_ERR_MODIFIED    3   /* Buffer modified while free   */
#define MEM_ERR_REDZONE     4   /* Redzone violation            */

/*
 * See PAGE_SIZE.
 */
static long _pagesize;

/*
 * Available CPU pool types.
 *
 * For each entry, the CPU pool size applies from the entry buf_size
 * (excluded) up to (and including) the buf_size of the preceding entry.
 *
 * See struct cpu_pool_type for a description of the values.
 */
static struct mem_cpu_pool_type mem_cpu_pool_types[] = {
    {  32768,   1, 0,           NULL },
    {   4096,   8, CPU_L1_SIZE, NULL },
    {    256,  64, CPU_L1_SIZE, NULL },
    {      0, 128, CPU_L1_SIZE, NULL }
};

/*
 * Caches where CPU pool arrays are allocated from.
 */
static struct mem_cache mem_cpu_array_caches[ARRAY_SIZE(mem_cpu_pool_types)];

/*
 * Cache for off slab data.
 */
static struct mem_cache mem_slab_cache;

/*
 * Cache for dynamically created caches.
 */
static struct mem_cache mem_cache_cache;

/*
 * General caches array.
 */
static struct mem_cache mem_caches[MEM_NR_MEM_CACHES];

/*
 * List of all caches managed by the allocator.
 */
static struct list mem_cache_list;
static pthread_mutex_t mem_cache_list_lock;

/*
 * Default backend functions.
 */
static void * mem_default_alloc(size_t size);
static void mem_default_free(void *ptr, size_t size);

/*
 * Default source of memory.
 */
static struct mem_source mem_default_source = {
    mem_default_alloc,
    mem_default_free
};

#define mem_error(format, ...)                                  \
    fprintf(stderr, "mem: error: %s(): " format "\n", __func__, \
            ## __VA_ARGS__)

#define mem_warn(format, ...)                                       \
    fprintf(stderr, "mem: warning: %s(): " format "\n", __func__,   \
            ## __VA_ARGS__)

#define mem_print(format, ...)                      \
    fprintf(stderr, format "\n", ## __VA_ARGS__)

static void mem_cache_error(struct mem_cache *cache, void *buf, int error,
                            void *arg);
static void * mem_cache_alloc_from_slab(struct mem_cache *cache);
static void mem_cache_free_to_slab(struct mem_cache *cache, void *buf);

#ifdef CONFIG_MEM_USE_PHYS
#include "phys.h"

static void *
mem_default_alloc(size_t size)
{
    return (void *)phys_alloc(size);
}

static void
mem_default_free(void *ptr, size_t size)
{
    phys_free((phys_paddr_t)ptr, size);
}
#else /* CONFIG_MEM_USE_PHYS */
static void *
mem_default_alloc(size_t size)
{
    void *addr;

    addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (addr == MAP_FAILED)
        return NULL;

    return addr;
}

static void
mem_default_free(void *ptr, size_t size)
{
    munmap(ptr, size);
}
#endif /* CONFIG_MEM_USE_PHYS */

static void *
mem_buf_verify_bytes(void *buf, void *pattern, size_t size)
{
    char *ptr, *pattern_ptr, *end;

    end = buf + size;

    for (ptr = buf, pattern_ptr = pattern; ptr < end; ptr++, pattern_ptr++)
        if (*ptr != *pattern_ptr)
            return ptr;

    return NULL;
}

static void *
mem_buf_verify(void *buf, uint64_t pattern, size_t size)
{
    uint64_t *ptr, *end;

    assert(P2ALIGNED((unsigned long)buf, sizeof(uint64_t)));
    assert(P2ALIGNED(size, sizeof(uint64_t)));

    end = buf + size;

    for (ptr = buf; ptr < end; ptr++)
        if (*ptr != pattern)
            return mem_buf_verify_bytes(ptr, &pattern, sizeof(pattern));

    return NULL;
}

static void
mem_buf_fill(void *buf, uint64_t pattern, size_t size)
{
    uint64_t *ptr, *end;

    assert(P2ALIGNED((unsigned long)buf, sizeof(uint64_t)));
    assert(P2ALIGNED(size, sizeof(uint64_t)));

    end = buf + size;

    for (ptr = buf; ptr < end; ptr++)
        *ptr = pattern;
}

static void *
mem_buf_verify_fill(void *buf, uint64_t old, uint64_t new, size_t size)
{
    uint64_t *ptr, *end;

    assert(P2ALIGNED((unsigned long)buf, sizeof(uint64_t)));
    assert(P2ALIGNED(size, sizeof(uint64_t)));

    end = buf + size;

    for (ptr = buf; ptr < end; ptr++) {
        if (*ptr != old)
            return mem_buf_verify_bytes(ptr, &old, sizeof(old));

        *ptr = new;
    }

    return NULL;
}

static inline union mem_bufctl *
mem_buf_to_bufctl(void *buf, struct mem_cache *cache)
{
    return (union mem_bufctl *)(buf + cache->bufctl_dist);
}

static inline struct mem_buftag *
mem_buf_to_buftag(void *buf, struct mem_cache *cache)
{
    return (struct mem_buftag *)(buf + cache->buftag_dist);
}

static inline void *
mem_bufctl_to_buf(union mem_bufctl *bufctl, struct mem_cache *cache)
{
    return (void *)bufctl - cache->bufctl_dist;
}

static void
mem_slab_create_verify(struct mem_slab *slab, struct mem_cache *cache)
{
    struct mem_buftag *buftag;
    size_t buf_size;
    unsigned long buffers;
    void *buf;

    buf_size = cache->buf_size;
    buf = slab->addr;
    buftag = mem_buf_to_buftag(buf, cache);

    for (buffers = cache->bufs_per_slab; buffers != 0; buffers--) {
        mem_buf_fill(buf, MEM_FREE_PATTERN, cache->bufctl_dist);
        buftag->state = MEM_BUFTAG_FREE;
        buf += buf_size;
        buftag = mem_buf_to_buftag(buf, cache);
    }
}

/*
 * Create an empty slab for a cache.
 *
 * The caller must drop all locks before calling this function.
 */
static struct mem_slab *
mem_slab_create(struct mem_cache *cache, size_t color)
{
    struct mem_slab *slab;
    union mem_bufctl *bufctl;
    size_t buf_size;
    unsigned long buffers;
    void *slab_buf;

    slab_buf = cache->source.alloc_fn(cache->slab_size);

    if (slab_buf == NULL)
        return NULL;

    if (cache->flags & MEM_CF_SLAB_EXTERNAL) {
        slab = mem_cache_alloc(&mem_slab_cache);

        if (slab == NULL) {
            cache->source.free_fn(slab_buf, cache->slab_size);
            return NULL;
        }
    } else {
        slab = (struct mem_slab *)(slab_buf + cache->slab_size) - 1;
    }

    list_node_init(&slab->list_node);
    avltree_node_init(&slab->tree_node);
    slab->nr_refs = 0;
    slab->first_free = NULL;
    slab->addr = slab_buf + color;

    buf_size = cache->buf_size;
    bufctl = mem_buf_to_bufctl(slab->addr, cache);

    for (buffers = cache->bufs_per_slab; buffers != 0; buffers--) {
        bufctl->next = slab->first_free;
        slab->first_free = bufctl;
        bufctl = (union mem_bufctl *)((void *)bufctl + buf_size);
    }

    if (cache->flags & MEM_CF_VERIFY)
        mem_slab_create_verify(slab, cache);

    return slab;
}

static void
mem_slab_destroy_verify(struct mem_slab *slab, struct mem_cache *cache)
{
    struct mem_buftag *buftag;
    size_t buf_size;
    unsigned long buffers;
    void *buf, *addr;

    buf_size = cache->buf_size;
    buf = slab->addr;
    buftag = mem_buf_to_buftag(buf, cache);

    for (buffers = cache->bufs_per_slab; buffers != 0; buffers--) {
        if (buftag->state != MEM_BUFTAG_FREE)
            mem_cache_error(cache, buf, MEM_ERR_BUFTAG, buftag);

        addr = mem_buf_verify(buf, MEM_FREE_PATTERN, cache->bufctl_dist);

        if (addr != NULL)
            mem_cache_error(cache, buf, MEM_ERR_MODIFIED, addr);

        buf += buf_size;
        buftag = mem_buf_to_buftag(buf, cache);
    }
}

/*
 * Destroy a slab.
 *
 * The caller must drop all locks before calling this function.
 */
static void
mem_slab_destroy(struct mem_slab *slab, struct mem_cache *cache)
{
    void *slab_buf;

    assert(slab->nr_refs == 0);
    assert(slab->first_free != NULL);

    if (cache->flags & MEM_CF_VERIFY)
        mem_slab_destroy_verify(slab, cache);

    slab_buf = (void *)P2ALIGN((unsigned long)slab->addr, PAGE_SIZE);
    cache->source.free_fn(slab_buf, cache->slab_size);

    if (cache->flags & MEM_CF_SLAB_EXTERNAL)
        mem_cache_free(&mem_slab_cache, slab);
}

static inline int
mem_slab_use_tree(int flags)
{
    return !(flags & MEM_CF_DIRECT) || (flags & MEM_CF_VERIFY);
}

static inline int
mem_slab_cmp_lookup(const void *addr, const struct avltree_node *node)
{
    struct mem_slab *slab;

    slab = avltree_entry(node, struct mem_slab, tree_node);

    if (addr == slab->addr)
        return 0;
    else if (addr < slab->addr)
        return -1;
    else
        return 1;
}

static inline int
mem_slab_cmp_insert(const struct avltree_node *a, const struct avltree_node *b)
{
    struct mem_slab *slab;

    slab = avltree_entry(a, struct mem_slab, tree_node);
    return mem_slab_cmp_lookup(slab->addr, b);
}

static void
mem_cpu_pool_init(struct mem_cpu_pool *cpu_pool, struct mem_cache *cache)
{
    pthread_mutex_init(&cpu_pool->lock, NULL);
    cpu_pool->flags = cache->flags;
    cpu_pool->size = 0;
    cpu_pool->transfer_size = 0;
    cpu_pool->nr_objs = 0;
    cpu_pool->array = NULL;
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
static inline struct mem_cpu_pool *
mem_cpu_pool_get(struct mem_cache *cache)
{
    return &cache->cpu_pools[cpu_id()];
}

static inline void
mem_cpu_pool_build(struct mem_cpu_pool *cpu_pool, struct mem_cache *cache,
                   void **array)
{
    cpu_pool->size = cache->cpu_pool_type->array_size;
    cpu_pool->transfer_size = (cpu_pool->size + MEM_CPU_POOL_TRANSFER_RATIO - 1)
                              / MEM_CPU_POOL_TRANSFER_RATIO;
    cpu_pool->array = array;
}

static inline void *
mem_cpu_pool_pop(struct mem_cpu_pool *cpu_pool)
{
    cpu_pool->nr_objs--;
    return cpu_pool->array[cpu_pool->nr_objs];
}

static inline void
mem_cpu_pool_push(struct mem_cpu_pool *cpu_pool, void *obj)
{
    cpu_pool->array[cpu_pool->nr_objs] = obj;
    cpu_pool->nr_objs++;
}

static int
mem_cpu_pool_fill(struct mem_cpu_pool *cpu_pool, struct mem_cache *cache)
{
    void *obj;
    int i;

    pthread_mutex_lock(&cache->lock);

    for (i = 0; i < cpu_pool->transfer_size; i++) {
        obj = mem_cache_alloc_from_slab(cache);

        if (obj == NULL)
            break;

        mem_cpu_pool_push(cpu_pool, obj);
    }

    pthread_mutex_unlock(&cache->lock);

    return i;
}

static void
mem_cpu_pool_drain(struct mem_cpu_pool *cpu_pool, struct mem_cache *cache)
{
    void *obj;
    int i;

    pthread_mutex_lock(&cache->lock);

    for (i = cpu_pool->transfer_size; i > 0; i--) {
        obj = mem_cpu_pool_pop(cpu_pool);
        mem_cache_free_to_slab(cache, obj);
    }

    pthread_mutex_unlock(&cache->lock);
}

static void
mem_cache_error(struct mem_cache *cache, void *buf, int error, void *arg)
{
    struct mem_buftag *buftag;

    mem_error("cache: %s, buffer: %p", cache->name, buf);

    switch(error) {
    case MEM_ERR_INVALID:
        mem_error("freeing invalid address");
        break;
    case MEM_ERR_DOUBLEFREE:
        mem_error("attempting to free the same address twice");
        break;
    case MEM_ERR_BUFTAG:
        mem_error("invalid buftag content");
        buftag = arg;
        mem_error("buftag state: %p", (void *)buftag->state);
        break;
    case MEM_ERR_MODIFIED:
        mem_error("free buffer modified");
        mem_error("fault address: %p, offset in buffer: %td", arg, arg - buf);
        break;
    case MEM_ERR_REDZONE:
        mem_error("write beyond end of buffer");
        mem_error("fault address: %p, offset in buffer: %td", arg, arg - buf);
        break;
    default:
        mem_error("unknown error");
    }

    error_die(ERR_MEM_CACHE);

    /*
     * Never reached.
     */
}

/*
 * Compute an appropriate slab size for the given cache.
 *
 * Once the slab size is known, this function sets the related properties
 * (buffers per slab and maximum color). It can also set the MEM_CF_DIRECT
 * and/or MEM_CF_SLAB_EXTERNAL flags depending on the resulting layout.
 */
static void
mem_cache_compute_sizes(struct mem_cache *cache, int flags)
{
    size_t i, buffers, buf_size, slab_size, free_slab_size, optimal_size;
    size_t waste, waste_min;
    int embed, optimal_embed;

    buf_size = cache->buf_size;

    if (buf_size < MEM_BUF_SIZE_THRESHOLD)
        flags |= MEM_CREATE_INTERNAL;

    i = 0;
    waste_min = (size_t)-1;

    do {
        i++;
        slab_size = P2ROUND(i * buf_size, PAGE_SIZE);
        free_slab_size = slab_size;

        if (flags & MEM_CREATE_INTERNAL)
            free_slab_size -= sizeof(struct mem_slab);

        buffers = free_slab_size / buf_size;
        waste = free_slab_size % buf_size;

        if (buffers > i)
            i = buffers;

        if (flags & MEM_CREATE_INTERNAL)
            embed = 1;
        else if (sizeof(struct mem_slab) <= waste) {
            embed = 1;
            waste -= sizeof(struct mem_slab);
        } else {
            embed = 0;
        }

        if (waste <= waste_min) {
            waste_min = waste;
            optimal_size = slab_size;
            optimal_embed = embed;
        }
    } while ((buffers < MEM_MIN_BUFS_PER_SLAB)
             && (slab_size < MEM_SLAB_SIZE_THRESHOLD));

    assert(!(flags & MEM_CREATE_INTERNAL) || optimal_embed);

    cache->slab_size = optimal_size;
    slab_size = cache->slab_size - (optimal_embed
                ? sizeof(struct mem_slab)
                : 0);
    cache->bufs_per_slab = slab_size / buf_size;
    cache->color_max = slab_size % buf_size;

    if (cache->color_max >= PAGE_SIZE)
        cache->color_max = PAGE_SIZE - 1;

    if (optimal_embed) {
        if (cache->slab_size == PAGE_SIZE)
            cache->flags |= MEM_CF_DIRECT;
    } else {
        cache->flags |= MEM_CF_SLAB_EXTERNAL;
    }
}

static void
mem_cache_init(struct mem_cache *cache, const char *name,
               size_t obj_size, size_t align, mem_cache_ctor_t ctor,
               const struct mem_source *source, int flags)
{
    struct mem_cpu_pool_type *cpu_pool_type;
    size_t i, buf_size;

#ifdef CONFIG_MEM_VERIFY
    cache->flags = MEM_CF_VERIFY;
#else
    cache->flags = 0;
#endif

    if (flags & MEM_CACHE_VERIFY)
        cache->flags |= MEM_CF_VERIFY;

    if (align < MEM_ALIGN_MIN)
        align = MEM_ALIGN_MIN;

    assert(obj_size > 0);
    assert(ISP2(align));
    assert(align < PAGE_SIZE);

    buf_size = P2ROUND(obj_size, align);

    if (source == NULL)
        source = &mem_default_source;

    pthread_mutex_init(&cache->lock, NULL);
    list_node_init(&cache->node);
    list_init(&cache->partial_slabs);
    list_init(&cache->free_slabs);
    avltree_init(&cache->active_slabs);
    cache->obj_size = obj_size;
    cache->align = align;
    cache->buf_size = buf_size;
    cache->bufctl_dist = buf_size - sizeof(union mem_bufctl);
    cache->color = 0;
    cache->nr_objs = 0;
    cache->nr_bufs = 0;
    cache->nr_slabs = 0;
    cache->nr_free_slabs = 0;
    cache->ctor = ctor;
    cache->source = *source;
    strncpy(cache->name, name, MEM_NAME_SIZE);
    cache->name[MEM_NAME_SIZE - 1] = '\0';
    cache->buftag_dist = 0;
    cache->redzone_pad = 0;

    if (cache->flags & MEM_CF_VERIFY) {
        cache->bufctl_dist = buf_size;
        cache->buftag_dist = cache->bufctl_dist + sizeof(union mem_bufctl);
        cache->redzone_pad = cache->bufctl_dist - cache->obj_size;
        buf_size += sizeof(union mem_bufctl) + sizeof(struct mem_buftag);
        buf_size = P2ROUND(buf_size, align);
        cache->buf_size = buf_size;
    }

    mem_cache_compute_sizes(cache, flags);

    for (cpu_pool_type = mem_cpu_pool_types;
         buf_size <= cpu_pool_type->buf_size;
         cpu_pool_type++);

    cache->cpu_pool_type = cpu_pool_type;

    for (i = 0; i < ARRAY_SIZE(cache->cpu_pools); i++)
        mem_cpu_pool_init(&cache->cpu_pools[i], cache);

    pthread_mutex_lock(&mem_cache_list_lock);
    list_insert_tail(&mem_cache_list, &cache->node);
    pthread_mutex_unlock(&mem_cache_list_lock);
}

struct mem_cache *
mem_cache_create(const char *name, size_t obj_size, size_t align,
                 mem_cache_ctor_t ctor, const struct mem_source *source,
                 int flags)
{
    struct mem_cache *cache;

    cache = mem_cache_alloc(&mem_cache_cache);

    if (cache == NULL)
        return NULL;

    mem_cache_init(cache, name, obj_size, align, ctor, source, flags);

    return cache;
}

static inline int
mem_cache_empty(struct mem_cache *cache)
{
    return cache->nr_objs == cache->nr_bufs;
}

static int
mem_cache_grow(struct mem_cache *cache)
{
    struct mem_slab *slab;
    size_t color;
    int empty;

    pthread_mutex_lock(&cache->lock);

    if (!mem_cache_empty(cache)) {
        pthread_mutex_unlock(&cache->lock);
        return 1;
    }

    color = cache->color;
    cache->color += cache->align;

    if (cache->color > cache->color_max)
        cache->color = 0;

    pthread_mutex_unlock(&cache->lock);

    slab = mem_slab_create(cache, color);

    pthread_mutex_lock(&cache->lock);

    if (slab != NULL) {
        list_insert_tail(&cache->free_slabs, &slab->list_node);
        cache->nr_bufs += cache->bufs_per_slab;
        cache->nr_slabs++;
        cache->nr_free_slabs++;
    }

    /*
     * Even if our slab creation failed, another thread might have succeeded
     * in growing the cache.
     */
    empty = mem_cache_empty(cache);

    pthread_mutex_unlock(&cache->lock);

    return !empty;
}

static void
mem_cache_reap(struct mem_cache *cache)
{
    struct mem_slab *slab;
    struct list dead_slabs;

    list_init(&dead_slabs);

    pthread_mutex_lock(&cache->lock);

    while (!list_empty(&cache->free_slabs)) {
        slab = list_first_entry(&cache->free_slabs, struct mem_slab, list_node);
        list_remove(&slab->list_node);
        list_insert(&dead_slabs, &slab->list_node);
        cache->nr_bufs -= cache->bufs_per_slab;
        cache->nr_slabs--;
        cache->nr_free_slabs--;
    }

    pthread_mutex_unlock(&cache->lock);

    while (!list_empty(&dead_slabs)) {
        slab = list_first_entry(&dead_slabs, struct mem_slab, list_node);
        list_remove(&slab->list_node);
        mem_slab_destroy(slab, cache);
    }
}

void
mem_cache_destroy(struct mem_cache *cache)
{
    struct mem_cpu_pool *cpu_pool;
    void **ptr;
    size_t i;

    pthread_mutex_lock(&mem_cache_list_lock);
    list_remove(&cache->node);
    pthread_mutex_unlock(&mem_cache_list_lock);

    for (i = 0; i < ARRAY_SIZE(cache->cpu_pools); i++) {
        cpu_pool = &cache->cpu_pools[i];

        pthread_mutex_lock(&cpu_pool->lock);

        if (cpu_pool->array == NULL) {
            pthread_mutex_unlock(&cpu_pool->lock);
            continue;
        }

        pthread_mutex_lock(&cache->lock);

        for (ptr = cpu_pool->array + cpu_pool->nr_objs - 1;
             ptr >= cpu_pool->array;
             ptr--)
            mem_cache_free_to_slab(cache, *ptr);

        pthread_mutex_unlock(&cache->lock);

        ptr = cpu_pool->array;
        cpu_pool->size = 0;
        cpu_pool->nr_objs = 0;
        cpu_pool->array = NULL;
        pthread_mutex_unlock(&cpu_pool->lock);

        mem_cache_free(cache->cpu_pool_type->array_cache, ptr);
    }

    mem_cache_reap(cache);

#ifndef NDEBUG
    if (cache->nr_objs != 0)
        mem_warn("'%s' not empty", cache->name);
    else {
        assert(list_empty(&cache->partial_slabs));
        assert(list_empty(&cache->free_slabs));
        assert(avltree_empty(&cache->active_slabs));
        assert(cache->nr_bufs == 0);
        assert(cache->nr_slabs == 0);
    }
#endif /* NDEBUG */

    pthread_mutex_destroy(&cache->lock);

    for (i = 0; i < ARRAY_SIZE(cache->cpu_pools); i++)
        pthread_mutex_destroy(&cache->cpu_pools[i].lock);

    mem_cache_free(&mem_cache_cache, cache);
}

/*
 * Allocate a raw (unconstructed) buffer from the slab layer of a cache.
 *
 * The cache must be locked before calling this function.
 */
static void *
mem_cache_alloc_from_slab(struct mem_cache *cache)
{
    struct mem_slab *slab;
    union mem_bufctl *bufctl;

    if (!list_empty(&cache->partial_slabs))
        slab = list_first_entry(&cache->partial_slabs, struct mem_slab,
                                list_node);
    else if (!list_empty(&cache->free_slabs))
        slab = list_first_entry(&cache->free_slabs, struct mem_slab, list_node);
    else
        return NULL;

    bufctl = slab->first_free;
    assert(bufctl != NULL);
    slab->first_free = bufctl->next;
    slab->nr_refs++;
    cache->nr_objs++;

    /*
     * The slab has become complete.
     */
    if (slab->nr_refs == cache->bufs_per_slab) {
        list_remove(&slab->list_node);

        if (slab->nr_refs == 1)
            cache->nr_free_slabs--;
    } else if (slab->nr_refs == 1) {
        /*
         * The slab has become partial.
         */
        list_remove(&slab->list_node);
        list_insert_tail(&cache->partial_slabs, &slab->list_node);
        cache->nr_free_slabs--;
    } else if (!list_singular(&cache->partial_slabs)) {
        struct list *node;
        struct mem_slab *tmp;

        /*
         * The slab remains partial. If there are more than one partial slabs,
         * maintain the list sorted.
         */

        assert(slab->nr_refs > 1);

        for (node = list_prev(&slab->list_node);
             !list_end(&cache->partial_slabs, node);
             node = list_prev(node)) {
            tmp = list_entry(node, struct mem_slab, list_node);

            if (tmp->nr_refs >= slab->nr_refs)
                break;
        }

        /*
         * If the direct neighbor was found, the list is already sorted.
         * If no slab was found, the slab is inserted at the head of the list.
         */
        if (node != list_prev(&slab->list_node)) {
            list_remove(&slab->list_node);
            list_insert_after(node, &slab->list_node);
        }
    }

    if ((slab->nr_refs == 1) && mem_slab_use_tree(cache->flags))
        avltree_insert(&cache->active_slabs, &slab->tree_node,
                       mem_slab_cmp_insert);

    return mem_bufctl_to_buf(bufctl, cache);
}

/*
 * Release a buffer to the slab layer of a cache.
 *
 * The cache must be locked before calling this function.
 */
static void
mem_cache_free_to_slab(struct mem_cache *cache, void *buf)
{
    struct mem_slab *slab;
    union mem_bufctl *bufctl;

    if (cache->flags & MEM_CF_DIRECT) {
        assert(cache->slab_size == PAGE_SIZE);
        slab = (struct mem_slab *)P2END((unsigned long)buf, cache->slab_size)
               - 1;
    } else {
        struct avltree_node *node;

        node = avltree_lookup_nearest(&cache->active_slabs, buf,
                                      mem_slab_cmp_lookup, AVLTREE_LEFT);
        assert(node != NULL);
        slab = avltree_entry(node, struct mem_slab, tree_node);
        assert((unsigned long)buf < (P2ALIGN((unsigned long)slab->addr
                                             + cache->slab_size, PAGE_SIZE)));
    }

    assert(slab->nr_refs >= 1);
    assert(slab->nr_refs <= cache->bufs_per_slab);
    bufctl = mem_buf_to_bufctl(buf, cache);
    bufctl->next = slab->first_free;
    slab->first_free = bufctl;
    slab->nr_refs--;
    cache->nr_objs--;

    /*
     * The slab has become free.
     */
    if (slab->nr_refs == 0) {
        if (mem_slab_use_tree(cache->flags))
            avltree_remove(&cache->active_slabs, &slab->tree_node);

        /*
         * The slab was partial.
         */
        if (cache->bufs_per_slab > 1)
            list_remove(&slab->list_node);

        list_insert_tail(&cache->free_slabs, &slab->list_node);
        cache->nr_free_slabs++;
    } else if (slab->nr_refs == (cache->bufs_per_slab - 1)) {
        /*
         * The slab has become partial.
         */
        list_insert(&cache->partial_slabs, &slab->list_node);
    } else if (!list_singular(&cache->partial_slabs)) {
        struct list *node;
        struct mem_slab *tmp;

        /*
         * The slab remains partial. If there are more than one partial slabs,
         * maintain the list sorted.
         */

        assert(slab->nr_refs > 0);

        for (node = list_next(&slab->list_node);
             !list_end(&cache->partial_slabs, node);
             node = list_next(node)) {
            tmp = list_entry(node, struct mem_slab, list_node);

            if (tmp->nr_refs <= slab->nr_refs)
                break;
        }

        /*
         * If the direct neighbor was found, the list is already sorted.
         * If no slab was found, the slab is inserted at the tail of the list.
         */
        if (node != list_next(&slab->list_node)) {
            list_remove(&slab->list_node);
            list_insert_before(node, &slab->list_node);
        }
    }
}

static void
mem_cache_alloc_verify(struct mem_cache *cache, void *buf, int construct)
{
    struct mem_buftag *buftag;
    union mem_bufctl *bufctl;
    void *addr;

    buftag = mem_buf_to_buftag(buf, cache);

    if (buftag->state != MEM_BUFTAG_FREE)
        mem_cache_error(cache, buf, MEM_ERR_BUFTAG, buftag);

    addr = mem_buf_verify_fill(buf, MEM_FREE_PATTERN, MEM_UNINIT_PATTERN,
                               cache->bufctl_dist);

    if (addr != NULL)
        mem_cache_error(cache, buf, MEM_ERR_MODIFIED, addr);

    addr = buf + cache->obj_size;
    memset(addr, MEM_REDZONE_BYTE, cache->redzone_pad);

    bufctl = mem_buf_to_bufctl(buf, cache);
    bufctl->redzone = MEM_REDZONE_WORD;
    buftag->state = MEM_BUFTAG_ALLOC;

    if (construct && (cache->ctor != NULL))
        cache->ctor(buf);
}

void *
mem_cache_alloc(struct mem_cache *cache)
{
    struct mem_cpu_pool *cpu_pool;
    int filled;
    void *buf;

    cpu_pool = mem_cpu_pool_get(cache);

    pthread_mutex_lock(&cpu_pool->lock);

fast_alloc_retry:
    if (likely(cpu_pool->nr_objs > 0)) {
        buf = mem_cpu_pool_pop(cpu_pool);
        pthread_mutex_unlock(&cpu_pool->lock);

        if (cpu_pool->flags & MEM_CF_VERIFY)
            mem_cache_alloc_verify(cache, buf, MEM_AV_CONSTRUCT);

        return buf;
    }

    if (cpu_pool->array != NULL) {
        filled = mem_cpu_pool_fill(cpu_pool, cache);

        if (!filled) {
            pthread_mutex_unlock(&cpu_pool->lock);

            filled = mem_cache_grow(cache);

            if (!filled)
                return NULL;

            pthread_mutex_lock(&cpu_pool->lock);
        }

        goto fast_alloc_retry;
    }

    pthread_mutex_unlock(&cpu_pool->lock);

slow_alloc_retry:
    pthread_mutex_lock(&cache->lock);
    buf = mem_cache_alloc_from_slab(cache);
    pthread_mutex_unlock(&cache->lock);

    if (buf == NULL) {
        filled = mem_cache_grow(cache);

        if (!filled)
            return NULL;

        goto slow_alloc_retry;
    }

    if (cache->flags & MEM_CF_VERIFY)
        mem_cache_alloc_verify(cache, buf, MEM_AV_NOCONSTRUCT);

    if (cache->ctor != NULL)
        cache->ctor(buf);

    return buf;
}

static void
mem_cache_free_verify(struct mem_cache *cache, void *buf)
{
    struct avltree_node *node;
    struct mem_buftag *buftag;
    struct mem_slab *slab;
    union mem_bufctl *bufctl;
    unsigned char *redzone_byte;
    unsigned long slabend;

    pthread_mutex_lock(&cache->lock);
    node = avltree_lookup_nearest(&cache->active_slabs, buf,
                                  mem_slab_cmp_lookup, AVLTREE_LEFT);
    pthread_mutex_unlock(&cache->lock);

    if (node == NULL)
        mem_cache_error(cache, buf, MEM_ERR_INVALID, NULL);

    slab = avltree_entry(node, struct mem_slab, tree_node);
    slabend = P2ALIGN((unsigned long)slab->addr + cache->slab_size, PAGE_SIZE);

    if ((unsigned long)buf >= slabend)
        mem_cache_error(cache, buf, MEM_ERR_INVALID, NULL);

    if ((((unsigned long)buf - (unsigned long)slab->addr) % cache->buf_size)
        != 0)
        mem_cache_error(cache, buf, MEM_ERR_INVALID, NULL);

    /*
     * As the buffer address is valid, accessing its buftag is safe.
     */
    buftag = mem_buf_to_buftag(buf, cache);

    if (buftag->state != MEM_BUFTAG_ALLOC) {
        if (buftag->state == MEM_BUFTAG_FREE)
            mem_cache_error(cache, buf, MEM_ERR_DOUBLEFREE, NULL);
        else
            mem_cache_error(cache, buf, MEM_ERR_BUFTAG, buftag);
    }

    redzone_byte = buf + cache->obj_size;
    bufctl = mem_buf_to_bufctl(buf, cache);

    while (redzone_byte < (unsigned char *)bufctl) {
        if (*redzone_byte != MEM_REDZONE_BYTE)
            mem_cache_error(cache, buf, MEM_ERR_REDZONE, redzone_byte);

        redzone_byte++;
    }

    if (bufctl->redzone != MEM_REDZONE_WORD) {
        unsigned long word;

        word = MEM_REDZONE_WORD;
        redzone_byte = mem_buf_verify_bytes(&bufctl->redzone, &word,
                                            sizeof(bufctl->redzone));
        mem_cache_error(cache, buf, MEM_ERR_REDZONE, redzone_byte);
    }

    mem_buf_fill(buf, MEM_FREE_PATTERN, cache->bufctl_dist);
    buftag->state = MEM_BUFTAG_FREE;
}

void
mem_cache_free(struct mem_cache *cache, void *obj)
{
    struct mem_cpu_pool *cpu_pool;
    void **array;

    cpu_pool = mem_cpu_pool_get(cache);

    if (cpu_pool->flags & MEM_CF_VERIFY)
        mem_cache_free_verify(cache, obj);

    pthread_mutex_lock(&cpu_pool->lock);

fast_free_retry:
    if (likely(cpu_pool->nr_objs < cpu_pool->size)) {
        mem_cpu_pool_push(cpu_pool, obj);
        pthread_mutex_unlock(&cpu_pool->lock);
        return;
    }

    if (cpu_pool->array != NULL) {
        mem_cpu_pool_drain(cpu_pool, cache);
        goto fast_free_retry;
    }

    pthread_mutex_unlock(&cpu_pool->lock);

    array = mem_cache_alloc(cache->cpu_pool_type->array_cache);

    if (array != NULL) {
        pthread_mutex_lock(&cpu_pool->lock);

        /*
         * Another thread may have built the CPU pool while the lock was
         * dropped.
         */
        if (cpu_pool->array != NULL) {
            pthread_mutex_unlock(&cpu_pool->lock);
            mem_cache_free(cache->cpu_pool_type->array_cache, array);
            goto fast_free_retry;
        }

        mem_cpu_pool_build(cpu_pool, cache, array);
        goto fast_free_retry;
    }

    mem_cache_free_to_slab(cache, obj);
}

void
mem_cache_info(struct mem_cache *cache)
{
    struct mem_cache *cache_stats;
    char flags_str[64];

    if (cache == NULL) {
        pthread_mutex_lock(&mem_cache_list_lock);

        list_for_each_entry(&mem_cache_list, cache, node)
            mem_cache_info(cache);

        pthread_mutex_unlock(&mem_cache_list_lock);

        return;
    }

    cache_stats = mem_alloc(sizeof(*cache_stats));

    if (cache_stats == NULL) {
        mem_warn("unable to allocate memory for cache stats");
        return;
    }

    pthread_mutex_lock(&cache->lock);
    cache_stats->flags = cache->flags;
    cache_stats->obj_size = cache->obj_size;
    cache_stats->align = cache->align;
    cache_stats->buf_size = cache->buf_size;
    cache_stats->bufctl_dist = cache->bufctl_dist;
    cache_stats->slab_size = cache->slab_size;
    cache_stats->color_max = cache->color_max;
    cache_stats->bufs_per_slab = cache->bufs_per_slab;
    cache_stats->nr_objs = cache->nr_objs;
    cache_stats->nr_bufs = cache->nr_bufs;
    cache_stats->nr_slabs = cache->nr_slabs;
    cache_stats->nr_free_slabs = cache->nr_free_slabs;
    strcpy(cache_stats->name, cache->name);
    cache_stats->buftag_dist = cache->buftag_dist;
    cache_stats->redzone_pad = cache->redzone_pad;
    cache_stats->cpu_pool_type = cache->cpu_pool_type;
    pthread_mutex_unlock(&cache->lock);

    snprintf(flags_str, sizeof(flags_str), "%s%s%s",
        (cache_stats->flags & MEM_CF_DIRECT) ? " DIRECT" : "",
        (cache_stats->flags & MEM_CF_SLAB_EXTERNAL) ? " SLAB_EXTERNAL" : "",
        (cache_stats->flags & MEM_CF_VERIFY) ? " VERIFY" : "");

    mem_print("name: %s", cache_stats->name);
    mem_print("flags: 0x%x%s", cache_stats->flags, flags_str);
    mem_print("obj_size: %zu", cache_stats->obj_size);
    mem_print("align: %zu", cache_stats->align);
    mem_print("buf_size: %zu", cache_stats->buf_size);
    mem_print("bufctl_dist: %zu", cache_stats->bufctl_dist);
    mem_print("slab_size: %zu", cache_stats->slab_size);
    mem_print("color_max: %zu", cache_stats->color_max);
    mem_print("bufs_per_slab: %lu", cache_stats->bufs_per_slab);
    mem_print("nr_objs: %lu", cache_stats->nr_objs);
    mem_print("nr_bufs: %lu", cache_stats->nr_bufs);
    mem_print("nr_slabs: %lu", cache_stats->nr_slabs);
    mem_print("nr_free_slabs: %lu", cache_stats->nr_free_slabs);
    mem_print("buftag_dist: %zu", cache_stats->buftag_dist);
    mem_print("redzone_pad: %zu", cache_stats->redzone_pad);
    mem_print("cpu_pool_size: %d", cache_stats->cpu_pool_type->array_size);
    mem_print("--");

    mem_free(cache_stats, sizeof(*cache_stats));
}

static void *
mem_gc(void *arg)
{
    struct mem_cache *cache;
    struct timespec ts;
    int error;

    (void)arg;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    for (;;) {
        ts.tv_sec += MEM_GC_INTERVAL;

        do
            error = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
        while (error == EINTR);

        /*
         * EINTR is the only expected error.
         */
        assert(error == 0);

#if 0
        mem_info();

#ifdef CONFIG_MEM_USE_PHYS
        phys_info();
#endif /* CONFIG_MEM_USE_PHYS */
#endif

        pthread_mutex_lock(&mem_cache_list_lock);

        list_for_each_entry(&mem_cache_list, cache, node)
            mem_cache_reap(cache);

        pthread_mutex_unlock(&mem_cache_list_lock);
    }

    return NULL;
}

void
mem_setup(void)
{
    static int mem_initialized = 0;
    struct mem_cpu_pool_type *cpu_pool_type;
    char name[MEM_NAME_SIZE];
    pthread_t thread;
    size_t i, size;
    int error;

    if (mem_initialized)
        return;

    mem_initialized = 1;

    _pagesize = sysconf(_SC_PAGESIZE);
    assert(ISP2(_pagesize));

    /*
     * Make sure a bufctl can always be stored in a buffer.
     */
    assert(sizeof(union mem_bufctl) <= MEM_ALIGN_MIN);

#ifdef CONFIG_MEM_USE_PHYS
    phys_setup();
#endif /* CONFIG_MEM_USE_PHYS */

    list_init(&mem_cache_list);
    pthread_mutex_init(&mem_cache_list_lock, NULL);

    for (i = 0; i < ARRAY_SIZE(mem_cpu_pool_types); i++) {
        cpu_pool_type = &mem_cpu_pool_types[i];
        cpu_pool_type->array_cache = &mem_cpu_array_caches[i];
        sprintf(name, "mem_cpu_array_%d", cpu_pool_type->array_size);
        size = sizeof(void *) * cpu_pool_type->array_size;
        mem_cache_init(cpu_pool_type->array_cache, name, size,
                       cpu_pool_type->array_align, NULL, NULL, 0);
    }

    /*
     * Prevent off slab data for the slab cache to avoid infinite recursion.
     */
    mem_cache_init(&mem_slab_cache, "mem_slab", sizeof(struct mem_slab),
                   0, NULL, NULL, MEM_CREATE_INTERNAL);
    mem_cache_init(&mem_cache_cache, "mem_cache", sizeof(struct mem_cache),
                   CPU_L1_SIZE, NULL, NULL, 0);

    size = 1 << MEM_CACHES_FIRST_SHIFT;

    for (i = 0; i < ARRAY_SIZE(mem_caches); i++) {
        sprintf(name, "mem_%zu", size);
        mem_cache_init(&mem_caches[i], name, size, 0, NULL, NULL, 0);
        size <<= 1;
    }

    error = pthread_create(&thread, NULL, mem_gc, NULL);

    if (error)
        mem_error("unable to create garbage collection thread: %s",
                  strerror(error));
}

/*
 * Return the mem cache index matching the given allocation size, which
 * must be strictly greater than 0.
 */
static inline size_t
mem_get_index(size_t size)
{
    assert(size != 0);

    size = (size - 1) >> MEM_CACHES_FIRST_SHIFT;

    if (size == 0)
        return 0;
    else
        return (sizeof(long) * CHAR_BIT) - __builtin_clzl(size);
}

static void
mem_alloc_verify(struct mem_cache *cache, void *buf, size_t size)
{
    size_t redzone_size;
    void *redzone;

    assert(size <= cache->obj_size);

    redzone = buf + size;
    redzone_size = cache->obj_size - size;
    memset(redzone, MEM_REDZONE_BYTE, redzone_size);
}

void *
mem_alloc(size_t size)
{
    size_t index;
    void *buf;

    if (size == 0)
        return NULL;

    index = mem_get_index(size);

    if (index < ARRAY_SIZE(mem_caches)) {
        struct mem_cache *cache;

        cache = &mem_caches[index];
        buf = mem_cache_alloc(cache);

        if ((buf != NULL) && (cache->flags & MEM_CF_VERIFY))
            mem_alloc_verify(cache, buf, size);
    } else {
        buf = mem_default_alloc(size);
    }

  return buf;
}

void *
mem_zalloc(size_t size)
{
    void *ptr;

    ptr = mem_alloc(size);

    if (ptr == NULL)
        return NULL;

    memset(ptr, 0, size);
    return ptr;
}

static void
mem_free_verify(struct mem_cache *cache, void *buf, size_t size)
{
    unsigned char *redzone_byte, *redzone_end;

    assert(size <= cache->obj_size);

    redzone_byte = buf + size;
    redzone_end = buf + cache->obj_size;

    while (redzone_byte < redzone_end) {
        if (*redzone_byte != MEM_REDZONE_BYTE)
            mem_cache_error(cache, buf, MEM_ERR_REDZONE, redzone_byte);

        redzone_byte++;
    }
}

void
mem_free(void *ptr, size_t size)
{
    size_t index;

    if ((ptr == NULL) || (size == 0))
        return;

    index = mem_get_index(size);

    if (index < ARRAY_SIZE(mem_caches)) {
        struct mem_cache *cache;

        cache = &mem_caches[index];

        if (cache->flags & MEM_CF_VERIFY)
            mem_free_verify(cache, ptr, size);

        mem_cache_free(cache, ptr);
    } else {
        mem_default_free(ptr, size);
    }
}

void
mem_info(void)
{
    struct mem_cache *cache, *cache_stats;
    size_t mem_usage, mem_reclaimable;

    cache_stats = mem_alloc(sizeof(*cache_stats));

    if (cache_stats == NULL) {
        mem_warn("unable to allocate memory for cache stats");
        return;
    }

    mem_print("-- cache                       obj slab  bufs   objs   bufs "
              "   total reclaimable");
    mem_print("-- name                       size size /slab  usage  count "
              "  memory      memory");

    pthread_mutex_lock(&mem_cache_list_lock);

    list_for_each_entry(&mem_cache_list, cache, node) {
        pthread_mutex_lock(&cache->lock);
        cache_stats->obj_size = cache->obj_size;
        cache_stats->slab_size = cache->slab_size;
        cache_stats->bufs_per_slab = cache->bufs_per_slab;
        cache_stats->nr_objs = cache->nr_objs;
        cache_stats->nr_bufs = cache->nr_bufs;
        cache_stats->nr_slabs = cache->nr_slabs;
        cache_stats->nr_free_slabs = cache->nr_free_slabs;
        strcpy(cache_stats->name, cache->name);
        pthread_mutex_unlock(&cache->lock);

        mem_usage = (cache_stats->nr_slabs * cache_stats->slab_size) >> 10;
        mem_reclaimable =
            (cache_stats->nr_free_slabs * cache_stats->slab_size) >> 10;

        mem_print("%-27s %6zu %3zuk  %4lu %6lu %6lu %7zuk %10zuk",
                  cache_stats->name, cache_stats->obj_size,
                  cache_stats->slab_size >> 10, cache_stats->bufs_per_slab,
                  cache_stats->nr_objs, cache_stats->nr_bufs, mem_usage,
                  mem_reclaimable);
    }

    pthread_mutex_unlock(&mem_cache_list_lock);

    mem_free(cache_stats, sizeof(*cache_stats));
}
