all: hello.fcgi

hello.fcgi: hello.cpp
	g++ -O2 -Wall -g hello.cpp -o hello.fcgi -lcppcms

clean:
	rm -f hello.fcgi hello.fcgi.exe
