#!/bin/bash

#SBATCH --account=tra24_IngInfB2
#SBATCH --partition=g100_usr_prod

#SBATCH --job-name=kkoltraka
#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=2

#SBATCH -o job.out
#SBATCH -e job.err

module load autoload intelmpi
srun ./versione_mpi 3

# for I in 12 24 48; do
# echo "Launching calculateR $I"
# srun calculateR $I
# done
