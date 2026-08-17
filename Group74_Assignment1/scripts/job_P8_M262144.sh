#!/bin/bash
#SBATCH --job-name=P8_M262144
#SBATCH --output=../outputs/out_P8_M262144_%j.txt
#SBATCH --error=../outputs/err_P8_M262144_%j.txt

#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --ntasks-per-node=8

#SBATCH --time=00:10:00
#SBATCH --partition=standard


M=262144
D1=2
D2=4
T=10
SEED=1000

echo "Running P=8 M=262144 (5 times)"

for run in 1 2 3 4 5
do
    echo "Run $run:"
    mpirun -np 8 ../src $M $D1 $D2 $T $SEED
done
