    #include <stdio.h>
    #include <stdlib.h>
    #include <mpi.h>
    #include <time.h>

    #define DIM 256
    
   int main(int argc, char* argv[])
    {
        MPI_Init(&argc, &argv);
     
        // Get number of processes and check that 4 processes are used
        int size, my_rank;
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
        if(size > DIM)
        {
            printf("il numero di processi %d è maggiore della dimensione %d.\n",size, DIM);
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
     
        printf("[Sono il processo %d di %d]\n",my_rank,size);
     
        // Define my value
        int  my_A[DIM], my_B[DIM], my_C[DIM];
     
        if (my_rank == 0)
        {
            int A[DIM],B[DIM], i ;
            srand((unsigned int)time(NULL)); 
            for(i=0;i<DIM;i++){
                A[i]=rand()%50;
                B[i]=rand()%50;
            }
            /*printf("inserire i %d elementi di A:\n", DIM);
            for(i=0; i<DIM; i++) //inizializzazione A
                scanf("%d", &A[i]);
            printf("inserire i %d elementi di B:\n", DIM);
            for(i=0; i<DIM; i++) //inizializzazione A
                scanf("%d", &B[i]);
            */
            // verifica
            printf("[processo %d] vettore A:\n", my_rank);
            for(i=0;i<DIM;i++)
                printf("\t%d\n",A[i]);
            printf("[processo %d] vettore B:\n", my_rank);
            for(i=0;i<DIM;i++)
                printf("\t%d\n",B[i]);

            MPI_Scatter(A, DIM/size, MPI_INT, &my_A, DIM/size, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Scatter(B, DIM/size, MPI_INT, &my_B, DIM/size, MPI_INT, 0, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Scatter(NULL, DIM/size, MPI_INT, &my_A, DIM/size, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Scatter(NULL, DIM/size, MPI_INT, &my_B, DIM/size, MPI_INT, 0, MPI_COMM_WORLD);
        }
     
        
        for(int i=0; i<DIM/size; i++)
            my_C[i]=my_A[i]+my_B[i];
            
        if (my_rank==0) //collettore
        {   int  C[DIM];
            MPI_Gather(&my_C, DIM/size, MPI_INT, C, DIM/size, MPI_INT, 0, MPI_COMM_WORLD);
            printf("Risultato C=A+B:\n");
            for(int i=0; i<DIM; i++)
                printf("\t%d\n", C[i]);
        }
        else
            MPI_Gather(&my_C, DIM/size, MPI_INT, NULL, 1, MPI_INT, 0, MPI_COMM_WORLD);
     
        MPI_Finalize();
     
        return EXIT_SUCCESS;
    }

