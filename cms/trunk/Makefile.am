SUBDIRS = ./cxxmarkdown templates

sbin_PROGRAMS = cppblog
cppblog_SOURCES = main.cpp blog.cpp views.cpp md5.c
cppblog_LDADD = cxxmarkdown/libmarkdowncxx.a
cppblog_CXXFLAGS =-Wall

noinst_HEADERS = blog.h data.h views.h error.h

sysconf_DATA = config.txt

build_static:
	g++ -g -Wall -O2 md5.c  main.cpp views.cpp blog.cpp cxxmarkdown/libmarkdowncxx.a /usr/lib/libdbixx.a /usr/lib/libtmpl.a /usr/lib/libtranstext.a /usr/lib/libcppcms.a -lfcgi++ -lcgicc -lboost_iostreams -lboost_regex -lboost_signals -ldbi -o cppblog

