SUBDIRS = templates cxxmarkdown po utf8

sbin_PROGRAMS = wikipp
wikipp_SOURCES = index.cpp  main.cpp  master.cpp  options.cpp  page.cpp  users.cpp  wiki.cpp
wikipp_LDFLAGS = -export-dynamic @CMSLIBS@
wikipp_LDADD = cxxmarkdown/libmarkdowncxx.a
wikipp_CXXFLAGS =-Wall

noinst_HEADERS = content.h diff.h index.h index_content.h master.h \
		 master_content.h options.h options_content.h page.h \
		 page_content.h users.h users_content.h wiki.h index_content.h 

pkgsysconfdir=$(sysconfdir)/wikipp
dist_pkgdata_DATA = mysql.sql sqlite3.sql
dist_pkgsysconf_DATA = config.txt

build_static:
	g++ -g -Wall -O2 *.o templates/*.o \
		cxxmarkdown/libmarkdowncxx.a \
		/usr/lib/libdbixx.a /opt/cppcms_0/lib/libcppcms.a /opt/cppcms_0/lib/libcppcmstranstext.a \
		-lfcgi++ -lcgicc -lboost_system -lboost_iostreams -lboost_regex -lboost_signals -ldbi -lmm -lgcrypt -lsqlite3 -lrt -o wikipp


ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath m4/ChangeLog \
	config.txt mysql.sql sqlite3.sql
