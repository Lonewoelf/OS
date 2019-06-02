#pragma once
#include <string>
#include <Windows.h>
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

	static DWORD WINAPI bassFilter(LPVOID);
	static DWORD WINAPI trebleFilter(LPVOID);

	void computeInput(int argc, char *argv[]);
	void divideIntoBlocks();
	void worker();

private:
	int maxThreads, treble, bass;
	string inputFile, outputFile;
	signed short Blocks[1000];
	vector<signed short> inputBuff;
	vector<Block> inputBlocks;
	vector<signed short> data;
	signed short sample;
	double *b0, *b1, *b2;
	double *a1, *a2;	
};