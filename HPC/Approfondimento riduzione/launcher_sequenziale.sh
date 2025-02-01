#!/bin/bash

#SBATCH --account=tra24_IngInfB2
#SBATCH --partition=g100_usr_prod

#SBATCH --job-name=kkoltraka
#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1

#SBATCH -o job_sequenziale.out
#SBATCH -e job_sequenziale.err

srun  ./versione_sequenziale 10000 $num_thread
