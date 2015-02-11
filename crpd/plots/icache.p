set term pngcairo dashed
set title "Breakdown Utilization Comparison"
set output "breakdown.png"
set xlabel "Block Reload Time (BRT) microseconds"
set ylabel "UCBs Shared with Subsequent Program Point"
set xtics 0,1
set xrange [0:400]
set yrange [0:100]
# UOE Line Style
set linestyle 1 lt 1 lc rgb "black"
# BEPP Line Style
set linestyle 2 lt 2 lc rgb "blue"
# EPP Line Style
set linestyle 3 lt 3 lc rgb "green"
plot \
'icache.min.dat' using 1:3:2 with labels offset 0,-1 tc ls 1 notitle , \
'icache.min.dat' using 1:3 ls 1 pt 7 notitle, \
'icache.min.dat' using 1:3 title "Minimum" with lines ls 1 , \
\
'icache.max.dat' using 1:3:2 with labels offset 0,1 tc ls 2 notitle, \
'icache.max.dat' using 1:3 ls 2 pt 7 notitle, \
'icache.max.dat' using 1:3 title "Maximum" with lines ls 2 , \
