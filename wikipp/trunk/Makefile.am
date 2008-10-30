SUBDIRS = templates cxxmarkdown po

sbin_PROGRAMS = wikipp
wikipp_SOURCES = data.cpp  index.cpp  main.cpp  master.cpp  options.cpp  page.cpp  users.cpp  wiki.cpp
wikipp_LDFLAGS = -export-dynamic @CMSLIBS@
wikipp_LDADD = cxxmarkdown/libmarkdowncxx.a
wikipp_CXXFLAGS =-Wall

noinst_HEADERS = data.h wiki.h

sysconf_DATA = config.txt mysql.sql

build_static:
	g++ -g -Wall -O2 *.o templates/*.o \
		cxxmarkdown/libmarkdowncxx.a \
		/usr/lib/libdbixx.a /usr/lib/libcppcms.a /usr/lib/libcppcmstranstext.a \
		-lfcgi++ -lcgicc -lboost_iostreams -lboost_regex -lboost_signals -ldbi -lmm -o wikipp


ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath m4/ChangeLog
