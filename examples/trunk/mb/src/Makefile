LDFLAGS=-export-dynamic
SRC=forums.cpp mb.cpp thread.cpp main.cpp
OBJ=$(patsubst %.cpp,%.o,$(SRC))
DEPS=$(patsubst %.o,.deps/%.o.dep,$(OBJ))

TARGET=../mb

all: .deps $(TARGET)

.deps:
	mkdir -p .deps

$(TARGET): $(OBJ)
	$(CXX)  $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -MD -MF .deps/$@.dep -c -o $@ $<

clean:
	rm -fr .deps *.o $(TARGET)

-include $(DEPS)
-include ../Makefile.spec
