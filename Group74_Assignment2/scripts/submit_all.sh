#!/bin/bash

echo "Submitting all jobs..."

sbatch job_P32_120.sh
sbatch job_P32_240.sh

sbatch job_P48_120.sh
sbatch job_P48_240.sh

sbatch job_P64_120.sh
sbatch job_P64_240.sh

sbatch job_P96_120.sh
sbatch job_P96_240.sh

echo "All jobs submitted!"
