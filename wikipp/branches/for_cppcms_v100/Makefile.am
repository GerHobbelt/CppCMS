SUBDIRS = templates cxxmarkdown po utf8

sbin_PROGRAMS = wikipp
wikipp_SOURCES = index.cpp  main.cpp  master.cpp  options.cpp  page.cpp  users.cpp  wiki.cpp
wikipp_LDFLAGS = -export-dynamic @CMSLIBS@
wikipp_LDADD = cxxmarkdown/libmarkdowncxx.a
wikipp_CXXFLAGS =-Wall

noinst_HEADERS = data.h diff.h index.h index_data.h master.h \
		 master_data.h options.h options_data.h page.h \
		 page_data.h users.h users_data.h wiki.h index_data.h 

pkgsysconfdir=$(sysconfdir)/wikipp
dist_pkgdata_DATA = mysql.sql sqlite3.sql
dist_pkgsysconf_DATA = config.txt

build_static:
	g++ -g -Wall -O2 *.o templates/*.o \
		cxxmarkdown/libmarkdowncxx.a \
		/usr/lib/libdbixx.a /usr/lib/libcppcms.a /usr/lib/libcppcmstranstext.a \
		-lfcgi++ -lcgicc -lboost_system -lboost_iostreams -lboost_regex -lboost_signals -ldbi -lmm -lgcrypt -lsqlite3 -lrt -o wikipp


ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath m4/ChangeLog \
	config.txt mysql.sql sqlite3.sql
