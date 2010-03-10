#!/bin/bash

rm -fr index.html html
pandoc -B header.html -T Boost.Locale -S --toc -c boostbook.css -f markdown -t html index.txt -o index.html
doxygen


