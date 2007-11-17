TR=test.fcgi 
SRC=main.cpp main_thread.cpp

OBJ := $(patsubst %.cpp,%.o,$(SRC))

LIBS = -lmysqlclient -lfcgi++ -lcgicc -lcppcms -L../framework/
CXX = g++
CFLAGS = -Wall -g -O2 -I../framework/


all: $(TR)


$(TR) : $(OBJ) ../framework/libcppcms.a
	$(CXX) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

clean:
	rm -f $(OBJ) $(TR) .depend

depend:
	$(CXX) -M $(SRC) > .depend

