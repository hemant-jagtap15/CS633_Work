#!/bin/bash

echo "P,Size,Run,Time" > timing.csv

for file in outputs/out_*.txt
do
    name=$(basename $file)

    # Extract P and Size
    P=$(echo $name | cut -d'_' -f2 | sed 's/P//')
    SIZE=$(echo $name | cut -d'_' -f3)

    run=1

    # Extract only timing lines (floating numbers)
    grep -E "^[0-9]+\.[0-9]+$" $file | while read t
    do
        echo "$P,$SIZE,$run,$t" >> timing.csv
        run=$((run+1))
    done

done

echo "Done Timing saved in timing.csv"
