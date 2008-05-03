SUBDIRS = cxxmarkdown templates

sbin_PROGRAMS = cppblog
cppblog_SOURCES = main.cpp blog.cpp views.cpp md5.c
cppblog_LDADD = cxxmarkdown/libmarkdowncxx.a
cppblog_CXXFLAGS =-Wall

noinst_HEADERS = blog.h data.h views.h error.h

sysconf_DATA = config.txt


