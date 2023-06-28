// NM3_Driver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define SYSTIMECLOCKHZ 1000000 // sys time clock at 1MHz
#define PROPTIMECOUNTHZ 16000 // propagation time is a 16kHz counter

#include <iostream>
#include "modem.h"
#include "maths.h"
#include "chirp.h"

using namespace std;

int main()
{
    //pass these in as parameters
    wchar_t COMport[4] = { L'C',L'O',L'M',L'3' };   // check COM port in device manager
    char modemtype = 'A';                           // A for Alice, B for Bob
    char epsilon = '5';                             // local ring down guard time in ms

    // these parameters are optional
    char durationindex = '0';                       // 0: 2ms, 1: 5ms, 2: 10ms, 3: 15ms, 4: 20ms, 5: 25ms, 6: 30ms    
    char guard = '0';                               // multiple of guard time (5ms) between chirps

    //check modem type is either 'A/a' or 'B/b'
    // may need to allow for epsilon to be more than one char... check epsilon
    // if present check duration index and guard are between 0-9 ie 1 char

    Chirp chirpinfo(durationindex, epsilon, guard);

    double test = CounterToMs(4127977, SYSTIMECLOCKHZ);
    int test1 = MsToCounter(test, SYSTIMECLOCKHZ);

    Modem alice(COMport, 9600);                 // configure alice
    int propagationtime = alice.Ping(0) / 2;    // retrieve propagation time to bob

    if (propagationtime > -1) {              // no errors


    }
}
