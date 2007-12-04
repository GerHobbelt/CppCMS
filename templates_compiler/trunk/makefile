TR=cc_templ_d cc_templ_s ln_templ disass_templ
SRC=parser.cpp dynamic.cpp static.cpp linker.cpp

OBJ := $(patsubst %.cpp,%.o,$(SRC))

LIBS = -lboost_regex 
CXX = g++
CC = gcc
CFLAGS = -Wall -g -I./share


all: $(TR)

cc_templ_d: parser.o dynamic.o
	$(CXX) $(LIBS) -o $@ parser.o dynamic.o

cc_templ_s: parser.o static.o
	$(CXX) $(LIBS) -o $@ parser.o static.o

ln_templ: parser.o linker.o
	$(CXX) $(LIBS) -o $@ parser.o linker.o

disass_templ: disasemmble.c
	$(CC) $(CFLAGS) -o $@ disasemmble.c

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

clean:
	rm -f $(OBJ) $(TR) .depend

depend:
	$(CXX) -M $(SRC) > .depend

install:
	cp $(TR) /usr/local/bin

uninstall:
	cd /usr/local/bin
	rm -f $(TR)

-include .depend
