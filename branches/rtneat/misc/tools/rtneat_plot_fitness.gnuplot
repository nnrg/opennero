# this is an example of plotting a raw fitness component from the nero_log file - when the log format is settled,
# this can be done for all the raw compoenents (on their respective scales)
set xlabel 'tick'
set ylabel '-d^2'
set yrange [-4e8:0]
set xrange [0:10000]
set title 'negative square distance to flag'
plot "< grep mean ~/.opennero/nero_log.txt | awk '{print $14, $23, $41}'" using :1 title 'min', "" using :2 title 'max', "" using :3 title 'mean'
