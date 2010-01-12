# For Cygwin (static only build)

#LIBS=-lcppcms -lcppcms_boost -lgcrypt -lz -licuuc -licudata -licui18n -lws2_32 -lwsock32

# For other dynamic library builds

LIBS=-lcppcms

all: clean hello-tmpl

hello-tmpl: hello-tmpl.cpp view.cpp data.h
	$(CXX) -Wall hello-tmpl.cpp view.cpp -o hello-tmpl.fcgi ${LIBS}

view.cpp: first.tmpl data.h
	cppcms_tmpl_cc first.tmpl -o view.cpp

clean:
	rm -fr *.fcgi *.exe *.so view.cpp cppcms_rundir

