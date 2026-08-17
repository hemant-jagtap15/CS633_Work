#!/bin/bash

echo "P,M,Run,Time" > timing.csv

for file in outputs/out_*.txt
do
    name=$(basename $file)

    P=$(echo $name | cut -d'_' -f2 | sed 's/P//')
    M=$(echo $name | cut -d'_' -f3 | sed 's/M//')

    run=1

    grep -E "^[[:space:]]*[0-9]" $file | awk '{print $3}' | while read t
    do
        echo "$P,$M,$run,$t" >> timing.csv
        run=$((run+1))
    done
done

echo "Done! Timing saved in timing.csv"
