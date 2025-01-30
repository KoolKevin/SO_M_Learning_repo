#!/bin/bash

#SBATCH --account=tra24_IngInfBo
#SBATCH --partition=g100_usr_prod

#SBATCH --job-name=kkoltraka
#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH -c 48

#SBATCH -o job.out
#SBATCH -e job.err

srun ./sommavet 48


# for I in 12 24 48; do
# echo "Launching calculateR $I"
# srun calculateR $I
# done