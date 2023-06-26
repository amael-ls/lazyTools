#!/bin/sh
echo "execution of prog $0"
echo "option mv = $3"
for i in `ls *.$1`
do
	mv $3 $i ${i%.$1}.$2
done

