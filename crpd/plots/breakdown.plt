#set term pngcairo dashed
set term postscript eps color
#set title "Breakdown Utilization Comparison"
#set output "breakdown.png"
set output "breakdown.eps"
set xlabel "Block Reload Time (BRT) microseconds"
set ylabel "Breakdown Utilization Percentage"
set xtics 0,100
set xrange [0:400]
set yrange [0:1]
# UOE Line Style
set linestyle 1 lt 1 pt 5 lc rgb "black"
# BEPP Line Style
set linestyle 2 lt 1 pt 6 lc rgb "blue"
# EPP Line Style
set linestyle 3 lt 1 pt 7 lc rgb "green"
plot \
'uoe_table.txt' using 1:2 ls 1 notitle , \
'uoe_table.txt' using 1:2 title "UOE" with linespoints ls 1 pt 5, \
\
'bepp_table.txt' using 1:2 ls 2 notitle, \
'bepp_table.txt' using 1:2 title "BEPP" with linespoints ls 2 pt 6, \
\
'epp_table.txt' using 1:2 ls 3 notitle, \
'epp_table.txt' using 1:2 title "EPP" with linespoints ls 3 pt 7, \
