#!/bin/bash
MC=14
rm output
# echo "N   real   user   %" >> output
for ((k = 1; k < 2; k++))
do
<<<<<<< HEAD
	for ((i = 1; i < 13; i++))
	do
		echo -n "$i " >> output
		(time -p ./a.out $i ) |& grep "real" | awk '{print 1/$2}' >> output
	done
=======
	echo -n "1, " >> output
	ONETIME=`(time -p ./a.out 1 ) |& grep "real" | awk '{print $2}'`
	echo ${ONETIME} >> output
	echo -e "[1 / ${MC}]\e[32mDone\e[0m"

	for ((i = 2; i < ${MC}+1; i++))
	do
		echo -n "$i, " >> output
		(time -p ./a.out $i ) |& grep "real" | awk '{print $2, " ", '$ONETIME'/('$i'*($2))}' >> output
		echo -e "[$i / ${MC}]\e[32mDone\e[0m"
	done
	echo "$k"
    cat output
>>>>>>> 220d2044746070e4a4d39a50aefebdc27c3b5c32
done
