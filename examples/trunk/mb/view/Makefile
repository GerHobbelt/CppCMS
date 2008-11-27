-include ../Makefile.spec
TEMPLATES= master.tmpl forums.tmpl thread.tmpl
TARGET=simple
NAMESPACE=simple
SO_EXT=so

DLL=$(TARGET).$(SO_EXT)


CXXFLAGS+= -shared -fPIC -DPIC

all: .deps $(DLL)

.deps:
	mkdir .deps

$(DLL): $(TARGET).cpp
	$(CXX) -MD -MF .deps/$@.deps $(CXXFLAGS) -o $@ $< $(LIBS)

$(TARGET).cpp: $(TEMPLATES)
	$(CTMPL) -o $@ -d $(GETTEXT_DOMAIN) -n $(NAMESPACE) $(TEMPLATES)

clean:
	rm -fr *.cpp *.so .deps

-include .deps/$(DLL).deps
