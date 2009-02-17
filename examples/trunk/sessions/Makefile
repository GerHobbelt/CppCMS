all: clean hello.fcgi 

hello.fcgi: hello.cpp view.cpp data.h
	$(CXX) -Wall hello.cpp view.cpp -o hello.fcgi -lcppcms

view.cpp: view.tmpl data.h
	cppcms_tmpl_cc view.tmpl -o view.cpp

clean:
	rm -fr *.fcgi *.exe *.so view.cpp cppcms_rundir
