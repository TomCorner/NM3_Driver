// NM3_Driver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define SYSTIMECLOCKHZ 1000000 // sys time clock at 1MHz
#define PROPTIMECOUNTHZ 16000 // propagation time is a 16kHz counter

#include <iostream>
#include "modem.h"
#include "maths.h"
#include "chirp.h"

using namespace std;

void Test() {
    //pass these in as parameters
    wchar_t COMportnum = L'4';   // check COM port in device manager
    char modemtype = 'A';                           // A for Alice, B for Bob
    unsigned epsilon = 5;                             // local ring down guard time in ms

    // these parameters are optional
    char chirpdurationindex = '0';                       // 0: 2ms, 1: 5ms, 2: 10ms, 3: 15ms, 4: 20ms, 5: 25ms, 6: 30ms    
    char chirpguard = '0';                               // multiple of guard time (5ms) between chirps
    char chirptype = 'U';

    //check modem type is either 'A/a' or 'B/b'
    // may need to allow for epsilon to be more than one char... check epsilon
    // if present check duration index and guard are between 0-9 ie 1 char

    Chirp chirpinfo(chirpdurationindex, chirpguard, chirptype);

    Modem alice(COMportnum);                 // configure alice

    unsigned bobaddress = 1;
    char messagebob[64];
    unsigned messagelength = 0;

    unsigned N = 5;

    // ****** enable systme time
    int systime = alice.SysTimeEnable();

    // ****** Ping bob
    int propagationtime = alice.Ping(bobaddress);

    if (propagationtime > 0) {
        double onewaytimems = CounterToMs(propagationtime, PROPTIMECOUNTHZ) / 2.0;
    }

    // ****** Ping bob

    // ****** Send Chirp info to bob
    if (chirptype == 'U' || chirptype == 'u') {
        messagelength = sprintf_s(messagebob, "%c%c%02u%c", 'D',chirpinfo.GetDurationChar(), N, chirpinfo.GetGuardChar());  // create message
    } else if (chirptype == 'D' || chirptype == 'd') {
        messagelength = sprintf_s(messagebob, "%c%c%02u%c", 'U', chirpinfo.GetDurationChar(), N, chirpinfo.GetGuardChar());  // create message
    } else {
        cout << "Error: invalid chirp type. Please enter 'U'/'u' for an up-chirp or 'D'/'d' for a down chirp." << "\n";
    }

    int err = alice.SendUnicast(bobaddress, messagebob, messagelength); // add time later...
    // ****** Send Chirp info to bob

    


}

void Prog() {
    //pass these in as parameters
    wchar_t COMportnum = L'3';   // check COM port in device manager
    char modemtype = 'A';                           // A for Alice, B for Bob
    char epsilon = '5';                             // local ring down guard time in ms

    // these parameters are optional
    char durationindex = '0';                       // 0: 2ms, 1: 5ms, 2: 10ms, 3: 15ms, 4: 20ms, 5: 25ms, 6: 30ms    
    char guard = '0';                               // multiple of guard time (5ms) between chirps

    //check modem type is either 'A/a' or 'B/b'
    // may need to allow for epsilon to be more than one char... check epsilon
    // if present check duration index and guard are between 0-9 ie 1 char

    Chirp chirpinfo(durationindex, guard, 'U');

    double test = CounterToMs(4127977, SYSTIMECLOCKHZ);
    int test1 = MsToCounter(test, SYSTIMECLOCKHZ);

    Modem alice(COMportnum);                 // configure alice
    int propagationtime = alice.Ping(0) / 2;    // retrieve propagation time to bob

    if (propagationtime > -1) {              // no errors


    }
}

int main()
{
    bool test = 1;

    if (test) {
        Test();
    }
    else {

    }
}
