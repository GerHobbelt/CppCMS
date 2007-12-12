bin_PROGRAMS = cc_templ_d cc_templ_s ln_templ disass_templ

AM_CPPFLAGS = -I$(srcdir)/share

noinst_HEADERS = share/bytecode.h parser.h

cc_templ_d_SOURCES = parser.cpp dynamic.cpp
cc_templ_s_SOURCES = parser.cpp static.cpp
ln_templ_SOURCES = parser.cpp linker.cpp
disass_templ_SOURCES = disasemmble.c

