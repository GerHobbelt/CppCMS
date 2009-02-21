SUBDIRS = ./cxxmarkdown ./templates

sbin_PROGRAMS = cppblog
cppblog_SOURCES = main.cpp blog.cpp views.cpp md5.c trackback.cpp 
cppblog_LDFLAGS = -export-dynamic @CMSLIBS@
cppblog_LDADD = cxxmarkdown/libmarkdowncxx.a
cppblog_CXXFLAGS =-Wall

noinst_HEADERS = blog.h data.h views.h error.h

sysconf_DATA = config.txt

build_static:
	g++ -g -Wall -O2 *.o\
		cxxmarkdown/libmarkdowncxx.a \
		-Wl,--whole-archive \
		templates/.libs/libadmin_view.a templates/.libs/libcontendend.a templates/.libs/liborangesky.a \
		-Wl,--no-whole-archive \
		/usr/lib/libdbixx.a /usr/lib/libcppcms.a /usr/lib/libcppcmstranstext.a \
		-lfcgi++ -lcgicc -lboost_iostreams -lboost_regex -lboost_signals -ldbi -lcurl -lmm -lsqlite3 -lgcrypt -o cppblog

all: locale/he/LC_MESSAGES/cppblog.mo

locale/he/LC_MESSAGES/cppblog.mo: locale/he/LC_MESSAGES/cppblog.po
	msgfmt locale/he/LC_MESSAGES/cppblog.po -o locale/he/LC_MESSAGES/cppblog.mo

update-pot:
	msgcat templates/messages-os.pot templates/messages.pot >messages.pot
	msgmerge -U locale/he/LC_MESSAGES/cppblog.po messages.pot

