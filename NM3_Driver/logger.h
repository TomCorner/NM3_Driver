#pragma once

#include <vector>
#include <iostream>

using namespace std;

//**************************************************************************************************
//*** Class for storing serial messages and printing them to screen and/or file
//**************************************************************************************************

class Logger
{
public:
	void Append(char* start);

	void Clear();

	void PrintToScreen();

	void PrintToFile();

private:
	vector<char> buf_;
};