pkgdata_DATA = style.css 
nobase_lib_LTLIBRARIES = libview.la

libview_la_SOURCES = view.cpp
libview_la_LDFLAGS = -no-undefined

TEMPLATES= main.tmpl page.tmpl hist.tmpl

view.cpp: ${TEMPLATES} 
	cppcms_tmpl_cc -o view.cpp -d wikipp ${TEMPLATES}

clean-local:
	rm -f *.cpp
