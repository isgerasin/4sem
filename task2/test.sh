#!/bin/bash
MC=16
rm output
for ((k = 1; k < 5; k++))
do
	for ((i = 1; i < ${MC}+1; i++))
	do
		echo -n "$i, " >> output
		(time -p ./a.out $i ) |& grep "real" | awk '{print $2}' >> output
		echo -e "[$i / ${MC}]\e[32mDone\e[0m"
	done
	echo "$k"
done
