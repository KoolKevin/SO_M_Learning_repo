#!/bin/bash

#SBATCH --account=tra24_IngInfB2
#SBATCH --partition=g100_usr_prod

#SBATCH --job-name=kkoltraka
#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH -c 48

#SBATCH -o job_omp_weak.out
#SBATCH -e job_omp_weak.err

for params in "200 1" "1000 5" "2000 10" "5000 25" "9600 48"; do
    read dim num_thread <<< "$params"  # Divide la stringa nei due valori

    echo "Launching ./versione_omp $dim $num_thread"
    srun ./versione_omp $dim $num_thread
done

