all : tables graphs
	latexmk -pdf -pdflatex='pdflatex -interaction=nonstopmode %O %S' -shell-escape paper

TABLES = gen-table-erdos.tex gen-table-dimacs10cluster.tex gen-table-dimacs10walshaw.tex gen-table-dimacs.tex

tables : $(TABLES)

gen-table-erdos.tex : make-table-graphs.bash erdos.list
	bash ./make-table-graphs.bash erdos.list ~/phd/datasets/graph/pajek/ net > $@

gen-table-dimacs.tex : make-table-graphs.bash dimacs.list
	bash ./make-table-graphs.bash dimacs.list ~/phd/datasets/graph/dimacs/ dimacs > $@

gen-table-dimacs10%.tex : make-table-graphs.bash dimacs10%.list
	bash ./make-table-graphs.bash dimacs10$*.list ~/phd/datasets/graph/dimacs10/ metis > $@

GRAPHS = \
	gen-graph-nodes-2.tex \
	gen-graph-nodes.tex \
	gen-graph-omega-2.tex \
	gen-graph-omega.tex

graphs : $(GRAPHS)

gen-graph-%.tex : graph-%.gnuplot
	gnuplot $<
	sed -i -e '4,5s/^\(\\path.*\)/\% \1/' $@ # epic haxx

