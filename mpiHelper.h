#pragma once
#define MASTER 0
#include "cFunctions.h"

void createScoreType(MPI_Datatype* scoreType);
void masterSendDataToWorkers(char* seq1, int numOfSequences, int numProc, char** seq2Arr, int seq2ArrSize, int workerArrSize, int** scoreMat);
int workerReciveDataFromMaster(char** seq1, int* numOfSequences, int* workerArrSize, char*** seq2Arr, int numProc,int myRank, Score** topScore, int** scoreMat);