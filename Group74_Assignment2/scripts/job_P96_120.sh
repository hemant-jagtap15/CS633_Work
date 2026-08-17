#!/bin/bash
#SBATCH --job-name=P96_120
#SBATCH --output=../outputs/out_P96_120_%j.txt
#SBATCH --error=../outputs/err_P96_120_%j.txt

#SBATCH --nodes=2
#SBATCH --ntasks=96
#SBATCH --ntasks-per-node=48

#SBATCH --time=00:10:00
#SBATCH --partition=standard

echo "Running P=96, Grid=120^3 (5 times)"

for run in 1 2 3 4 5
do
    echo "Run $run:"
    mpirun -np 96 ../src 7 48 6 4 4 120 120 120 5 1000 2 500
done

echo "Finished P96 120"
