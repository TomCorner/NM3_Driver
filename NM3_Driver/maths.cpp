#include "maths.h"

//**************************************************************************************************
//*** Function to calculate the number of chirps that can fit into a propagation delay
//**************************************************************************************************

unsigned CalculateChirpNumber(double tao, Chirp chirpinfo) {
	double epsilon = (double)chirpinfo.GetEpsilonVal();	// cast values to doubles before division
	double duration = (double)chirpinfo.GetDurationVal();
	double guard = (double)chirpinfo.GetGuardVal();
	return (int)((tao - epsilon) / (duration + guard));	// cast back to int as can only have an integer number of chirps
}

//**************************************************************************************************
//*** Function to convert an int counter of a given clock frequency to time in ms
//**************************************************************************************************

double CounterToMs(unsigned int count, unsigned int clock) {
	return ((double)count * (1.0 / (double)clock)) / 1000.0;
}

//**************************************************************************************************
//*** Function to convert a time in ms to an int counter of a given clock frequency
//**************************************************************************************************

unsigned MsToCounter(double ms, unsigned int clock) {
	return (int)(1000.0 * ms * (double)clock);
}