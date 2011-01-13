#!/bin/bash

#pandoc -B header.html -T Boost.Locale -S --toc -c boostbook.css -f markdown -t html index.txt -o tutorial.html
rm -fr html

./Markdown.pl <tutorial.txt | ./create_toc.pl | cat tut_header.html - tut_footer.html >tutorial.html

doxygen
cp boostbook.css boost_designed_for.png html
mv tutorial.html html


