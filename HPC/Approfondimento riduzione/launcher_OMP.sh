#!/bin/bash

#SBATCH --account=tra24_IngInfB2
#SBATCH --partition=g100_usr_prod

#SBATCH --job-name=kkoltraka
#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH -c 48

#SBATCH -o job_omp.out
#SBATCH -e job_omp.err

for num_thread in 1 5 10 25 48 50 100; do
 	echo "Launching ./versione_omp 10000 $num_thread"
	srun  ./versione_omp 10000 $num_thread
done
