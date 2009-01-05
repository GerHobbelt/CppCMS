CXXFLAGS=-Wall -g -O0 -I../inc

EXEC_CXXFLAGS=$(CXXFLAGS)
EXEC_LDFLAGS=-export-dynamic
EXEC_LIBS=-lcppcms -ldbixx -lcgicc

VIEW_CXXFLAGS=$(CXXFLAGS) -fPIC -DPIC
VIEW_LDFLAGS=-shared
VIEW_LIBS=-lcppcms

CXX=g++
CTMPL=cppcms_tmpl_cc
GETTEXT_DOMAIN=mb

# Linux
SO_EXT=so
EXEC_EXT=

# cygwin
# SO_EXT=dll
# EXEC_EXT=.exe


