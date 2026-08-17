#!/bin/bash
#SBATCH --job-name=P48_120
#SBATCH --output=../outputs/out_P48_120_%j.txt
#SBATCH --error=../outputs/err_P48_120_%j.txt

#SBATCH --nodes=1
#SBATCH --ntasks=48
#SBATCH --ntasks-per-node=48

#SBATCH --time=00:10:00
#SBATCH --partition=standard

echo "Running P=48, Grid=120^3 (5 times)"

for run in 1 2 3 4 5
do
    echo "Run $run:"
    mpirun -np 48 ../src 7 48 6 4 2 120 120 120 5 1000 2 500
done

echo "Finished P48 120"
