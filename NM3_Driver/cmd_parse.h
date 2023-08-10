#pragma once

#include <iostream>
#include <stdint.h>

using namespace std;

int16_t ArgumentCheck(int argc, char** argv);

int16_t ParseType(char** argv, char& type);

int16_t ParsePortNumber(char** argv, wchar_t& port_number);

int16_t ParseEpsilon(char** argv, uint32_t& epsilon);

int16_t ParseChirpDuration(char** argv, char& chirp_duration_index);

int16_t ParseChirpGuard(char** argv, char& chirp_guard_index);