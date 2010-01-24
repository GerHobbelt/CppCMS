ifeq ($(host),cygwin)
   LIBS=-lcppcms -lcppcms_boost -lgcrypt -lz -licuuc -licudata -licui18n -lws2_32 -lwsock32 -liconv
else
   LIBS=-lcppcms
endif

all: clean hello.fcgi 

hello.fcgi: hello.cpp view.cpp content.h
	$(CXX) -Wall hello.cpp view.cpp -o hello.fcgi $(LIBS)

view.cpp: view.tmpl content.h
	cppcms_tmpl_cc view.tmpl -o view.cpp

clean:
	rm -fr *.fcgi *.exe *.so view.cpp cppcms_rundir
