1) Per creare il certificato necessario per l'accesso

	step ssh login 'kevin.koltraka@studio.unibo.it' --provisioner cineca-hpc

2) Per effettuare effettivamente il login

	ssh kkoltrak@login.g100.cineca.it

3) Per copiare un sorgente

	scp ./prod_matrici.c kkoltrak@login.g100.cineca.it:.

4) Per compilare con MPI

	module load autoload intelmpi
 	mpiicc -std=c99 sommavet.c –o sommavet


