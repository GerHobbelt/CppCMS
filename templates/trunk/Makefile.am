SUBDIRS = ./transtext

noinst_PROGRAMS = test test_f
test_SOURCES = test.cpp 
test_LDADD = libtmpl.la 
test_f_SOURCES = test_f.cpp
test_f_LDADD = libtmpl.la transtext/libtranstext.a

bin_PROGRAMS	= tmpl_as
tmpl_as_SOURCES	= assemly.cpp

lib_LTLIBRARIES     = libtmpl.la
libtmpl_la_SOURCES = renderer.cpp content.cpp
libtmpl_la_LIBADD = transtext/libtranstext.a

nobase_pkginclude_HEADERS = content.h renderer.h
bin_SCRIPTS = tmpl_cc tmpl_bld
