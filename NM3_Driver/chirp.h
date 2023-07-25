#pragma once

#include <stdint.h>

class Chirp
{
public:
	Chirp(char duration, char guard, char type);

	char GetDurationChar();

	char GetGuardChar();

	char GetType();

	uint16_t GetDurationVal();

	uint16_t GetGuardVal();
private:
	char durationindex_;    // chirp duration index (ms) (0=2ms,1=5ms,2=10ms,3=20ms,4=30ms,5=40ms,6=50ms)
	char guard_;       // guard between chirps, multiple of 5ms (ms) eg 0=0ms,1=5ms,2=20ms etc...
	char type_;
};