SUBDIRS = ./transtext

noinst_PROGRAMS = test test_f
test_SOURCES = test.cpp 
test_LDADD = libtmpl.la
test_CXXFLAGS = -Itranstext -Wall
test_f_SOURCES = test_f.cpp
test_f_LDADD = libtmpl.la
test_f_CXXFLAGS = -Itranstext -Wall

bin_PROGRAMS	= tmpl_as
tmpl_as_SOURCES	= assemly.cpp
tmpl_as_CXXFLAGS = -Itranstext -Wall

lib_LTLIBRARIES     = libtmpl.la
libtmpl_la_SOURCES = renderer.cpp content.cpp
libtmpl_la_LIBADD = transtext/libtranstext.la
libtmpl_la_CXXFLAGS = -Itranstext -Wall
libtmpl_la_LDFLAGS  = -version-info 0:0:0

nobase_pkginclude_HEADERS = content.h renderer.h
dist_bin_SCRIPTS = tmpl_cc tmpl_bld
