SUBDIRS = cxxmarkdown

sbin_PROGRAMS = cppblog
cppblog_SOURCES = main.cpp blog.cpp views.cpp
cppblog_LDADD = cxxmarkdown/libmarkdowncxx.a
cppblog_config_CXXFLAGS = -DCONFIG_ONLY

noinst_HEADERS = blog.h data.h views.h error.h

pkgdata_DATA = templates/look.opcode templates/style.css templates/adminstyle.css


sysconf_DATA = config.txt

templates/look.opcode: templates/*.html
	cd templates; ./build.sh

clean-local:
	rm -f templates/look.opcode

