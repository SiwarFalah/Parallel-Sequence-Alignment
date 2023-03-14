#pragma once

#define ABC_NUMBER 26
#define LEVEL_ONE_LENGTH 9
#define LEVEL_TWO_LENGTH 11
#define NUM_WEIGHTS 4
#define SEQ1_MAX_LENGTH 3000
#define SEQ2_MAX_LENGTH 2000
#define BUFF_SIZE 1 << 20
#define NUM_VAR 3

struct score {
	int n; // offset
	int k; // hyphen index mutant
	int scoreWeight; // weight
}typedef Score;

int readFromFile(char*** seq2arr,char** seq1,int* numOfSequences,Score** topScore, int*** scoreMat);
int initScoreMat(int*** scoreMat);
void scoreMatToArray(int** scoreMat, int* scoreArray);
void createScoreMat(int*** scoreMat, int* weights);
void makeUpperStr(char* str);
int initScore(Score** topScore, int size);
void calcSeq2Size(int* seq2ArrSize, int numProc,int myRank,int numOfSequences);
void calcScoreAlgorithm(char* seq1,char* seq2,Score* topScore,int** scoreMat);
void printRes(Score* topScore,int workerArrSize);
void freeMemoryOfMaster(char** seq2Arr, char* seq1, Score* topScores, int size, Score* allScoresFromCuda, int* allScoresFromCudaBySize, int** scoreMat);
void freeMemoryOfWorker(char** seq2Arr, char* seq1, Score* topScores, int size, int** scoreMat);
int allocateAllScores(Score** allScoresFromCuda, int** allScoresFromCudaBySize, int seq2ArrSize, char** seq2Arr, char* seq1);
void calcMaxScoreInSeq2Parallel(Score* allScoresFromCuda, int* allScoresFromCudaBySize, Score* topScores, int seq2ArrSize);
void assignMaxScore(Score* topScore, Score* scoreToAssign);
int initCudaCalcs(char** seq2Arr, int seq2ArrSize, score* allScoresFromCuda, int* allScoresFromCudaBySize, char* seq1, int** scoreMat);
