#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>

#include <Windows.h>
#include "ateC.h"


using namespace std;
//string defaultPath = "C:\\Users\\Madita\\Documents\\";
string defaultPath = "C:\\Users\\2125228\\Documents\\GitHub\\OS\\";
mutex mtx;

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

void Ate::bassCoefficients(int intensity)
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

	bassa1 = -(-2.0*((a - 1) + (a + 1)*cos(w0))) / a0;
	bassa2 = -((a + 1) + (a - 1)*cos(w0) - 2.0*sqrt(a)*alpha) / a0;
	bassb0 = (a*((a + 1) - (a - 1)*cos(w0) + 2.0*sqrt(a)*alpha)) / a0;
	bassb1 = (2 * a*((a - 1) - (a + 1)*cos(w0))) / a0;
	bassb2 = (a*((a + 1) - (a - 1)*cos(w0) - 2.0*sqrt(a)*alpha)) / a0;
}

void Ate::trebleCoefficients(int intensity)
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

	treblea1 = -(2.0*((a - 1) - (a + 1)*cos(w0))) / a0;
	treblea2 = -((a + 1) - (a - 1)*cos(w0) - 2.0*sqrt(a)*alpha) / a0;
	trebleb0 = (a*((a + 1) + (a - 1)*cos(w0) + 2.0*sqrt(a)*alpha)) / a0;
	trebleb1 = (-2.0*a*((a - 1) + (a + 1)*cos(w0))) / a0;
	trebleb2 = (a*((a + 1) + (a - 1)*cos(w0) - 2.0*sqrt(a)*alpha)) / a0;
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
	trebleCoefficients(this->treble);
}

void Ate::setBass(int bass)
{
	this->bass = bass;
	bassCoefficients(this->bass);
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
	if ((string(argv[1]) == "-p" || string(argv[1]) == "-b" || string(argv[1]) == "-t") && argc < 3) {
		cout << "Not enough arguments for option: " << argv[1] << endl;
		usage();
	}
	if (string(argv[1]) == "-p") {
		long num = strtol(argv[2], &p, 10); //convert argv[2] to long for comparing reasons
		if (num <= 8 && num >= 1) {
			cout << "max threads: " << argv[2] << endl; //test
			maxThreads = strtol(argv[2], &p, 10); // set max threads to argv[2]
		}
		else {
			cout << num << " is not a valid argument for: " << argv[1] << endl;
		}
	}
	else {
		cout << "Bad syntax 1" << endl;
	}
	if (string(argv[3]) == "-b") {
		long num = strtol(argv[4], &p, 10);
		if (num <= 6 && num >= -6) {
			cout << "bass is now: " << argv[4] << endl; //test
			bass = strtol(argv[4], &p, 10); //pass argv[2] to bass function
		}
		else {
			cout << num << " is not a valid argument for: " << argv[1] << endl;
		}
	}
	else {
		cout << "Bad syntax 2" << endl;
	}
	if (string(argv[5]) == "-t") {
		long num = strtol(argv[6], &p, 10);
		if (num <= 6 && num >= -6) {
			cout << "treble is now: " << argv[6] << endl; //test
			treble = strtol(argv[6], &p, 10); //pass argv[2] to treble function
		}
		else {
			cout << num << " is not a valid argument for: " << argv[1] << endl;
		}
	}
	else {
		cout << "Bad syntax 3" << endl;
	}
	string path = defaultPath;
	path += string(argv[7]);
	ifstream myAudio(path, ios::in | ios::binary);

	if (checkFile(path)) {
		this->inputFile = path;
	}
	else {
		cout << "Bad syntax 4" << endl;
	}
	path = defaultPath + argv[8];
	if (checkFile(path)) {
		this->outputFile = path;
		divideIntoBlocks();
	}
	else {
		cout << "Bad syntax 5" << endl;
	}
	myAudio.close();
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
	//worker();
	unsigned size = this->inputBlocks.size();
	for (int i = 0; i < size; i++) {
		data.push_back(inputBlocks.at(i).getSample());
	}
	this->inputBuff = move(this->data);
	this->writeOutput();
}

void Ate::trebleFilter(vector<signed short> inputBlock, vector<signed short>* outputBlock) //Zo kan de functie wel aangeroepen worden vanuit de thread
{
	int size = inputBlock.size();
	for (int i = 0; i < size; i++)
	{
		signed short sample = 0;
		if (i != 1 && i != 0)
		{
			sample = trebleb0 * inputBlock.at(i) + trebleb1 * inputBlock.at(i - 1) + trebleb2 * inputBlock.at(i - 2) + treblea1 * outputBlock->at(i - 1) + treblea2 * outputBlock->at(i - 2);
		}
		outputBlock->push_back(sample);
	}
}

void Ate::bassFilter(vector<signed short> inputBlock, vector<signed short>* outputBlock) //Zo kan de functie wel aangeroepen worden vanuit de thread
{
	int size = inputBlock.size();
	for (int i = 0; i < size; i++)
	{
		signed short sample = 0;
		if (i != 1 && i != 0)
		{
			sample = bassb0 * inputBlock.at(i) + bassb1 * inputBlock.at(i - 1) + bassb2 * inputBlock.at(i - 2) + bassa1 * outputBlock->at(i - 1) + bassa2 * outputBlock->at(i - 2);
		}
		outputBlock->push_back(sample);
	}
}

void Ate::writeOutput()
{
	cout << "writeOutput" << endl;
	ofstream outputFile;
	outputFile.open(this->outputFile, ios::out | ios::binary);
	if (outputFile.is_open())
	{
		cout << "File is opened" << endl;
		outputFile.write((char*)&inputBuff[0], this->inputBuff.size() * sizeof(signed short));
	}
	else
	{
		cout << "failed to open file" << endl;
	}
}

void Ate::worker()
{
	for (int i = 0; i < getMaxThread(); i++)
	{
		//Coefficients args = { b0, b1, b2, a1, a2 };
		//CreateThread(0, 0, trebleFilter, &args, 0, NULL);
		//CreateThread(0, 0, bassFilter, &args, 0, NULL); //Bass filter moet nog aangepast worden, zie trebleFilter
		writeOutput();
	}
}