# Parallel implementation of Sequence Alignment
**Final project**
**Course 10324, Parallel and Distributed Computation**

Sequence Alignment – a way to estimate a similarity of two strings of letters - is an important field in bioinformatics Sequence is a string of capital letters including hyphen sign (-)

## Problem Definition

Let Seq1 and Seq2 be a given Sequences of letters.
For all Mutant Sequences of Seq2 find an offset n which produce a maximum Alignment Score against Seq1. Among all results, choose the Mutant Sequence MS(k) with the best Alignment Score.

## Mutant Sequence Definition
For a given Sequence S we define a Mutant Sequence MS(n) which is received by insertion of hyphen (-) after n-th letter in S. For example, for a Sequence S = PSHLQY a set of 6 Mutant Sequences

![](https://i.ibb.co/G0qtTWS/sd.png)

>Mutant Sequence

For example, for Seq1 = PSHLQY and Seq2 = SHQ, the Mutant Sequence MS(2) = SH-Q produce the best Alignment Score at offset n = 1

## Input data
You will be provided with the input file input.txt with Weight Coefficients, Sequence Seq1 and few Sequences Seq2. 

####  Input File format

- First line - weight coefficients W1, W2, W3, W4
- Next line – Seq1 (not more than 3000 chars in line)
- Next line – the number NS2 of Sequences Seq2 to check against Seq1
- Next NS2 lines - Seq2 in each line (not more than 2000 chars in each line)


## Requirements
1. Implement the Simplified Sequence Alignment algorithm.
2. The input file input.txt initially is known for one machine only. The results must be written to console(stdout) on the same machine
3. The computation time of the parallel program must be faster than sequential solution. 
