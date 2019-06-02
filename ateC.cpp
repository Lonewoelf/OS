#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <thread>
#include "semaphore.h"
#include "ateC.h"
#include <Windows.h>

using namespace std;

string defaultPath = "C:\\Users\\Madita\\Documents\\";

Ate::Ate()
{
	//default == max
	this->inputFile = defaultPath + "input.pcm";
	this->outputFile = defaultPath + "output.pcm";
	this->maxThreads = 8;
	this->treble = 6;
	this->bass = 6;
}

Ate::~Ate()
{
}

struct Coefficients
{
	double* b0; double* b1; double* b2; double* a1; double* a2;
};

void Ate::bassCoefficients(int intensity, double* b0, double* b1, double* b2, double* a0, double* a1)
{
	double frequency = 330;     
	double qFactor = 0.5;     
	double gain = intensity;     
	double sampleRate = 44100;

	double pi = 4.0*atan(1);     
	double a = pow(10.0, gain / 40);     
	double w0 = 2 * pi*frequency / sampleRate;     
	double alpha = sin(w0) / (2.0*qFactor);
	double a0 = (a + 1) + (a - 1)*cos(w0) + 2.0*sqrt(a)*alpha;

	*a1 = -(-2.0 * ((a - 1) + (a + 1) * cos(w0))) / *a0;
	*a2 = -((a + 1) + (a - 1) * cos(w0) - 2.0 * sqrt(a) * alpha) / *a0;

	*b0 = (a * ((a + 1) - (a - 1) * cos(w0) + 2.0 * sqrt(a) * alpha)) / *a0;
	*b1 = (2 * a * ((a - 1) - (a + 1) * cos(w0))) / *a0;
	*b2 = (a * ((a + 1) - (a - 1) * cos(w0) - 2.0 * sqrt(a) * alpha)) / *a0;
}

void Ate::trebleCoefficients(int intensity, double* b0, double* b1, double* b2, double* a0, double* a1)
{
	double frequency = 3300;
	double qFactor = 0.5;
	double gain = intensity;
	double sampleRate = 44100;

	double pi = 4.0*atan(1);
	double a = pow(10.0, gain / 40);
	double w0 = 2 * pi*frequency / sampleRate;
	double alpha = sin(w0) / (2.0*qFactor);
	double a0 = (a + 1) - (a - 1)*cos(w0) + 2.0*sqrt(a)*alpha;

	*a1 = -(2.0*((a - 1) - (a + 1)*cos(w0))) / *a0;
	*a2 = -((a + 1) - (a - 1)*cos(w0) - 2.0*sqrt(a)*alpha) / *a0;

	*b0 = (a*((a + 1) + (a - 1)*cos(w0) + 2.0*sqrt(a)*alpha)) / *a0;
	*b1 = (-2.0*a*((a - 1) + (a + 1)*cos(w0))) / *a0;
	*b2 = (a*((a + 1) + (a - 1)*cos(w0) - 2.0*sqrt(a)*alpha)) / *a0;
}

void Ate::usage()
{ //cout behaves in a weird way, so it has to be like this to be printed correctly
	cout << endl;
	cout << "	 ____________________________________________________________________________________________________" << endl;
	cout << "	| Parameter       | Betekenis                                  | Geldige waarden   | Voorbeeld      |" << endl;
	cout << "	|---------------------------------------------------------------------------------------------------|" << endl;
	cout << "	| - p <n_o_t>     | Het aantal threads dat de applicatie       | integer 1 .. 8    | - p 4          |" << endl;
	cout << "	|                 | gebruikt.                                  |                   |                |" << endl;
	cout << "	|---------------------------------------------------------------------------------------------------|" << endl;
	cout << "	| - b <b_i>	  | De aanpassing van de intensiteit van de    | integer - 6 tot 6 | - b -3         |" << endl;
	cout << "	|                 | lage tonen in decibels(dB)                 |                   |                |" << endl;
	cout << "	|---------------------------------------------------------------------------------------------------|" << endl;
	cout << "	| - t <t_i>	  | De aanpassing van de intensiteit van de    | integer - 6 tot 6 | - t  4         |" << endl;
	cout << "	|                 | hoge tonen in decibels(dB)                 |                   |                |" << endl;
	cout << "	|---------------------------------------------------------------------------------------------------|" << endl;
	cout << "	| <input file>    | Het geluidsbestand waarop de	       | *.pcm		   | C:\\input.pcm   |" << endl;
	cout << "	|                 | toonregeling moet worden toegepast.        |                   |                |" << endl;
	cout << "	|                 |                                            |                   |                |" << endl;
	cout << "	|                 | Dit moet een raw pcm - bestand zijn met de |                   |                |" << endl;
	cout << "	|                 | volgende kenmerken :                       |                   |                |" << endl;
	cout << "	|                 |     * Sample rate = 44100                  |                   |                |" << endl;
	cout << "	|                 |     * Bitdepth = 16 bits                   |                   |                |" << endl;
	cout << "	|                 |     * Aantal kanalen = 1 (ofwel mono)      |                   |                |" << endl;
	cout << "	|---------------------------------------------------------------------------------------------------|" << endl;
	cout << " 	| <output file>   | Het uitvoerbestand met daarin de	       | *.pcm		   | C:\\output.pcm  |" << endl;
	cout << "	|                 | resultaten na equalizing.                  |                   |                |" << endl;
	cout << "	|___________________________________________________________________________________________________|" << endl;

}

void Ate::setMaxThread(int amount)
{
	this->maxThreads = amount;
}

int Ate::getMaxThread()
{
	return this->maxThreads;
}

void Ate::setTreble(int treble)
{
	this->treble = treble;
}

void Ate::setBass(int bass)
{
	this->bass = bass;
}

