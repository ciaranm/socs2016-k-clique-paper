# vim: set et ft=gnuplot sw=4 :

set terminal tikz color size 3.3in,2.4in font '\tiny'
set output "gen-graph-nodes-2.tex"

set xlabel "Edge Probability"
set ylabel "Average Search Nodes"

set key top right Right at 0.29, 50000

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
    "g-50-2.futuna-bmcsa1-nodes-plot" u 1:2 with lines lw 2 ti "$G(50, x)^2$", \
    "g-100-2.futuna-bmcsa1-nodes-plot" u 1:2 with lines lw 2 ti "$G(100, x)^2$", \
    "g-150-2.futuna-bmcsa1-nodes-plot" u 1:2 with lines lw 2 ti "$G(150, x)^2$", \
    "g-200-2.futuna-bmcsa1-nodes-plot" u 1:2 with lines lw 2 ti "$G(200, x)^2$"

