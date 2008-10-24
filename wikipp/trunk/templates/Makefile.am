pkgdata_DATA = style.css 
nobase_lib_LTLIBRARIES = libview.ls

libaview_la_SOURCES = view.cpp

libview_la_LDFLAGS = -no-undefined

TEMPLATES= main.tmpl

view.cpp: ${TEMPLATES} 
	cppcms_tmpl_cc -o view.cpp -d wikipp -n view ${TEMPLATES}

clean-local:
	rm -f *.cpp
