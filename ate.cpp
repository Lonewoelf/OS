// ate.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//ate is the main because of the assignment

//Paths staan nu in de nodige functies


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <thread>
#include "ateC.h"

using namespace std;

int main(int argc, char *argv[])
{
	Ate music;
	music.computeInput(argc, argv);
	
	

	cin.get();
	return 0;

	//ifstream myAudio("input.pcm", ios::in | ios::binary);

	//vector<signed short> input;
	//signed short sample;

	//while (myAudio.read((char*)& sample, sizeof(signed short)))
	//{
	//	input.push_back(sample);
	//}
}