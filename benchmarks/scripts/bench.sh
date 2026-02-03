#!/bin/bash

# adapted from https://hal.science/hal-02391841

TIMEOUT=1200
DIRECTORIES="$2"

date
echo "Software : $1"
echo "Timeout : $TIMEOUT"

for dir in $DIRECTORIES
do
	echo $dir;

	files=$(find -L $dir -type f -name $3)

	for file in $files
	do
		echo $file
        time systemd-run --user --scope \
			  -p MemoryMax=32G \
			  -p MemorySwapMax=7G \
			  timeout $TIMEOUT $1 $file
		echo "********************"
	done
	date
done
