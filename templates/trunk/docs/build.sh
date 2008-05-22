latex2html --split=0 --noexternal_images -address 0 -info 0 -no_navigation --tmp=/tmp templates.tex
rst2latex.py --use-latex-citations --use-latex-footnotes templates.txt test.tex
