#!/bin/bash

#SBATCH --job-name=kkoltraka
#SBATCH --nodes=5
#SBATCH --ntasks-per-node=48
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --time 00:05:00

#SBATCH --account=tra24_IngInfB2
#SBATCH --partition=g100_usr_prod

module load autoload intelmpi
srun ./sommavet
