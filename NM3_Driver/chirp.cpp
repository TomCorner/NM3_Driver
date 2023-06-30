#include "chirp.h"


//**************************************************************************************************
//*** Chirp class constructor
//**************************************************************************************************

Chirp::Chirp(char duration, char guard, char type): durationindex_(duration),
													guard_(guard),
													type_(type){}

//**************************************************************************************************
//*** Function to return the char value for duration index
//**************************************************************************************************

char Chirp::GetDurationChar() { return durationindex_; }

//**************************************************************************************************
//*** Function to return the char value for fuard multiple
//**************************************************************************************************

char Chirp::GetGuardChar() { return guard_; }

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
	case '7':
		duration = 100;
		break;
	case '8':
		duration = 200;
		break;
	case '9':
		duration = 200;
		break;
	default:
		duration = 2;
		break;
	}
	return duration;
}

//**************************************************************************************************
//*** Function to return the guard time in ms
//**************************************************************************************************

unsigned short Chirp::GetGuardVal() {
	unsigned short guard;
	switch (guard_)
	{
	case '0':
		guard = 0;
		break;
	case '1':
		guard = 5;
		break;
	case '2':
		guard = 10;
		break;
	case '3':
		guard = 20;
		break;
	case '4':
		guard = 30;
		break;
	case '5':
		guard = 40;
		break;
	case '6':
		guard = 50;
		break;
	case '7':
		guard = 100;
		break;
	case '8':
		guard = 200;
		break;
	case '9':
		guard = 500;
		break;
	default:
		guard = 5;
		break;
	}
	return guard;
}