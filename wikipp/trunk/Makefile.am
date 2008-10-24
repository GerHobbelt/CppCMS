SUBDIRS = ./templates

sbin_PROGRAMS = wikipp
wikipp_SOURCES = main.cpp data.cpp
wikipp_LDFLAGS = -export-dynamic @CMSLIBS@
wikipp_CXXFLAGS =-Wall

noinst_HEADERS = data.h

sysconf_DATA = config.txt mysql.sql


