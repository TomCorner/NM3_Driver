#pragma once

#include "chirp.h"

unsigned CalculateChirpNumber(double tao, Chirp chirpinfo);

double CounterToMs(unsigned int count, unsigned int clock);

unsigned MsToCounter(double ms, unsigned int clock);