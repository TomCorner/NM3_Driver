#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include "maths.h"

using namespace std;

//**************************************************************************************************
//*** Class for storing serial messages and printing them to screen and/or file
//**************************************************************************************************

class Logger
{
public:
	Logger(char type);

	void Append(char* start);

	void Clear();

	void PrintToScreen();

	void PrintToFile();

private:
	vector<char> buf_;
	char filename_[50];

	void SetFileName(char type);

	void CreateTextFile();
};