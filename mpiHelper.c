#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "mpiHelper.h"
#include <stddef.h>


char buffer[BUFF_SIZE];
int pos;

void createScoreType(MPI_Datatype* scoreType)
{
	Score maxScore;
    MPI_Datatype type[NUM_VAR] = { MPI_INT, MPI_INT, MPI_INT };
    int blocklen[NUM_VAR] = { 1, 1, 1 };
    MPI_Aint disp[NUM_VAR];

    disp[0] = (char *) &maxScore.n -	(char *) &maxScore;
    disp[1] = (char *) &maxScore.k -	(char *) &maxScore;
    disp[2] = (char *) &maxScore.scoreWeight -	(char *) &maxScore;

    MPI_Type_create_struct(NUM_VAR, blocklen, disp, type, scoreType);
    MPI_Type_commit(scoreType);
}

void masterSendDataToWorkers(char* seq1, int numOfSequences, int numProc, char** seq2Arr, int seq2ArrSize,int workerArrSize, int** scoreMat)
{
    int lenghtOfSeq1 = strlen(seq1) + 1;
    int i = seq2ArrSize;
    int lenghtOfSeq2;
    pos = 0;

    MPI_Pack(&lenghtOfSeq1,1, MPI_INT,buffer,BUFF_SIZE,&pos,MPI_COMM_WORLD);
    MPI_Pack(seq1,lenghtOfSeq1, MPI_CHAR,buffer,BUFF_SIZE,&pos,MPI_COMM_WORLD);
    for (int i = 0; i < ABC_NUMBER; i++)
        for (int j = 0; j < ABC_NUMBER; j++)
            MPI_Pack(&scoreMat[i][j], 1, MPI_INT,buffer,BUFF_SIZE,&pos,MPI_COMM_WORLD);
        
    MPI_Pack(&numOfSequences,1, MPI_INT,buffer,BUFF_SIZE,&pos,MPI_COMM_WORLD);

    for(int wId = 1; wId < numProc; wId++)
        MPI_Send(buffer,pos,MPI_PACKED,wId,0,MPI_COMM_WORLD);

    for(int wId = 1; wId < numProc; wId++)
    {
        pos = 0;
        MPI_Recv(&workerArrSize,1,MPI_INT,wId,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int j = 0; j < workerArrSize; j++)
        {
            lenghtOfSeq2 = strlen(seq2Arr[i])+1;
            MPI_Pack(&lenghtOfSeq2,1, MPI_INT,buffer,BUFF_SIZE,&pos,MPI_COMM_WORLD);
            MPI_Pack(seq2Arr[i],lenghtOfSeq2, MPI_CHAR,buffer,BUFF_SIZE,&pos,MPI_COMM_WORLD);
            i++;
        }
        if(workerArrSize > 0)
            MPI_Send(buffer,pos,MPI_PACKED,wId,0,MPI_COMM_WORLD);
    }   
}

int workerReciveDataFromMaster(char** seq1, int* numOfSequences, int* workerArrSize, char*** seq2Arr,int numProc,int myRank,Score** topScore, int** scoreMat)
{
    int lenghtOfSeq1;
    int lenghtOfSeq2;
    pos = 0;
    MPI_Recv(buffer,BUFF_SIZE,MPI_PACKED,MASTER,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Unpack(buffer,BUFF_SIZE,&pos,&lenghtOfSeq1,1,MPI_INT,MPI_COMM_WORLD);
    *seq1 = (char*)malloc(lenghtOfSeq1*sizeof(char));
    if(!*seq1)
    {
        fprintf(stderr,"Allocation error\n");
        return 0;
    }
    MPI_Unpack(buffer,BUFF_SIZE,&pos,*seq1,lenghtOfSeq1,MPI_CHAR,MPI_COMM_WORLD);

    for (int i = 0; i < ABC_NUMBER; i++)
        for (int j = 0; j < ABC_NUMBER; j++)
            MPI_Unpack(buffer,BUFF_SIZE,&pos,&scoreMat[i][j],1,MPI_INT,MPI_COMM_WORLD);
             
    MPI_Unpack(buffer,BUFF_SIZE,&pos,numOfSequences,1,MPI_INT,MPI_COMM_WORLD);

    calcSeq2Size(workerArrSize,numProc,myRank,*numOfSequences);
    if(!initScore(topScore,*workerArrSize))
        return 0; 
    
    *seq2Arr = (char**)malloc(*workerArrSize*sizeof(char*));
    if(!*seq2Arr)
    {
        fprintf(stderr,"Allocation error\n");
        return 0;
    }

    MPI_Send(workerArrSize,1,MPI_INT,MASTER,0,MPI_COMM_WORLD);
    pos = 0;
    if(*workerArrSize > 0)
    {
        MPI_Recv(buffer,BUFF_SIZE,MPI_PACKED,MASTER,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        for(int i = 0; i < *workerArrSize; i++)
        {
            MPI_Unpack(buffer,BUFF_SIZE,&pos,&lenghtOfSeq2,1,MPI_INT,MPI_COMM_WORLD);
            (*seq2Arr)[i] = (char*)malloc(lenghtOfSeq2*sizeof(char));
            if(!(*seq2Arr)[i])
            {
                fprintf(stderr,"Allocation error\n");
                return 0;
            }
            MPI_Unpack(buffer,BUFF_SIZE,&pos,(*seq2Arr)[i],lenghtOfSeq2,MPI_CHAR,MPI_COMM_WORLD);
        }
    }
    return 1;
}