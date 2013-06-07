EXTRA_DIST =
MOSTLYCLEANFILES =

AM_CPPFLAGS = \
	-pipe \
	-std=gnu99 \
	-imacros config.h \
	-I$(top_srcdir)

AM_CFLAGS = \
	-Wall \
	-Wextra \
	-Wmissing-prototypes

lib_LTLIBRARIES = librbraun.la

librbraun_la_SOURCES = \
	avltree.c \
	avltree.h \
	avltree_i.h \
	cpu.h \
	error.c \
	error.h \
	hash.h \
	list.c \
	list.h \
	macros.h \
	mem.c \
	mem.h \
	mem_malloc.c \
	mem_malloc.h \
	phys.c \
	phys.h \
	rbtree.c \
	rbtree.h \
	rbtree_i.h \
	rdxtree.c \
	rdxtree.h \
	xprintf.c \
	xprintf.h
librbraun_la_LIBADD = -lrt -lpthread

bin_PROGRAMS = \
	test_avltree \
	test_mem \
	test_mem_cache \
	test_mem_cache_invalid_free \
	test_mem_cache_double_free \
	test_mem_cache_write_free \
	test_mem_cache_write_beyond \
	test_mem_cache_write_buftag \
	test_mem_offbyone \
	test_phys \
	test_rbtree \
	test_rdxtree \
	test_xprintf

test_avltree_SOURCES = test/test_avltree.c
test_avltree_LDADD = librbraun.la

test_mem_SOURCES = test/test_mem.c
test_mem_LDADD = librbraun.la

test_mem_cache_SOURCES = test/test_mem_cache.c
test_mem_cache_LDADD = -lrt -lpthread

test_mem_cache_invalid_free_SOURCES = test/test_mem_cache_invalid_free.c
test_mem_cache_invalid_free_LDADD = librbraun.la

test_mem_cache_double_free_SOURCES = test/test_mem_cache_double_free.c
test_mem_cache_double_free_LDADD = librbraun.la

test_mem_cache_write_free_SOURCES = test/test_mem_cache_write_free.c
test_mem_cache_write_free_LDADD = librbraun.la

test_mem_cache_write_beyond_SOURCES = test/test_mem_cache_write_beyond.c
test_mem_cache_write_beyond_LDADD = librbraun.la

test_mem_cache_write_buftag_SOURCES = test/test_mem_cache_write_buftag.c
test_mem_cache_write_buftag_LDADD = librbraun.la

test_mem_offbyone_SOURCES = test/test_mem_offbyone.c
test_mem_offbyone_LDADD = librbraun.la

test_phys_SOURCES = test/test_phys.c
test_phys_LDADD = -lrt -lpthread

test_rbtree_SOURCES = test/test_rbtree.c
test_rbtree_LDADD = librbraun.la

test_rdxtree_SOURCES = test/test_rdxtree.c
test_rdxtree_LDADD = -lrt -lpthread

test_xprintf_SOURCES = test/test_xprintf.c
test_xprintf_CFLAGS = $(AM_CFLAGS) -Wno-format
test_xprintf_LDADD = librbraun.la
