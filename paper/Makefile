all : tables graphs
	latexmk -pdf -pdflatex='pdflatex -interaction=nonstopmode %O %S' -shell-escape paper

TABLES =

tables : $(TABLES)

GRAPHS = \
	gen-graph-nodes-2.tex \
	gen-graph-nodes.tex \
	gen-graph-omega-2.tex \
	gen-graph-omega.tex

graphs : $(GRAPHS)

gen-graph-%.tex : graph-%.gnuplot
	gnuplot $<
	sed -i -e '4,5s/^\(\\path.*\)/\% \1/' $@ # epic haxx

