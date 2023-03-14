#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cFunctions.h"
#include <omp.h>
#include <ctype.h>


int readFromFile(char*** seq2arr,char** seq1,int* numOfSequences,Score** topScore, int*** scoreMat)
{
    int weights[NUM_WEIGHTS]; // array of weights
    char tempString[SEQ1_MAX_LENGTH];

    for(int i=0;i<NUM_WEIGHTS;i++)
    {
        scanf("%d",&weights[i]);
        if(i!= 0)
            weights[i] *=-1;
    }

    initScoreMat(scoreMat);
    createScoreMat(scoreMat, weights);

    scanf("%s",tempString);
    *seq1 = (char*)malloc(strlen(tempString)*sizeof(char)+1);
    if(!*seq1)
    {
        fprintf(stderr,"Allocation error\n");
        return 0;
    }
    strcpy(*seq1,tempString);
    makeUpperStr(*seq1);

    scanf("%d",numOfSequences);

    *seq2arr = (char**)malloc(sizeof(char*)* (*numOfSequences));
    if(!*seq2arr)
    {
        fprintf(stderr,"Allocation error\n");
        return 0;
    }

    for(int i = 0; i < *numOfSequences; i++)
    {
        scanf("%s",tempString);
        (*seq2arr)[i] = (char*)malloc(strlen(tempString)*sizeof(char)+1);
        if(!(*seq2arr)[i])
        {
            fprintf(stderr,"Allocation error\n");
            return 0;
        }
        strcpy((*seq2arr)[i],tempString);
        makeUpperStr((*seq2arr)[i]);
    }

    if(!initScore(topScore, *numOfSequences))
        return 0;

    return 1;
}

void makeUpperStr(char* str)
{
    for(int i=0;i<strlen(str);i++)
        str[i] = toupper(str[i]);
}

