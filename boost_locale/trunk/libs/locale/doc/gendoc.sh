#!/bin/bash

pandoc -B header.html -T Boost.Locale -S --toc -c boostbook.css -f markdown -t html index.txt -o tutorial.html
doxygen
cp tutorial.html boostbook.css boost_designed_for.png html


