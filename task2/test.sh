#!/bin/bash

while /bin/true
do
	for ((i = 1; i < 8; i++))
	do
		echo -n "$i " >> output
		(time -p ./a.out $i ) |& grep "real" | awk '{print $2}' >> output
	done
done