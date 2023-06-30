#pragma once

#include "chirp.h"

unsigned CalculateChirpRepetitions(double tao, unsigned epsilon, Chirp chirpinfo);

double CounterToMs(unsigned int count, unsigned int clock);

unsigned MsToCounter(double ms, unsigned int clock);