void Ate::setInputFile(string path)
{
	this->inputFile = path;
}

void Ate::setOutputFile(string path)
{
	this->outputFile = path;
}

bool checkFile(const std::string& name) {
	
	if (FILE *file = fopen(name.c_str(), "r")) {  //check if file exists
		if (name.substr(name.find_last_of(".") + 1) == "pcm") { //check if file has the right extension
			return true;
		}
		fclose(file);
	}
	else {
		return false;
	}
	return true;
}

void Ate::computeInput(int argc, char * argv[])
{
	/*
		argc is amount of arguments
		argv[0] is program that is called (in this case ate)
		argv[1] is the option given
		argv[2] is the option argument given

		string(argv[1]) is used so I can compare argv[1] to a string
	*/
	char *p;
	if (argc <= 1) {
		cout << "No arguments given" << endl;
		usage();
	}
	if ((string(argv[1]) == "-p" || string(argv[1]) == "-b" || string(argv[1]) == "-t") && argc != 3) {
		cout << "Not enough arguments for option: " << argv[1] << endl;
		usage();
	}
	else if (string(argv[1]) == "-p") {
		long num = strtol(argv[2], &p, 10); //convert argv[2] to long for comparing reasons
		if (num <= 8 && num >= 1) {
			cout << "max threads: " << argv[2]; //test
			// set max threads to argv[2]
		}
		else {
			cout << num << " is not a valid argument for: " << argv[1] << endl;
		}
	}
	else if (string(argv[1]) == "-b") {
		long num = strtol(argv[2], &p, 10);
		if (num <= 6 && num >= -6) {
			cout << "bass is now: " << argv[2]; //test
			//pass argv[2] to bass function
		}
		else {
			cout << num << " is not a valid argument for: " << argv[1] << endl;
		}
	}
	else if (string(argv[1]) == "-t") {
		long num = strtol(argv[2], &p, 10);
		if (num <= 6 && num >= -6) {
			cout << "treble is now: " << argv[2]; //test
			//pass argv[2] to treble function
		}
		else {
			cout << num << " is not a valid argument for: " << argv[1] << endl;
		}
	}
	else {
		string path = defaultPath;
		path += string(argv[1]);
		ifstream myAudio(path, ios::in | ios::binary);

		if (checkFile(path)) {
			
			if (myAudio.peek() == EOF) {
				cout << argv[1] << " Recognized as output file";
			}
			else {
				cout << argv[1] << " Recognized as input file";

				divideIntoBlocks();
			}
			myAudio.close();
		}
		else {
			cout << argv[1] << " is not a valid option" << endl;
		}
	}
}



void Ate::divideIntoBlocks()
{
	ifstream myAudio(this->inputFile, ios::in | ios::binary);
	int i = 0;

	while (myAudio.read((char*)& sample, sizeof(signed short))) {
		Block b(sample, i);
		inputBlocks.push_back(b);
		i++;
	}
	myAudio.close();
}

void Ate::bassFilter(LPVOID pArgs)
{
	Coefficients *pArgs = (Coefficients*)pArgs;
	//init size here
	unsigned size = inputBlocks.size();
	
	for (int i = 0; i < size; i++)
	{
		if (i == 0)
		{
			//als er nog geen data in de data vector zit, moet deze eerst aan de hand van onderstaande formule worden ingevoegd
			data.push_back(*b0 * inputBlocks[i]);
		}
		if (i == 1)
		{
			//als er maar 1 data element in de vector staat moet onderstaande formule worden toegepast
			data.push_back(*b0 * inputBlocks[i] + *b1 * this->inputBlocks[i - 1] + *a1 * data[i - 1]);
		}
		else
		{
			//nu zijn er genoeg gegevens in de data vector om de volledige formule toe te passen
			data.push_back(*b0 * this->inputBlocks[i] + *b1 * this->inputBlocks[i - 1] + *b2 * this->inputBlocks[i - 2] + *a1 * data[i - 1] + *a2 * data[i - 2]);
		}
		this->inputBuff = move(data);
	}
}

void Ate::trebleFilter(LPVOID pArgs)
{
	Coefficients *pArgs = (Coefficients*)pArgs;
	//init size here
	unsigned size = inputBlocks.size();

	for (int i = 0; i < size; i++)
	{
		if (i == 0)
		{
			//als er nog geen data in de data vector zit, moet deze eerst aan de hand van onderstaande formule worden ingevoegd
			data.push_back(*b0 * inputBlocks[i]);
		}
		if (i == 1)
		{
			//als er maar 1 data element in de vector staat moet onderstaande formule worden toegepast
			data.push_back(*b0 * inputBlocks[i] + *b1 * this->inputBlocks[i - 1] + *a1 * data[i - 1]);
		}
		else
		{
			//nu zijn er genoeg gegevens in de data vector om de volledige formule toe te passen
			data.push_back(*b0 * this->inputBlocks[i] + *b1 * this->inputBlocks[i - 1] + *b2 * this->inputBlocks[i - 2] + *a1 * data[i - 1] + *a2 * data[i - 2]);
		}
		this->inputBuff = move(data);
	}
}

void Ate::writeOutput()
{
	ofstream outputAudio;
	outputAudio.open(this->outputFile, ios::out | ios::binary);
	/*
	schrijf hier de data weg naar de output file
	outputAudio << 
	*/
	outputAudio.close();

}

void Ate::worker()
{
	for (int i = 0; i < maxThreads; i++)
	{
		Coefficients args = { b0, b1, b2, a1, a2 };
		CreateThread(NULL, 0, this->bassFilter, &args, 0, NULL);
		CreateThread(NULL, 0, this->trebleFilter, &args, 0, NULL);
	}
	

}