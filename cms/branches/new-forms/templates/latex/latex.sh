#!/bin/bash
TEX="$1"
FNAME="$2"

FTMP=`mktemp`
FILE=`basename $FTMP`

cd `dirname $FTMP`

cat >$FILE.tex << EOF
\documentclass[12pt]{article}
\usepackage[latin1]{inputenc}
\usepackage{amsmath}
\usepackage{amsfonts}
\usepackage{amssymb}
\pagestyle{empty}
\setlength{\oddsidemargin}{-5in}
\setlength{\topmargin}{0cm}
\setlength{\headheight}{-5in}
\setlength{\headsep}{0cm}
\begin{document}
\[$TEX\]
\end{document}
EOF

if 	latex --interaction=nonstopmode $FILE.tex && \
	dvigif $FILE.dvi -o $FILE.gif ; then
	if [ -e $FILE.gif ]; then
		cp $FILE.gif $FNAME
		rm -f $FTMP.*
		exit 0
	fi
fi

rm -f $FTMP.*
exit 1

