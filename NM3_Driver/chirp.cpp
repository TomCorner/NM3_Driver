#include "chirp.h"


//**************************************************************************************************
//*** Chirp class constructor
//**************************************************************************************************

Chirp::Chirp(char duration, char epsilon, char guard, char type): durationindex_(duration),  
														epsilon_(epsilon),
														guardmultiple_(guard),
														type_(type){}

//**************************************************************************************************
//*** Function to return the char value for duration index
//**************************************************************************************************

char Chirp::GetDurationChar() { return durationindex_; }

//**************************************************************************************************
//*** Function to return the char value for fuard multiple
//**************************************************************************************************

char Chirp::GetGuardChar() { return guardmultiple_; }

//**************************************************************************************************
//*** Function to return the chirp type (U/D)
//**************************************************************************************************

char Chirp::GetType() { return type_; }

//**************************************************************************************************
//*** Function to return the duration in ms
//**************************************************************************************************

unsigned short Chirp::GetDurationVal() {
	unsigned short duration;
	switch (durationindex_)
	{
	case '0':
		duration = 2;
		break;
	case '1':
		duration = 5;
		break;
	case '2':
		duration = 10;
		break;
	case '3':
		duration = 20;
		break;
	case '4':
		duration = 30;
		break;
	case '5':
		duration = 40;
		break;
	case '6':
		duration = 50;
		break;
	default:
		duration = 2;
		break;
	}
	return duration;
}

//**************************************************************************************************
//*** Function to return the epsilon in ms
//**************************************************************************************************

unsigned short Chirp::GetEpsilonVal() { return epsilon_ - 48; }	// convert epsilon to int

//**************************************************************************************************
//*** Function to return the guard time in ms
//**************************************************************************************************

unsigned short Chirp::GetGuardVal() {
	unsigned short guardmultiple = guardmultiple_ - 48;	// convert multiple to int
	return guardmultiple * 5;							// multiply by predetermined guard time (5ms)
}