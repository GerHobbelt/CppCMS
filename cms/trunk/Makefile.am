sbin_PROGRAMS = cppblog cppblog_config
cppblog_SOURCES = main.cpp blog.cpp views.cpp templates/look.h

cppblog_config_SOURCES = data.cpp
cppblog_config_CXXFLAGS = -DCONFIG_ONLY

noinst_HEADERS = blog.h data.h views.h error.h

pkgdata_DATA = templates/look.template templates/style.css templates/adminstyle.css 


sysconf_DATA = config.txt

templates/look.h: templates/look.def
	cd templates; make

clean-local: 
	cd templates; make clean

