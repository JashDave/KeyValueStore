#!/bin/bash


#Mode (statistics)
#Parameter Filename BinSize
function statMode {
 filename=$1
 binsize=$2
 mode=`uniq -c $filename | awk -v bs=$binsize 'BEGIN{idx=0; sum=0; maxsum=0; for(i=1; i<=$bs; i++){bin[i]=0;}}  NR<=$bs {bin[NR]=$1; sum+=$1; maxsum+=$1} NR>$bs{ sum-=bin[1]; sum+=$1; for(i=1; i<$bs; i++){bin[i]=bin[i+1];}; bin[bs]=$1; if(sum>maxsum){maxsum=sum; idx=NR} } END{}'`
 return mode 
}

combined_stats="stats.csv"

#Cleanup
function delAndMakeDirs {
	#d=$1
	rm $1$combined_stats
	for d in $1*/ ; do
if [ -d "$d" ]; then
	    imp=$d"intermediate_processing"
	    rm -r $imp
	    mkdir -p $imp
	    processed=$d"processed"
	    rm -r $processed
	    mkdir -p $processed
fi
	done
}



#Merge all file
function mergeFiles {
#MERGE
	d=$1
	merged=$2
	pattern=$3
	#for each file in the given dir $d with start pattern $pattern
	    for f in $d"$pattern"* ; do
	       #echo "$f"
	       tail -n+6 $f >> $merged
	    done
}


#Parameter $1=File starting pattern
function processFiles {
	#loop through all dirs
	for d in $2*/; do

	    #echo "JD: $d"
if [ -d "$d" ]; then
	    #echo "$d"
	    imp=$d"intermediate_processing"
	    processed=$d"processed"
	    merged=$imp"/Merged"$1".csv"
	    sorted=$imp"/Sorted"$1".csv"
	    statfile=$processed"/stats.csv"

#MERGE
	mergeFiles $d $merged $1

#SORT
	#sort the merged file
	sort -t, -k1 -n $merged > $sorted

#STATISTICS
	percent=1
	#Get min
	    min_j=`head -n+1 $sorted | awk -F, '{print $1}'`
	    echo "Min:"$min_j
	#Get max
	    max_j=`tail -n 1 $sorted | awk -F, '{print $1}'`
	    echo "Max:"$max_j
	#Avg 
	    avg_j=`awk -F, 'BEGIN{sum=0; count=0;} {sum=sum+$1; count++;} END{print sum/count}' $sorted`
	    echo "Avg:"$avg_j

	#Std Dev 
	    std_dev=`awk -F, 'BEGIN{sum=0; count=0;} {sum=sum+($1-'$avg_j')^2; count++;} END{print (sum/count)^0.5 }' $sorted`
	    echo "StdDev:"$std_dev

	#Calc no of lines in 1%
	    noOfLine=`cat $sorted | wc -l` #cat to ignore filename
	    onepercent=$(($percent * $noOfLine / 100)) #Or use `expr ...`
	    #echo $onepercent

	#Avg min top 1%
	    min_1p=`head -n+$onepercent $sorted | awk -F, 'BEGIN{sum=0; count=0;} {sum=sum+$1; count++;} END{print sum/count}'`
	    echo "Min 1%:"$min_1p

	#Avg max last 1%
	    max_1p=`tail -n $onepercent $sorted | awk -F, 'BEGIN{sum=0; count=0;} {sum=sum+$1; count++;} END{print sum/count}'`
	    echo "Max 1%:"$max_1p
	#Write Stats
	    desc="Min,Max,Avg,StdDev,Min $percent%,Max $percent%,No. Of Lines"
	    data="$min_j,$max_j,$avg_j,$std_dev,$min_1p,$max_1p,$noOfLine"
	    echo -e  "$desc\n$data" > $statfile
	#Append to common stats file
			thread_count=`basename $d | grep -Eo '[0-9]+$'`
	    echo "$thread_count,$d,$data" >> $2$combined_stats
	fi
	done
	sort -t, -k1 -n $2$combined_stats -o $2$combined_stats
}



dpath="../src/PerformanceData/Client/RAMCloud/2016-09-22/5/2KB/"

delAndMakeDirs $dpath
processFiles "ThreadedRead" $dpath
#processFiles "ThreadedWrite"
#processFiles "MixedThreadedRead"
#processFiles "MixedThreadedWrite"
