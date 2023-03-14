#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <mpi.h>
#include "cFunctions.h"
#include "mpiHelper.h"

int main(int argc, char *argv[]) 
{
	char** seq2Arr; // sequence 2 string
	int seq2ArrSize; // Length of sequence 2 string for each process
	int workerArrSize; // Length of worker array size
	char* seq1; // sequence 1 string
	int numOfSequences; // number of sequences from file
	int** scoreMat; // score mat with weights
	Score* topScores; // The best score for every sequence 2 string
	double time; // calculate serial and parallel times
	int myRank, numProc;

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
	MPI_Comm_size(MPI_COMM_WORLD,&numProc);

	MPI_Datatype scoreType;
	createScoreType(&scoreType);

	if(myRank == MASTER)
	{
		Score* allScoresFromCuda;
		int* allScoresFromCudaBySize;	

		if(!readFromFile(&seq2Arr,&seq1,&numOfSequences,&topScores,&scoreMat))
			MPI_Abort(MPI_COMM_WORLD,1);

		calcSeq2Size(&seq2ArrSize,numProc,myRank,numOfSequences);

		if(!allocateAllScores(&allScoresFromCuda, &allScoresFromCudaBySize, seq2ArrSize, seq2Arr, seq1))
			MPI_Abort(MPI_COMM_WORLD,1);

		masterSendDataToWorkers(seq1,numOfSequences,numProc,seq2Arr,seq2ArrSize,workerArrSize, scoreMat);
			
		time = MPI_Wtime();
		
		if(!initCudaCalcs(seq2Arr, seq2ArrSize, allScoresFromCuda, allScoresFromCudaBySize, seq1, scoreMat))
			MPI_Abort(MPI_COMM_WORLD,1);
		
		calcMaxScoreInSeq2Parallel(allScoresFromCuda, allScoresFromCudaBySize, topScores, seq2ArrSize);
	
        int numJobs = seq2ArrSize;
        for (int wId = 1; wId < numProc; wId++)   
        {
			MPI_Recv(&workerArrSize,1,MPI_INT,wId,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(topScores + numJobs, workerArrSize, scoreType, wId, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			numJobs += workerArrSize;
        }   

		// handle remains
		for (; numJobs < numOfSequences; numJobs++)
			calcScoreAlgorithm(seq1, seq2Arr[numJobs],&topScores[numJobs],scoreMat); 
                  
        printf("Time for parallel is %lf\n", MPI_Wtime()-time);
		printRes(topScores,numOfSequences);
                   
	  	memset(topScores, 0, sizeof(Score)*numOfSequences);
         
        time = MPI_Wtime();     
        for(int i=0; i<numOfSequences; i++)
          calcScoreAlgorithm(seq1, seq2Arr[i], &topScores[i],scoreMat); 
        printf("Time for serial is %lf\n", MPI_Wtime()-time);
		printRes(topScores,numOfSequences);

		freeMemoryOfMaster(seq2Arr, seq1, topScores, numOfSequences, allScoresFromCuda, allScoresFromCudaBySize, scoreMat);
		
	}
	else
	{	
		initScoreMat(&scoreMat);

		if(!workerReciveDataFromMaster(&seq1,&numOfSequences,&workerArrSize,&seq2Arr,numProc,myRank,&topScores, scoreMat))
			MPI_Abort(MPI_COMM_WORLD,1);

		#pragma omp parallel for 
    		for (int i = 0; i < workerArrSize; i++)
       			 calcScoreAlgorithm(seq1, seq2Arr[i], &topScores[i], scoreMat);
		
		MPI_Send(&workerArrSize,1,MPI_INT,MASTER,0,MPI_COMM_WORLD);
		MPI_Send(topScores,workerArrSize,scoreType,MASTER,0,MPI_COMM_WORLD);

		freeMemoryOfWorker(seq2Arr, seq1, topScores, workerArrSize, scoreMat);		
	}

	MPI_Finalize();

	return 0;
	
}


	