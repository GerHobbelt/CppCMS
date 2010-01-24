ifeq ($(host),cygwin)
   LIBS=-lcppcms -lcppcms_boost -lgcrypt -lz -licuuc -licudata -licui18n -lws2_32 -lwsock32 -liconv
else
   LIBS=-lcppcms
endif


all: hello

hello: hello.cpp
	$(CXX) -O2 -Wall -g hello.cpp -o hello ${LIBS}

clean:
	rm -fr hello hello.exe cppcms_rundir
