all: hello.fcgi

hello.fcgi: hello.cpp
	$(CXX) -O2 -Wall -g hello.cpp -o hello.fcgi -lcppcms

clean:
	rm -fr hello.fcgi hello.fcgi.exe cppcms_rundir
