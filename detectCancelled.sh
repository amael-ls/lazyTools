#!/bin/bash

echo Program $0 is running with argument $1

for file in $(grep CANCELLED slurm-$1_*.out -lR)
do
	begin=`awk -F_ '{print length($1)+1}' <<< "$file"`
	end=`awk -F. '{print length($1)+1}' <<< "$file"`
	len="(($end - $begin - 1))"
	array=$array","${file:$begin:$len}
done

echo $array