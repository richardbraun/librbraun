EXTRA_DIST =
MOSTLYCLEANFILES =

AM_CPPFLAGS = \
        -pipe \
        -std=gnu11 \
        -imacros config.h \
        -I$(top_srcdir)/src

AM_CFLAGS = \
        -Wall \
        -Wextra \
        -Wshadow \
        -Wstrict-prototypes \
        -Wmissing-prototypes

lib_LTLIBRARIES = librbraun.la

librbraun_la_SOURCES = \
        src/avltree.c \
        src/avltree.h \
        src/avltree_i.h \
        src/bitmap.c \
        src/bitmap.h \
        src/bitmap_i.h \
        src/cbuf.c \
        src/cbuf.h \
        src/check.h \
        src/cpu.h \
        src/fmt.c \
        src/fmt.h \
        src/hash.h \
        src/hlist.h \
        src/list.c \
        src/list.h \
        src/macros.h \
        src/mbuf.c \
        src/mbuf.h \
        src/plist.c \
        src/plist.h \
        src/rbtree.c \
        src/rbtree.h \
        src/rbtree_i.h \
        src/rdxtree.c \
        src/rdxtree.h \
        src/rdxtree_i.h \
        src/slist.h \
        src/shell.c \
        src/shell.h

librbraun_la_LIBADD = -lrt -lpthread

bin_PROGRAMS = \
        test_avltree \
        test_cbuf \
        test_fmt_sprintf \
        test_fmt_sscanf \
        test_hlist \
        test_mbuf \
        test_plist \
        test_rbtree \
        test_rdxtree \
        test_shell \
        test_slist

test_avltree_SOURCES = test/test_avltree.c
test_avltree_LDADD = librbraun.la

test_cbuf_SOURCES = test/test_cbuf.c
test_cbuf_LDADD = librbraun.la

test_fmt_sprintf_SOURCES = test/test_fmt_sprintf.c
test_fmt_sprintf_LDADD = librbraun.la

test_fmt_sscanf_SOURCES = test/test_fmt_sscanf.c
test_fmt_sscanf_LDADD = librbraun.la

test_hlist_SOURCES = test/test_hlist.c
test_hlist_LDADD = librbraun.la

test_mbuf_SOURCES = test/test_mbuf.c
test_mbuf_LDADD = librbraun.la

test_plist_SOURCES = test/test_plist.c
test_plist_LDADD = librbraun.la

test_rbtree_SOURCES = test/test_rbtree.c
test_rbtree_LDADD = librbraun.la

test_rdxtree_SOURCES = test/test_rdxtree.c
test_rdxtree_LDADD = -lrt -lpthread

test_shell_SOURCES = test/test_shell.c
test_shell_LDADD = librbraun.la

test_slist_SOURCES = test/test_slist.c
test_slist_LDADD = librbraun.la
