#pragma once
#include <string>
#include <Windows.h>
#include "block.h"
using namespace std;

class Ate {
public:
	Ate();
	virtual ~Ate();
	void bassCoefficients(int);
	void trebleCoefficients(int);
	void usage();

	void setMaxThread(int amount);
	void setTreble(int treble);
	int  getMaxThread();
	void setBass(int bass);
	void setInputFile(string path);
	void setOutputFile(string path);
	void writeOutput();

	void bassFilter(vector<signed short> inputBlock, vector<signed short>* outputBlock);
	void trebleFilter(vector<signed short> inputBlock, vector<signed short>* outputBlock);

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
	double bassb0, bassb1, bassb2, bassa1, bassa2;
	double trebleb0, trebleb1, trebleb2, treblea1, treblea2;	
};