TR=cc_templ_s
SRC=parser.cpp static.cpp

OBJ := $(patsubst %.cpp,%.o,$(SRC))

LIBS = -lboost_regex 
CXX = g++
CFLAGS = -Wall -g 


all: $(TR)

$(TR) : $(OBJ)
	$(CXX) -o $@ $^ $(LIBS)

.cpp.o:
	$(CXX) -c $(CFLAGS) $<

clean:
	rm -f $(OBJ) $(TR) .depend

depend:
	$(CXX) -M $(SRC) > .depend


-include .depend
