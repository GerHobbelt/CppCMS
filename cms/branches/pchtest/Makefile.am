SUBDIRS = cxxmarkdown templates

sbin_PROGRAMS = cppblog
cppblog_SOURCES = main.cpp blog.cpp views.cpp md5.c
cppblog_LDADD = cxxmarkdown/libmarkdowncxx.a
cppblog_CXXFLAGS =-Winvalid-pch

noinst_HEADERS = blog.h data.h views.h error.h

sysconf_DATA = config.txt

if USE_PCH

cppblog_SOURCES +=  pch.h.gch

PCH_SOURCE=pch.h
PCH_FILE=$(PCH_SOURCE).gch

sinclude ./pch.h.gch.dep

$(cppblog_OBJECTS): $(PCH_FILE)

$(PCH_FILE):
	$(CXXCOMPILE) $(PCH_SOURCE) -o $(PCH_FILE) \
		-MT $(PCH_FILE) -MD -MP -MF pch.h.gch.dep

endif

clean-local:
	rm -f ./pch.h.gch.dep ./pch.h.gch

