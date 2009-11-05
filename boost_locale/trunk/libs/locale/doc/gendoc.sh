#!/bin/bash

if [ "$1" == "" ]; then
	pandoc -S --toc -c ../../../doc/html/boostbook.css -f markdown -t html index.txt -o index.html
else
	pandoc -S --toc -c "$1"/doc/html/boostbook.css -f markdown -t html index.txt -o index.html
fi

