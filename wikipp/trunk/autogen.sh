#!/bin/sh
aclocal -I m4
libtoolize --force
automake --add-missing
autoconf
autoheader
