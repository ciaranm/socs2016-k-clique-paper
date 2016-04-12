# vim: set et ft=gnuplot sw=4 :

set terminal tikz color size 3.3in,2.4in font '\tiny'
set output "gen-graph-nodes.tex"

set xlabel "Edge Probability"
set ylabel "Average Search Nodes"

set key top right Left at 0.28, 50000

set border 3
set grid
set xtics nomirror
set ytics nomirror

set xrange [0:0.3]
set xtics 0.05
set mxtics
set yrange [1:200000]
set logscale y
set format y '$10^%T$'

plot \
    "g-200-1.futuna-bmcsa1-nodes-plot" u 1:2 with lines lw 2 ti "$k = 1$", \
    "g-200-2.futuna-bmcsa1-nodes-plot" u 1:2 with lines lw 2 ti "$k = 2$", \
    "g-200-3.futuna-bmcsa1-nodes-plot" u 1:2 with lines lw 2 ti "$k = 3$", \
    "g-200-4.futuna-bmcsa1-nodes-plot" u 1:2 with lines lw 2 ti "$k = 4$"

