#!/bin/bash

#SBATCH --account=tra24_IngInfB2
#SBATCH --partition=g100_usr_prod

#SBATCH --job-name=kkoltraka
#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=5
#SBATCH --mem=20GB # non funziona...

#SBATCH -o job_mpi_weak.out
#SBATCH -e job_mpi_weak.err

DIM=1600

module load autoload intelmpi
srun ./versione_mpi $DIM