#!/bin/bash

while /bin/true
do
	for ((i = 1; i < 13; i++))
	do
		echo -n "$i " >> output
		(time -p ./a.out $i ) |& grep "real" | awk '{print 1/$2}' >> output
	done
done
