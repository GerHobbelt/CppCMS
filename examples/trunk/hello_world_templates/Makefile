all: hello-tmpl.fcgi

hello-tmpl.fcgi: hello-tmpl.cpp view.cpp data.h
	g++ -Wall hello-tmpl.cpp view.cpp -o hello-tmpl.fcgi -lcppcms -ltmpl

view.cpp: first.tmpl data.h
	cppcms_tmpl_cc first.tmpl -o view.cpp

clean:
	rm -f *.fcgi *.exe *.so view.cpp
