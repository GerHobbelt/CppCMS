all: clean hello-tmpl.fcgi

hello-tmpl.fcgi: hello-tmpl.cpp view.cpp data.h
	$(CXX) -Wall hello-tmpl.cpp view.cpp -o hello-tmpl.fcgi -lcppcms

view.cpp: first.tmpl data.h
	cppcms_tmpl_cc first.tmpl -o view.cpp

clean:
	rm -fr *.fcgi *.exe *.so view.cpp cppcms_rundir
