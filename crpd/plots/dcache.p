set term pngcairo dashed
set title "bsort100 Data Cache"
set output "bsort-dcache.png"
set xlabel "Program Point"
set ylabel "UCBs Shared with Subsequent Program Point"
set xtics 0,1
set xrange [0:14]
set yrange [14:22]
# Minimum Line Style
set linestyle 1 lt 1 lc rgb "black"
# Maximum Line Style
set linestyle 2 lt 2 lc rgb "blue"
plot \
'dcache.min.dat' using 1:3:2 with labels offset 0,-1 tc ls 1 notitle , \
'dcache.min.dat' using 1:3 ls 1 pt 7 notitle, \
'dcache.min.dat' using 1:3 title "Minimum" with lines ls 1 , \
\
'dcache.max.dat' using 1:3:2 with labels offset 0,1 tc ls 2 notitle, \
'dcache.max.dat' using 1:3 ls 2 pt 7 notitle, \
'dcache.max.dat' using 1:3 title "Maximum" with lines ls 2 , \
