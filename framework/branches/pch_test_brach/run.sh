#!/bin/bash

rm *.o *.gch

for f in all.h *.cpp;
do 
	echo $f
	g++ -I/usr/include/boost-1_33_1 -I/usr/include/db4.5 -c -g $1 $f
	if [ "$?" != "0" ]; then
		exit 1;
	fi
done 


