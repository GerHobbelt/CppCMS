-include ../Makefile.spec
TEMPLATES= master.tmpl forums.tmpl thread.tmpl
TARGET=simple
NAMESPACE=simple

DLL=$(TARGET).$(SO_EXT)

all: .deps $(DLL)

.deps:
	mkdir .deps

$(DLL): $(TARGET).cpp
	$(CXX) -MD -MF .deps/$@.deps $(VIEW_CXXFLAGS) $(VIEW_LDFLAGS) -o $@ $< $(VIEW_LIBS)

$(TARGET).cpp: $(TEMPLATES)
	$(CTMPL) -o $@ -d $(GETTEXT_DOMAIN) -n $(NAMESPACE) $(TEMPLATES)

clean:
	rm -fr *.cpp $(DLL) .deps

-include .deps/$(DLL).deps
