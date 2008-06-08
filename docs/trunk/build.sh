#!/bin/bash
TARGETS="dbixx templates"

if [ "$1" == "clean" ]; then
	rm -fr *.log *.out *.aux *.lop *.toc $TARGETS *.pdf *.dvi *~ *.backup
	exit 0
fi

bld ()
{
	if latex2html --split=0 --noexternal_images -address 0 -info 0 -no_navigation --tmp=/tmp $1.tex && \
		pdflatex $1 \
		pdflatex $1 ; then
		return 0
	else
		return 1
	fi
}

for D in dbixx templates ; 
do
	if ! bld $D ; then
		exit 1;
	fi
done
