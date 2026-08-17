#!/bin/bash
#SBATCH --job-name=P32_120
#SBATCH --output=../outputs/out_P32_120_%j.txt
#SBATCH --error=../outputs/err_P32_120_%j.txt

#SBATCH --nodes=1
#SBATCH --ntasks=32
#SBATCH --ntasks-per-node=32

#SBATCH --time=00:10:00
#SBATCH --partition=standard

echo "Running P=32, Grid=120^3 (5 times)"

for run in 1 2 3 4 5
do
    echo "Run $run:"
    mpirun -np 32 ../src 7 32 4 4 2 120 120 120 5 1000 2 500
done

echo "Finished P32 120"
