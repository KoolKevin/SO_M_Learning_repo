Nello script di lancio dei job usare come account sempre:

    #SBATCH --account=tra24_IngInfB2

La partizione consigliata è quella più GP:

    #SBATCH --partition=g100_usr_prod

Ricordarsi anche di caricare il modulo per l'esecuzione (oltre che per la compilazione) di programmi MPI

    module load autoload intelmpi

...

Modalità interattiva sconsigliata, spreca tempo di calcolo 