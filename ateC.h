#pragma once
#include <string>
#include "block.h"
using namespace std;

class Ate {
public:
	Ate();
	virtual ~Ate();
	void bassCoefficients(int, double*, double*, double*, double*, double*);
	void trebleCoefficients(int, double*, double*, double*, double*, double*);
	void usage();

	void setMaxThread(int amount);
	void setTreble(int treble);
	int  getMaxThread();
	void setBass(int bass);
	void setInputFile(string path);
	void setOutputFile(string path);
	void writeOutput();

<<<<<<< HEAD
	unsigned short bassFilter(double*, double*, double*, double*, double* );
	DWORD WINAPI trebleFilter(double*, double*, double*, double*, double*);
=======
	void bassFilter(double*, double*, double*, double*, double*);
	void trebleFilter(double*, double*, double*, double*, double*);
>>>>>>> 6399b1cef5166f7bcf51ba658dcb64ad02ebb742

	void computeInput(int argc, char *argv[]);
	void divideIntoBlocks();
	void worker();

private:
	int maxThreads, treble, bass;
	string inputFile, outputFile;
	signed short Blocks[1000];
	vector<signed short> inputBuff;
	vector<Block> inputBlocks;
	vector<int16_t> data;
	signed short sample;
	double *b0, *b1, *b2;
	double *a1, *a2;	
};