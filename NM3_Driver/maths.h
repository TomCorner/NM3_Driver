#pragma once

#include "chirp.h"

uint16_t CalculateChirpRepetitions(double tao, uint32_t epsilon, Chirp chirpinfo);

double CounterToMs(uint64_t count, uint64_t clock);

uint64_t MsToCounter(double ms, uint64_t clock);