#!/bin/bash

#SBATCH --account=tra24_IngInfB2
#SBATCH --partition=g100_usr_prod

#SBATCH --job-name=kkoltraka
#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1
#SBATCH --mem=20480 # non funziona ....

#SBATCH -o job_sequenziale.out
#SBATCH -e job_sequenziale.err

for dim in 2000 4000 6000 8000 10000; do
    echo "Launching ./versione_sequenziale $dim"
    srun ./versione_sequenziale $dim
done