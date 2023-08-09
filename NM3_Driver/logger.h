#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "maths.h"

using namespace std;

//**************************************************************************************************
//*** List of enums for error control and function declaration for printing a message
//**************************************************************************************************

enum ErrNum {
	ENoErr = 0,				// No errors
	ECOMErr = -1,			// Cannot open Modem COM port
	EGetDCBErr = -2,		// Cannot get DCB for Modem COM port
	ESetDCBErr = -3,		// Cannot set DCB for Modem COM port
	ENM3CommandErr = -4,	// Modem given unrecognised command
	ENM3TimeoutErr = -5,	// Timed out waiting for modem response
	ENM3UnexpectedErr = -6	// Modem received unexpected message
};

//**************************************************************************************************
//*** Class for storing serial messages and printing them to screen and/or file
//**************************************************************************************************

class Logger
{
public:
	Logger(char type);

	void Append(char* start);

	void AppendError(ErrNum err);

	void Clear();

	void PrintToScreen();

	void PrintToFile();

private:
	vector<char> buf_;
	char filename_[50];

	void SetFileName(char type);

	void CreateTextFile();

	string GetErrorMessage(ErrNum error);
};