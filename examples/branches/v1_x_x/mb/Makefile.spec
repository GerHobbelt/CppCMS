ifeq ($(host),cygwin)
   LIBS=-lcppcms -lcppcms_boost -lgcrypt -lz -licuuc -licudata -licui18n -lws2_32 -lwsock32 -liconv
else
   LIBS=-lcppcms
endif

CXXFLAGS=-Wall -g -O0 -I../inc

EXEC_CXXFLAGS=$(CXXFLAGS)
EXEC_LDFLAGS=-export-dynamic
EXEC_LIBS=$(LIBS) -ldbixx 

VIEW_CXXFLAGS=$(CXXFLAGS) -fPIC -DPIC
VIEW_LDFLAGS=-shared
VIEW_LIBS=$(LIBS)

CXX=g++
CTMPL=cppcms_tmpl_cc
GETTEXT_DOMAIN=mb

# Linux
SO_EXT=so
EXEC_EXT=

# cygwin
# SO_EXT=dll
# EXEC_EXT=.exe


