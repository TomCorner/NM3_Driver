#include "maths.h"

//**************************************************************************************************
//*** Function to calculate the number of chirps that can fit into a propagation delay
//**************************************************************************************************

uint16_t CalculateChirpRepetitions(double tao, uint32_t epsilon, Chirp chirpinfo) {
	double eps = (double)epsilon;	// cast values to doubles before division
	double duration = (double)chirpinfo.GetDurationVal();
	double guard = (double)chirpinfo.GetGuardVal();
	int16_t n_repititions = (uint16_t)((tao - eps - duration) / (duration + guard)); // calculate number of repitions using decided formula
	if (n_repititions < 0) {
		return 0;
	}
	else if (n_repititions > 99) {
		return 99;
	}
	else {
		return n_repititions;
	}
}

//**************************************************************************************************
//*** Function to convert an int64_t counter of a given clock frequency to time in ms
//**************************************************************************************************

double CounterToMs(uint64_t count, uint64_t clock) {
	return ((double)count * (1.0 / (double)clock)) * 1000.0;
}

//**************************************************************************************************
//*** Function to convert a time in ms to an int64_t counter of a given clock frequency
//**************************************************************************************************

uint64_t MsToCounter(double ms, uint64_t clock) {
	return (int)((ms * (double)clock) / 1000.0);
}