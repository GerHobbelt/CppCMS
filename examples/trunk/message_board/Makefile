all: bb.fcgi bb.opcode bb.db

bb.fcgi: bb.cpp
	g++ bb.cpp -o bb.fcgi -lcppcms -ltmpl -ldbixx

bb.opcode: master.tmpl main.tmpl topic.tmpl
	tmpl_bld -o bb.opcode  master.tmpl main.tmpl topic.tmpl

bb.db: 
	sqlite3 bb.db <db.sql

clean:
	rm -fr *.fcgi *.exe *.opcode *.s .libs *.db
