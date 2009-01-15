DIRS=src view

all:	mb.db
	cd src ; make
	cd view ; make 
clean:
	cd src ; make clean
	cd view ; make clean
	rm -fr cppcms_rundir
	rm -f mb.db

mb.db:
	sqlite3 mb.db <sqlite3.sql