void createScoreMat(int*** scoreMat, int* weights)
{
    const char* levelOneStrings[LEVEL_ONE_LENGTH] = {"NDEQ", "NEQK", "STA", "MILV", "QHRK", "NHQK", "FYW", "HY", "MILF"};
    const char* levelTwoStrings[LEVEL_TWO_LENGTH] = {"SAG", "ATV", "CSA", "SGND", "STPA", "STNK", "NEQHRK", "NDEQHK", "SNDEQK", "HFY", "FVLIM"};
    char abcArray[ABC_NUMBER] = {'A', 'B' ,'C', 'D', 'E', 'F' ,'G' ,'H' ,'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
    int isLevelOneFirstCharMatch = 0; int isLevelOneSecondCharMatch = 0;
    int isLevelTwoFirstCharMatch = 0; int isLevelTwoSecondCharMatch = 0;
    int innerIndex = 0;

    for (int i = 0; i < ABC_NUMBER; i++)
      {
        for (int j = 0; j < ABC_NUMBER; j++)
        {       
          if (abcArray[i] == abcArray[j])
          {
            (*scoreMat)[i][j] = weights[0];  // $       
            continue;
          }
          
          // levelOneStringsMatch
          for (int k = 0; k < LEVEL_ONE_LENGTH; k++)
          {
            if (isLevelOneFirstCharMatch == 1 && isLevelOneSecondCharMatch == 1)
              break;

            innerIndex = 0;  
            isLevelOneFirstCharMatch = 0;
            isLevelOneSecondCharMatch = 0;
            
            while (levelOneStrings[k][innerIndex] != '\0')
            {
              if (abcArray[i] == levelOneStrings[k][innerIndex])
                isLevelOneFirstCharMatch = 1;
              if (abcArray[j] == levelOneStrings[k][innerIndex])
                isLevelOneSecondCharMatch = 1;
              innerIndex++;
            }
          }

          if (isLevelOneFirstCharMatch == 1 && isLevelOneSecondCharMatch == 1)
          {
            (*scoreMat)[i][j] = weights[1]; // % 
             isLevelOneFirstCharMatch = 0;
             isLevelOneSecondCharMatch = 0;
            continue;
          }

          // levelTwoStringsMatch
          for (int k = 0; k < LEVEL_TWO_LENGTH; k++)
          {
            if (isLevelTwoFirstCharMatch == 1 && isLevelTwoSecondCharMatch == 1)
              break;

            innerIndex = 0;  
            isLevelTwoFirstCharMatch = 0;
            isLevelTwoSecondCharMatch = 0;
            
            while (levelTwoStrings[k][innerIndex] != '\0')
            {
              if (abcArray[i] == levelTwoStrings[k][innerIndex])
                isLevelTwoFirstCharMatch = 1;
              if (abcArray[j] == levelTwoStrings[k][innerIndex])
                isLevelTwoSecondCharMatch = 1;
              innerIndex++;
            }
          }

          if (isLevelTwoFirstCharMatch == 1 && isLevelTwoSecondCharMatch == 1)
          {
            (*scoreMat)[i][j] = weights[2]; // # 
            isLevelTwoFirstCharMatch = 0;
            isLevelTwoSecondCharMatch = 0;		 
            continue;
          }
          else
          {
            (*scoreMat)[i][j] = weights[3]; // 
          }
        }
      }
}

int initScoreMat(int*** scoreMat)
{
  *scoreMat = (int**)malloc(sizeof(int*)* ABC_NUMBER);
    if(!*scoreMat)
    {
        fprintf(stderr,"Allocation error\n");
        return 0;
    }

    for(int i = 0; i < ABC_NUMBER; i++)
    {
        (*scoreMat)[i] = (int*)malloc(sizeof(int) * ABC_NUMBER);
        if(!((*scoreMat)[i]))
        {
            fprintf(stderr,"Allocation error\n");
            return 0;
        }
    }

    return 1;
}

int initScore(Score** topScore,int size)
{
    *topScore = (Score*)malloc(sizeof(Score)*size);
    if(!*topScore)
    {
        fprintf(stderr,"Allocation error\n");
        return 0;
    }

    memset(*topScore, 0, sizeof(Score)*size);
   
    return 1;
}

void calcSeq2Size(int* seq2ArrSize, int numProc,int myRank,int numOfSequences)
{
    if(numOfSequences >= numProc)
        *seq2ArrSize = numOfSequences/numProc;
    else if(myRank < numOfSequences)
        *seq2ArrSize = 1;
    else
        *seq2ArrSize = 0;
}

void scoreMatToArray(int** scoreMat, int* scoreArray)
{
    int k = 0;
    for (int i = 0; i < ABC_NUMBER; i++)
    {
        for (int j = 0; j < ABC_NUMBER; j++)
        {
            scoreArray[k] = scoreMat[i][j];
            k++;
        }
        
    }
}

void calcScoreAlgorithm(char* seq1,char* seq2,Score* topScore,int** scoreMat)
{
    int maxScore;
    int lenghtOfSeq2 = strlen(seq2);
    int offset = strlen(seq1) - lenghtOfSeq2;

    for (int k = 1; k <= lenghtOfSeq2; k++)
    {
        for (int n = 0; n < offset; n++)
        {
            for (int i = 0; i <  lenghtOfSeq2 ; i++)
            {
               if(i<k) 
                   topScore->scoreWeight+= scoreMat[seq1[i+n] - 'A'][seq2[i] - 'A'];
               else
                   topScore->scoreWeight+= scoreMat[seq1[i+n+1] - 'A'][seq2[i] - 'A'];                              
            }

            if (n == 0 && k == 1)
                maxScore = topScore->scoreWeight;
            
            if (maxScore <= topScore->scoreWeight)
            {
                topScore->n = n;
                topScore->k = k;
                maxScore = topScore->scoreWeight;
            }  

            topScore->scoreWeight = 0;             
        }      
    }
    topScore->scoreWeight = maxScore;
}

void printRes(Score* topScore,int workerArrSize)
{
    for(int i = 0; i < workerArrSize; i++)
        printf("N:%d K:%d\n", topScore[i].n, topScore[i].k);
}


void freeMemoryOfMaster(char** seq2Arr, char* seq1, Score* topScores, int size, Score* allScoresFromCuda, int* allScoresFromCudaBySize, int** scoreMat)
{
    for (int i = 0; i < size; i++)
        free(seq2Arr[i]);

    for (int i = 0; i < ABC_NUMBER; i++)
        free(scoreMat[i]);
  
    free(seq2Arr);
    free(scoreMat);
    free(seq1);
    free(topScores);
    free(allScoresFromCuda);
    free(allScoresFromCudaBySize);
}

void freeMemoryOfWorker(char** seq2Arr, char* seq1, Score* topScores, int size, int** scoreMat)
{
    for (int i = 0; i < size; i++)
        free(seq2Arr[i]);
    
    for (int i = 0; i < ABC_NUMBER; i++)
        free(scoreMat[i]);
      
    free(seq2Arr);
    free(scoreMat);
    free(seq1);
    free(topScores);
}

int allocateAllScores(Score** allScoresFromCuda, int** allScoresFromCudaBySize, int seq2ArrSize, char** seq2Arr, char* seq1)
{
    int finalSize = 0;
    int lenghtOfSeq1 = strlen(seq1);

    *allScoresFromCudaBySize = (int*)malloc(sizeof(int) * seq2ArrSize);
    if(!(*allScoresFromCudaBySize))
    {
        fprintf(stderr, "Allocation error\n");  
        return 0;
    }
   
    for (int i = 0; i < seq2ArrSize; i++)
    {
      (*allScoresFromCudaBySize)[i] = strlen(seq2Arr[i]) * (lenghtOfSeq1 - strlen(seq2Arr[i])); 
      finalSize += (*allScoresFromCudaBySize)[i]; 
    }

    *allScoresFromCuda = (Score*)malloc(sizeof(Score) * finalSize);
    if(!(*allScoresFromCuda))
    {
        fprintf(stderr, "Allocation error\n");  
        return 0;
    }  

    memset(*allScoresFromCuda, 0, sizeof(Score)*finalSize);
    
    return 1; 
}

void calcMaxScoreInSeq2Parallel(Score* allScoresFromCuda, int* allScoresFromCudaBySize, Score* topScores, int seq2ArrSize)
{   
      #pragma omp parallel for 
      for (int i = 0; i < seq2ArrSize; i++)
      { 
        int maxScore;
        int lowerLimit = 0; 
        int upperLimit = 0;      
        for (int j = 0; j < i; j++)
          lowerLimit += allScoresFromCudaBySize[j]; 
           
        maxScore = allScoresFromCuda[lowerLimit].scoreWeight; 
        assignMaxScore(&topScores[i], &allScoresFromCuda[lowerLimit]);
        upperLimit = lowerLimit + allScoresFromCudaBySize[i];
        lowerLimit++;
        for (; lowerLimit < upperLimit; lowerLimit++)
        {
             if (allScoresFromCuda[lowerLimit].scoreWeight >= maxScore)
             {      
                maxScore = allScoresFromCuda[lowerLimit].scoreWeight;
                assignMaxScore(&topScores[i], &allScoresFromCuda[lowerLimit]);  
             }    
        }  
        lowerLimit = 0;
        upperLimit = 0;        
      }        
}

void assignMaxScore(Score* topScore, Score* scoreToAssign)
{
   topScore->scoreWeight = scoreToAssign->scoreWeight;
   topScore->n = scoreToAssign->n;
   topScore->k = scoreToAssign->k;
}

