#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <iostream>
#include "cFunctions.h"
#define MAX_THREADS 1024

                        
__global__ void calcScore(Score* deviceAllScores, char* deviceSeq1, char* deviceSeq2, int lenghtOfSeq2, int threadsInBlock, int allScoresOffset, int* deviceScoreMat)
{     
    int i,j;
    int count = 0; 
    int loc = 0;
    int scoreIndex = blockIdx.y * gridDim.x + blockIdx.x + allScoresOffset; 
    extern __shared__ int shared_arr[];
    
    while (loc + threadIdx.x < lenghtOfSeq2)
    {     	            		   
        j = deviceSeq2[threadIdx.x+loc] - 'A';
        if (threadIdx.x+loc >= blockIdx.y + 1) 
            i = deviceSeq1[threadIdx.x+loc+blockIdx.x+1] - 'A';
        else
            i = deviceSeq1[threadIdx.x+loc+blockIdx.x] - 'A';
        shared_arr[threadIdx.x+loc] = deviceScoreMat[i*ABC_NUMBER + j];
        count++;
        loc = count*threadsInBlock;          
    }  
    
    __syncthreads();

    if(threadIdx.x == 0) 
    {
        deviceAllScores[scoreIndex].k = blockIdx.y + 1; 
        deviceAllScores[scoreIndex].n = blockIdx.x; 
        for (int i = 0; i < lenghtOfSeq2; i++)
            deviceAllScores[scoreIndex].scoreWeight+= shared_arr[i];              
    }
    
}

int checkStatus(cudaError_t cudaStatus, int* deviceScoreMat, char* deviceSeq1, char* deviceSeq2, score* deviceAllScores, std::string err)
{
    if(cudaStatus != cudaSuccess)
    {
        std::cout << err <<std::endl;

        free(deviceScoreMat);
        free(deviceSeq1);        
        free(deviceSeq2);    
        free(deviceAllScores);
            
        return 0;
    }
    return 1; 
}


int initCudaCalcs(char** seq2Arr, int seq2ArrSize, score* allScoresFromCuda, int* allScoresFromCudaBySize, char* seq1, int** scoreMat)
{
    cudaError_t cudaStatus;
    int* deviceScoreMat = 0;
    char* deviceSeq1 = 0;  
    char* deviceSeq2 = 0; 
    int lenghtOfSeq1, lenghtOfSeq2;
    Score* deviceAllScores = 0;
    int allScoresOffset = 0;
    int threadsInBlock = 0;
    int* scoreArr = (int*)malloc(sizeof(int)* ABC_NUMBER * ABC_NUMBER); 
    scoreMatToArray(scoreMat, scoreArr);
    
    cudaStatus = cudaMalloc((void**)&deviceScoreMat, ABC_NUMBER * ABC_NUMBER * sizeof(int));
    if(!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaMalloc have failed!"))
        return 0;
    cudaStatus = cudaMemcpy(deviceScoreMat, scoreArr, ABC_NUMBER * ABC_NUMBER * sizeof(int), cudaMemcpyHostToDevice);
    if(!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaMemcpy have failed!"))
        return 0; 
 
    int allSize = 0;
    for (int i = 0; i < seq2ArrSize; i++)
        allSize += allScoresFromCudaBySize[i];
    cudaStatus = cudaMalloc((void**)&deviceAllScores, sizeof(score) * allSize);
    if(!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaMalloc have failed!"))
        return 0;
    cudaStatus = cudaMemcpy(deviceAllScores, allScoresFromCuda, sizeof(score) * allSize, cudaMemcpyHostToDevice);
    if(!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaMemcpy have failed!"))
        return 0;
     
    lenghtOfSeq1 = strlen(seq1);
    cudaStatus = cudaMalloc((void**)&deviceSeq1, sizeof(char)* (lenghtOfSeq1 + 1));
    if(!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaMalloc have failed!"))
        return 0;
    cudaStatus = cudaMemcpy(deviceSeq1, seq1, sizeof(char)* (lenghtOfSeq1 + 1), cudaMemcpyHostToDevice);
    if(!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaMemcpy have failed!"))
        return 0;
      
    for (int i = 0; i < seq2ArrSize; i++)
    {
        lenghtOfSeq2 = strlen(seq2Arr[i]); 
        cudaStatus = cudaMalloc((void**)&deviceSeq2, sizeof(char)* (lenghtOfSeq2 + 1));
        if(!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaMalloc have failed!"))
            return 0;  
        cudaStatus = cudaMemcpy(deviceSeq2, seq2Arr[i], sizeof(char)* (lenghtOfSeq2 + 1), cudaMemcpyHostToDevice);
        if(!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaMemcpy have failed!"))
            return 0;
         
        dim3 numBlocks (lenghtOfSeq1 - lenghtOfSeq2, lenghtOfSeq2);
        if (MAX_THREADS < lenghtOfSeq2)
            threadsInBlock = MAX_THREADS;
        else
            threadsInBlock = lenghtOfSeq2;
    
        calcScore<<<numBlocks,threadsInBlock,lenghtOfSeq2*sizeof(int)>>>(deviceAllScores, deviceSeq1, deviceSeq2, lenghtOfSeq2, threadsInBlock, allScoresOffset, deviceScoreMat);
        cudaStatus = cudaDeviceSynchronize();
        if(!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "calculateAlignmentScores have failed!"))
            return 0;
        
        allScoresOffset += allScoresFromCudaBySize[i];
    }
    
    cudaStatus = cudaMemcpy(allScoresFromCuda, deviceAllScores, sizeof(score) * allSize, cudaMemcpyDeviceToHost);
    if(!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaMemcpy have failed!"))
        return 0;
  
    cudaStatus = cudaFree(deviceSeq1);
    if (!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaFree have failed!"))
        return 0;    
    cudaStatus = cudaFree(deviceSeq2);
    if (!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaFree have failed!"))
        return 0;   
    cudaStatus = cudaFree(deviceAllScores);
    if (!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaFree have failed!"))
        return 0;
     cudaStatus = cudaFree(deviceScoreMat);
    if (!checkStatus(cudaStatus, deviceScoreMat, deviceSeq1, deviceSeq2, deviceAllScores, "cudaFree have failed!"))
       return 0;    
 
    return 1; 
}


