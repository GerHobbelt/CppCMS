ifeq ($(host),cygwin)
   LIBS=-lcppcms -lcppcms_boost -lgcrypt -lz -licuuc -licudata -licui18n -lws2_32 -lwsock32 -liconv
else
   LIBS=-lcppcms
endif

all: clean hello-tmpl

hello-tmpl: hello-tmpl.cpp view.cpp content.h
	$(CXX) -Wall hello-tmpl.cpp view.cpp -o hello-tmpl.fcgi ${LIBS}

view.cpp: first.tmpl content.h
	cppcms_tmpl_cc first.tmpl -o view.cpp

clean:
	rm -fr *.fcgi *.exe *.so view.cpp cppcms_rundir

