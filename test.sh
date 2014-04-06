#!/bin/bash
make clean
make
echo $'****BEGINNING TEST****\n\n' > test.txt
SUM=0
for i in {1..100}
do
	mkdir generic_directory
	mkdir moving_directory
	START=$(date +%s%N)
	mv moving_directory ./generic_directory
	END=$(date +%s%N)
	DIFF=$(( $END -$START ))
	SUM=$(( $DIFF + $SUM ))
	rm -r generic_directory
done

AVERAGE=$(( $SUM/100 ))
SUM=0
echo "average rename: $AVERAGE" >> test.txt

for i in {1..100}
do
	dd if=/dev/zero of=/tmp/bigfile.txt bs=1048576 count=1
	START=$(date +%s%N)
	cp /tmp/bigfile.txt ./
	END=$(date +%s%N)
	DIFF=$(( $END -$START ))
	SUM=$(( $DIFF + $SUM ))
	rm ./bigfile.txt
done

AVERAGE=$(( $SUM/100 ))
SUM=0
echo "average move: $AVERAGE" >> test.txt

for i in {1..100}
do

	mkdir trash
	export TRASH=$PWD/trash
	mkdir /tmp/smeagol
	dd if=/dev/zero of=/tmp/smeagol/bigfile.txt bs=1048576 count=1
	START=$(date +%s%N)
	./rm -r /tmp/smeagol/bigfile.txt
	END=$(date +%s%N)
	DIFF=$(( $END -$START ))
	SUM=$(( $DIFF + SUM ))
	./dump
done

AVERAGE=$(( $SUM/100 ))
echo "average rm -r: $AVERAGE" >> test.txt

