#pragma once
#include <string>
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
	void setBass(int bass);
	void setInputFile(string path);
	void setOutputFile(string path);

	void computeInput(int argc, char *argv[]);
	void divideIntoBlocks();

private:
	int maxThreads, treble, bass;
	string inputFile, outputFile;
	signed short Blocks[1000];
};