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
	rbtree.c \
	rbtree.h \
	rbtree_i.h \
	rdxtree.c \
	rdxtree.h \
	rdxtree_i.h

librbraun_la_LIBADD = -lrt -lpthread

bin_PROGRAMS = \
	test_avltree \
	test_rbtree \
	test_rdxtree

test_avltree_SOURCES = test/test_avltree.c
test_avltree_LDADD = librbraun.la

test_rbtree_SOURCES = test/test_rbtree.c
test_rbtree_LDADD = librbraun.la

test_rdxtree_SOURCES = test/test_rdxtree.c
test_rdxtree_LDADD = -lrt -lpthread
