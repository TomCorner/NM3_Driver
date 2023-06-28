#pragma once

class Chirp
{
public:
    Chirp(char duration, char epsilon, char guard);

    char GetDurationChar();

    char GetGuardChar();

    unsigned short GetDurationVal();

    unsigned short GetEpsilonVal();

    unsigned short GetGuardVal();
private:
    char durationindex_;    // chirp duration index (ms) (0=2ms,1=5ms,2=10ms,3=20ms,4=30ms,5=40ms,6=50ms)
    char epsilon_;     // local ringdown guard (ms)
    char guardmultiple_;       // guard between chirps, multiple of 5ms (ms) eg 0=0ms,1=5ms,2=20ms etc...
};