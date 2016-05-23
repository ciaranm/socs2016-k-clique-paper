# vim: set et ft=gnuplot sw=4 :

set terminal tikz standalone color size 3.3in,2.4in font '\scriptsize' preamble '\usepackage{times,microtype,amssymb,amsmath}'
set output "gen-graph-omega-2.tex"

set xlabel "Edge Probability"
set ylabel "Size of Maximum 2-clique"

set key top left Right at 0.0, 190

set border 3
set grid
set xtics nomirror
set ytics nomirror

set xrange [0:0.3]
set xtics 0.05
set mxtics
set yrange [0:200]

plot \
    "g-50-2.futuna-bmcsa1-omega-plot" u 1:2 with lines lw 2 ti "$G(50, x)^2$", \
    "g-100-2.futuna-bmcsa1-omega-plot" u 1:2 with lines lw 2 ti "$G(100, x)^2$", \
    "g-150-2.futuna-bmcsa1-omega-plot" u 1:2 with lines lw 2 ti "$G(150, x)^2$", \
    "g-200-2.futuna-bmcsa1-omega-plot" u 1:2 with lines lw 2 ti "$G(200, x)^2$"

