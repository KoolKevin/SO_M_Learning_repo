#!/bin/bash

#SBATCH --account=tra24_IngInfB2
#SBATCH --partition=g100_usr_prod

#SBATCH --job-name=kkoltraka
#SBATCH -t 00:05:00
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=5

#SBATCH -o job_mpi.out
#SBATCH -e job_mpi.err

module load autoload intelmpi
srun ./versione_mpi 10000