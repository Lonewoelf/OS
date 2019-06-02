#include <iostream>
#include <math.h>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>

#include <Windows.h>
#include "ateC.h"


using namespace std;

string defaultPath = "C:\\Users\\Madita\\Documents\\";
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

void Ate::bassCoefficients(int intensity, double *b0, double *b1, double *b2, double *a1, double *a2)
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

	*a1 = -(-2.0*((a - 1) + (a + 1)*cos(w0))) / a0;
	*a2 = -((a + 1) + (a - 1)*cos(w0) - 2.0*sqrt(a)*alpha) / a0;
	*b0 = (a*((a + 1) - (a - 1)*cos(w0) + 2.0*sqrt(a)*alpha)) / a0;
	*b1 = (2 * a*((a - 1) - (a + 1)*cos(w0))) / a0;
	*b2 = (a*((a + 1) - (a - 1)*cos(w0) - 2.0*sqrt(a)*alpha)) / a0;
}

void Ate::trebleCoefficients(int intensity, double *b0, double *b1, double *b2, double *a1, double *a2)
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

	*a1 = -(2.0*((a - 1) - (a + 1)*cos(w0))) / a0;
	*a2 = -((a + 1) - (a - 1)*cos(w0) - 2.0*sqrt(a)*alpha) / a0;
	*b0 = (a*((a + 1) + (a - 1)*cos(w0) + 2.0*sqrt(a)*alpha)) / a0;
	*b1 = (-2.0*a*((a - 1) + (a + 1)*cos(w0))) / a0;
	*b2 = (a*((a + 1) + (a - 1)*cos(w0) - 2.0*sqrt(a)*alpha)) / a0;
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
	if (string(argv[1]) == "-p") {
		long num = strtol(argv[2], &p, 10); //convert argv[2] to long for comparing reasons
		if (num <= 8 && num >= 1) {
			cout << "max threads: " << argv[2]; //test
			maxThreads = strtol(argv[2], &p, 10); // set max threads to argv[2]
		}
		else {
			cout << num << " is not a valid argument for: " << argv[1] << endl;
		}
	}
	else {
		cout << "Bad syntax" << endl;
	}
	if (string(argv[3]) == "-b") {
		long num = strtol(argv[4], &p, 10);
		if (num <= 6 && num >= -6) {
			cout << "bass is now: " << argv[2]; //test
			bass = strtol(argv[4], &p, 10); //pass argv[2] to bass function
		}
		else {
			cout << num << " is not a valid argument for: " << argv[1] << endl;
		}
	}
	else {
		cout << "Bad syntax" << endl;
	}
	if (string(argv[5]) == "-t") {
		long num = strtol(argv[6], &p, 10);
		if (num <= 6 && num >= -6) {
			cout << "treble is now: " << argv[2]; //test
			treble = strtol(argv[6], &p, 10); //pass argv[2] to treble function
		}
		else {
			cout << num << " is not a valid argument for: " << argv[1] << endl;
		}
	}
	else {
		cout << "Bad syntax" << endl;
	}
	string path = defaultPath;
	path += string(argv[7]);
	ifstream myAudio(path, ios::in | ios::binary);

	if (checkFile(path)) {
		this->inputFile = path;
	}
	else {
		cout << "Bad syntax" << endl;
	}
	path = defaultPath + argv[8];
	if (checkFile(path)) {
		this->outputFile = path;
		divideIntoBlocks();
	}
	else {
		cout << "Bad syntax" << endl;
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
}

DWORD WINAPI Ate::bassFilter(LPVOID info)
{
	unique_lock<mutex> lck(mtx, defer_lock);
	//init size here
	Ate *ate = reinterpret_cast<Ate*>(info);

	unsigned size = ate->inputBlocks.size();

	for (int i = 0; i < size; i++)
	{
		if (i == 0)
		{
			//als er nog geen data in de data vector zit, moet deze eerst aan de hand van onderstaande formule worden ingevoegd
			ate->data.push_back(*ate->b0 * ate->inputBlocks.at(i).getSample());
		}
		if (i == 1)
		{
			//als er maar 1 data element in de vector staat moet onderstaande formule worden toegepast
			ate->data.push_back(*ate->b0 * ate->inputBlocks.at(i).getSample() + *ate->b1 * ate->inputBlocks.at(i - 1).getSample() + *ate->a1 * ate->data[i - 1]);
		}
		else
		{
			//nu zijn er genoeg gegevens in de data vector om de volledige formule toe te passen
			ate->data.push_back(*ate->b0 * ate->inputBlocks.at(i).getSample() + *ate->b1 * ate->inputBlocks.at(i).getSample() + *ate->b2 * ate->inputBlocks.at(i - 2).getSample() + *ate->a1 * ate->data[i - 1] + *ate->a2 * ate->data[i - 2]);
		}
		ate->inputBuff = move(ate->data);
	}
	lck.unlock();
	return 0;
}


DWORD WINAPI Ate::trebleFilter(LPVOID info) //Zo kan de functie wel aangeroepen worden vanuit de thread
{
	unique_lock<mutex> lck(mtx, defer_lock);
	//init size here
	Ate *ate = reinterpret_cast<Ate*>(info);

	unsigned size = ate->inputBlocks.size();

	for (int i = 0; i < size; i++)
	{
		if (i == 0)
		{
			//als er nog geen data in de data vector zit, moet deze eerst aan de hand van onderstaande formule worden ingevoegd
			ate->data.push_back(*ate->b0 * ate->inputBlocks.at(i).getSample());
		}
		if (i == 1)
		{
			//als er maar 1 data element in de vector staat moet onderstaande formule worden toegepast
			ate->data.push_back(*ate->b0 * ate->inputBlocks.at(i).getSample() + *ate->b1 * ate->inputBlocks.at(i - 1).getSample() + *ate->a1 * ate->data[i - 1]);
		}
		else
		{
			//nu zijn er genoeg gegevens in de data vector om de volledige formule toe te passen
			ate->data.push_back(*ate->b0 * ate->inputBlocks.at(i).getSample() + *ate->b1 * ate->inputBlocks.at(i).getSample() + *ate->b2 * ate->inputBlocks.at(i - 2).getSample() + *ate->a1 * ate->data[i - 1] + *ate->a2 * ate->data[i - 2]);
		}
		ate->inputBuff = move(ate->data);
	}
	lck.unlock();
	return 0;
}

void Ate::writeOutput()
{
	ofstream outputAudio;
	outputAudio.open(this->outputFile, ios::out | ios::binary);
	/*
	schrijf hier de data weg naar de output file

	unsigned size = inputBuff.size();
	for(int i = 0; i < size; i++)
	{
		outputAudio << inputBuff.at(i);
	}
	*/
	outputAudio.close();
}

void Ate::worker()
{
	for (int i = 0; i < getMaxThread(); i++)
	{
		Coefficients args = { b0, b1, b2, a1, a2 };
		CreateThread(0, 0, this->trebleFilter, &args, 0, NULL);
		CreateThread(0, 0, this->bassFilter, &args, 0, NULL); //Bass filter moet nog aangepast worden, zie trebleFilter
	}

}