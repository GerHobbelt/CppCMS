SUBDIRS = ./cxxmarkdown templates

sbin_PROGRAMS = cppblog
cppblog_SOURCES = main.cpp blog.cpp views.cpp md5.c trackback.cpp 
cppblog_LDADD = cxxmarkdown/libmarkdowncxx.a templates/libadmin_view.a
cppblog_CXXFLAGS =-Wall

noinst_HEADERS = blog.h data.h views.h error.h

sysconf_DATA = config.txt

build_static:
	g++ -g -Wall -O2 md5.c trackback.cpp main.cpp views.cpp blog.cpp cxxmarkdown/libmarkdowncxx.a /usr/lib/libdbixx.a /usr/lib/libtmpl.a /usr/lib/libtranstext.a /usr/lib/libcppcms.a -lfcgi++ -lcgicc -lboost_iostreams -lboost_regex -lboost_signals -ldbi -lcurl -lmm -o cppblog

all: locale/he/LC_MESSAGES/cppblog.mo

locale/he/LC_MESSAGES/cppblog.mo: locale/he/LC_MESSAGES/cppblog.po
	msgfmt locale/he/LC_MESSAGES/cppblog.po -o locale/he/LC_MESSAGES/cppblog.mo

update-pot:
	msgcat templates/messages-os.pot templates/messages.pot >messages.pot
	msgmerge -U locale/he/LC_MESSAGES/cppblog.po messages.pot

