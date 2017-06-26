#!/bin/bash

#dpath="../src/PerformanceData/Client/RAMCloud/2016-09-15/1/2KB/"
dpath="../../Helper/src/PerformanceData/Server/RAMCloud/2016-09-15/1/100KB/"

fnwstat=$dpath"fnwstat.csv"
fcpustat=$dpath"fcpustat.csv"
fmemstat=$dpath"fmemstat.csv"

rm $fnwstat
rm $fcpustat
rm $fmemstat

#perf_data_ThreadedMixedRW_DataSize2000Bytes_Iter5000_2xTC1
filename="perf_data_ThreadedRead*"
#filename="perf_data_ThreadedWrite*"
#filename="perf_data_ThreadedMixed*"
iface="ens3"

for dir in $dpath*/; do
echo $dir
thread_count=`basename $dir | grep -Eo '[0-9]+$'`
sar -f $dir$filename -n DEV | grep $iface | tail -n 1 | awk '{print "'"$thread_count,$dir"'" "," $5 "," $6}' >> $fnwstat
sar -f $dir$filename -u | tail -n 1 | awk '{print "'"$thread_count,$dir"'" "," 100-$8}' >> $fcpustat
sar -f $dir$filename -r | tail -n 1 | awk '{print "'"$thread_count,$dir"'" "," $4}' >> $fmemstat
done

sort -t, -k1 -n $fnwstat  -o $fnwstat
sort -t, -k1 -n $fcpustat -o $fcpustat
sort -t, -k1 -n $fmemstat -o $fmemstat


#sar -f sar_threadedread -n DEV | tail -n 2 | awk 'NR<2 {print $5 " " $6}'












