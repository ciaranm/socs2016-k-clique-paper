# vim: set et ft=gnuplot sw=4 :

set terminal tikz color size 3.3in,2in font '\tiny'
set output "gen-graph-omega.tex"

set xlabel "Edge Probability"
set ylabel "Size of Maximum $k$-clique"

set key center right Left at 0.25, 100

set border 3
set grid
set xtics nomirror
set ytics nomirror

set xrange [0:0.3]
set xtics 0.05
set mxtics
set yrange [0:200]

plot \
    "g-200-1.futuna-bmcsa1-omega-plot" u 1:2 with lines lc 2 lt 1 lw 2 ti "$\\omega$", \
    "g-200-2.futuna-bmcsa1-omega-plot" u 1:2 with lines lc 1 lt 1 lw 2 ti "$\\tilde{\\omega}_2$", \
    "g-200-3.futuna-bmcsa1-omega-plot" u 1:2 with lines lc 6 lt 1 lw 2 ti "$\\tilde{\\omega}_3$", \
    "g-200-4.futuna-bmcsa1-omega-plot" u 1:2 with lines lc 5 lt 1 lw 2 ti "$\\tilde{\\omega}_4$"

