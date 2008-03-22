noinst_PROGRAMS = test
test_SOURCES = test.cpp 
test_LDADD = libtmpl.la

bin_PROGRAMS	= tmpl_as
tmpl_as_SOURCES	= assemly.cpp

lib_LTLIBRARIES     = libtmpl.la
libtmpl_la_SOURCES = renderer.cpp content.cpp

nobase_pkginclude_HEADERS = content.h renderer.h
bin_SCRIPTS = tmpl_cc tmpl_bld
