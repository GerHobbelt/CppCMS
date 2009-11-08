#!/bin/bash

if [ "$1" == "" ]; then
	pandoc -T Boost.Locale -S --toc -c ../../../doc/html/boostbook.css -f markdown -t html index.txt -o index.html
else
	pandoc -T Boost.Locale -S --toc -c "$1"/doc/html/boostbook.css -f markdown -t html index.txt -o index.html
fi

