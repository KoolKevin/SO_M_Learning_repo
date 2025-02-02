#!/bin/bash

#SBATCH --account=tra24_IngInfB2
#SBATCH --partition=g100_usr_prod

#SBATCH --job-name=kkoltraka
#SBATCH -t 00:05:00
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=5

#SBATCH -o job_mpi_strong.out
#SBATCH -e job_mpi_strong.err

DIM=2000

module load autoload intelmpi
srun ./versione_mpi $DIM