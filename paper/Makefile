all : tables graphs
	latexmk -pdf -pdflatex='pdflatex -interaction=nonstopmode %O %S' -shell-escape paper

TABLES = gen-table-erdos.tex gen-table-dimacs10cluster.tex gen-table-dimacs10walshaw.tex gen-table-dimacs.tex

tables : $(TABLES)

gen-table-erdos.tex : make-table-graphs.bash erdos.list
	bash ./make-table.bash erdos.list net > $@

gen-table-dimacs.tex : make-table.bash dimacs.list
	bash ./make-table.bash dimacs.list dimacs > $@

gen-table-dimacs10%.tex : make-table-graphs.bash dimacs10%.list
	bash ./make-table.bash dimacs10$*.list metis > $@

GRAPHS = \
	gen-graph-nodes-2.tex \
	gen-graph-nodes.tex \
	gen-graph-omega-2.tex \
	gen-graph-omega.tex

graphs : $(GRAPHS)

gen-graph-%.tex : graph-%.gnuplot
	gnuplot $<
	sed -i -e '4,5s/^\(\\path.*\)/\% \1/' $@ # epic haxx

