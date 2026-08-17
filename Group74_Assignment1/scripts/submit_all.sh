#!/bin/bash

echo "Submitting all 6 jobs..."

sbatch job_P8_M262144.sh
sbatch job_P8_M1048576.sh

sbatch job_P16_M262144.sh
sbatch job_P16_M1048576.sh

sbatch job_P32_M262144.sh
sbatch job_P32_M1048576.sh

echo "All jobs submitted!"
