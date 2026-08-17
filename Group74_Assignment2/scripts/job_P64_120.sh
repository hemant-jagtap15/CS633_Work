#!/bin/bash
#SBATCH --job-name=P64_120
#SBATCH --output=../outputs/out_P64_120_%j.txt
#SBATCH --error=../outputs/err_P64_120_%j.txt

#SBATCH --nodes=2
#SBATCH --ntasks=64
#SBATCH --ntasks-per-node=32

#SBATCH --time=00:10:00
#SBATCH --partition=standard

echo "Running P=64, Grid=120^3 (5 times)"

for run in 1 2 3 4 5
do
    echo "Run $run:"
    mpirun -np 64 ../src 7 32 4 4 4 120 120 120 5 1000 2 500
done

echo "Finished P64 120"
