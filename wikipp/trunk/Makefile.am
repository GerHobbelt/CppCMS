SUBDIRS = po ./templates cxxmarkdown

sbin_PROGRAMS = wikipp
wikipp_SOURCES = main.cpp data.cpp wiki.cpp 
wikipp_LDFLAGS = -export-dynamic @CMSLIBS@
wikipp_LDADD = cxxmarkdown/libmarkdowncxx.a
wikipp_CXXFLAGS =-Wall

noinst_HEADERS = data.h wiki.h

sysconf_DATA = config.txt mysql.sql



ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath m4/ChangeLog
