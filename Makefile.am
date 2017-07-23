EXTRA_DIST =
MOSTLYCLEANFILES =

# TODO Remove tabs
AM_CPPFLAGS = \
        -pipe \
        -std=gnu11 \
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
        bitmap.c \
        bitmap.h \
        bitmap_i.h \
        cbuf.c \
        cbuf.h \
        check.h \
        cpu.h \
        error.c \
        error.h \
        fmt.c \
        fmt.h \
        hash.h \
        list.c \
        list.h \
        macros.h \
        plist.c \
        plist.h \
        rbtree.c \
        rbtree.h \
        rbtree_i.h \
        rdxtree.c \
        rdxtree.h \
        rdxtree_i.h \
        slist.h \
        shell.c \
        shell.h \
        x86/atomic.h \
        x86/mb.h

librbraun_la_LIBADD = -lrt -lpthread

bin_PROGRAMS = \
        test_avltree \
        test_cbuf \
        test_fmt_sprintf \
        test_fmt_sscanf \
        test_plist \
        test_slist \
        test_rbtree \
        test_rdxtree \
        test_shell

test_avltree_SOURCES = test/test_avltree.c
test_avltree_LDADD = librbraun.la

test_cbuf_SOURCES = test/test_cbuf.c
test_cbuf_LDADD = librbraun.la

test_fmt_sprintf_SOURCES = test/test_fmt_sprintf.c
test_fmt_sprintf_LDADD = librbraun.la

test_fmt_sscanf_SOURCES = test/test_fmt_sscanf.c
test_fmt_sscanf_LDADD = librbraun.la

test_plist_SOURCES = test/test_plist.c
test_plist_LDADD = librbraun.la

test_slist_SOURCES = test/test_slist.c
test_slist_LDADD = librbraun.la

test_rbtree_SOURCES = test/test_rbtree.c
test_rbtree_LDADD = librbraun.la

test_rdxtree_SOURCES = test/test_rdxtree.c
test_rdxtree_LDADD = -lrt -lpthread

test_shell_SOURCES = test/test_shell.c
test_shell_LDADD = librbraun.la
