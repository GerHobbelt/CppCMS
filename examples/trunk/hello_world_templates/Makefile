all: hello-tmpl.fcgi first.opcode

hello-tmpl.fcgi: hello-tmpl.cpp
	g++ hello-tmpl.cpp -o hello-tmpl.fcgi -lcppcms -ltmpl

first.opcode: first.tmpl
	tmpl_bld -o first.opcode first.tmpl

clean:
	rm -f *.fcgi *.exe *.opcode *.s
