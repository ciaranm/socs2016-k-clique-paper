all : tables graphs
	latexmk -pdf -pdflatex='pdflatex -interaction=nonstopmode %O %S' -shell-escape paper

TABLES = gen-table-erdos.tex gen-table-dimacs10cluster.tex gen-table-dimacs10walshaw.tex gen-table-dimacs.tex gen-table-parallel.tex

tables : $(TABLES)

gen-table-erdos.tex : make-table.bash erdos.list
	bash ./make-table.bash erdos.list net > $@

gen-table-dimacs.tex : make-table.bash dimacs.list
	bash ./make-table.bash dimacs.list dimacs > $@

gen-table-dimacs10%.tex : make-table.bash dimacs10%.list
	bash ./make-table.bash dimacs10$*.list metis > $@

gen-table-parallel.tex : make-table-parallel.bash parallel.list
	bash ./make-table-parallel.bash parallel.list > $@

GRAPHS = \
	gen-graph-nodes-2.pdf \
	gen-graph-nodes.pdf \
	gen-graph-omega-2.pdf \
	gen-graph-omega.pdf

graphs : $(GRAPHS)

gen-graph-%.pdf : graph-%.gnuplot
	gnuplot $<
	sed -i -e '19s/^\(\\path.*\)/\% \1/' gen-graph-$*.tex # epic haxx
	latexmk -pdf gen-graph-$*

