// NM3_Driver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define SYSTIMECLOCKHZ 1000000 // sys time clock at 1MHz
#define PROPTIMECOUNTHZ 16000 // propagation time is a 16kHz counter

const unsigned alicedelayms = 500;    // in ms
const unsigned bobdelayms = 1500;      // in ms

#include <iostream>
#include "modem.h"
#include "maths.h"
#include "chirp.h"
#include <chrono>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono;

int Alice() {
    //pass these in as parameters
    wchar_t COMportnum = L'3';   // check COM port in device manager
    char modemtype = 'A';                           // A for Alice, B for Bob
    unsigned epsilon = 5;                             // local ring down guard time in ms (default 5)

    // these parameters are optional
    char chirpdurationindex = '0';                       // duration index - see chirp.cpp for values (default 1)
    char chirpguardindex = '0';                          // guard index - see chirp.cpp for values    (default 1)
    char chirptype = 'U';                                // Alice up (U), bob down (D)

    Chirp chirpinfo(chirpdurationindex, chirpguardindex, chirptype);

    unsigned bobaddress = 2;            // Bobs address
    char messagebob[64];                // unicast configuration message to bob
    unsigned messagelength = 0;

    Modem alice(COMportnum);                 // configure alice
    char systimeflag = 'D';                  // 'E' for enabled or 'D' for disabled

    // ****** enable or clear system time 
    int  systime = alice.SysTimeGet(systimeflag);
    if (systime < 0) {
        return -1;
    }
    if (systimeflag == 'D') {
        systime = alice.SysTimeEnable(systimeflag);
    }
    else {
        systime = alice.SysTimeClear(systimeflag);
    }
    if (systime < 0) {
        return -1;
    }

    // ****** Ping bob
    int twowaytimecount = alice.Ping(bobaddress);
    if (twowaytimecount < 0) {
        return -1;
    }
    double onewaytimems = 0;
    if (twowaytimecount > 0) {
        onewaytimems = CounterToMs(twowaytimecount, PROPTIMECOUNTHZ) / 2.0;
    }
    unsigned nrepetitions = CalculateChirpRepetitions(onewaytimems, epsilon, chirpinfo);

    // ****** prepare Chirp message
    if (chirptype == 'U' || chirptype == 'u') {
        messagelength = sprintf_s(messagebob, "%c%c%02u%c", 'D', chirpinfo.GetDurationChar(), nrepetitions, chirpinfo.GetGuardChar());  // create message
    }
    else if (chirptype == 'D' || chirptype == 'd') {
        messagelength = sprintf_s(messagebob, "%c%c%02u%c", 'U', chirpinfo.GetDurationChar(), nrepetitions, chirpinfo.GetGuardChar());  // create message
    }
    else {
        cout << "Error: invalid chirp type. Please enter 'U'/'u' for an up-chirp or 'D'/'d' for a down chirp." << "\n";
    }

    // ****** Get system time and calculate unicast tx time    
    systime = alice.SysTimeGet(systimeflag);
    if (systime < 0) {
        return -1;
    }
    unsigned unicasttxtime = systime + MsToCounter(alicedelayms, SYSTIMECLOCKHZ);

    // ****** schedule chirp info message to bob
    int txdurationms = alice.UnicastWithAck(bobaddress, messagebob, messagelength); // add time later...
    if (txdurationms < 0) {
        return -1;
    }
    unsigned probetxtime = unicasttxtime + MsToCounter(onewaytimems, SYSTIMECLOCKHZ) + MsToCounter(bobdelayms + 30, SYSTIMECLOCKHZ);

    // ****** Schedule probe signal to bob    
    txdurationms = alice.Probe(nrepetitions, chirpinfo, probetxtime);
    if (txdurationms < 0) {
        return -1;
    }    
    std::this_thread::sleep_for(std::chrono::milliseconds(alicedelayms + txdurationms)); // wait for probe to be sent

    // ****** disable system time
    systime = alice.SysTimeDisable(systimeflag);
    if (systime < 0) {
        return -1;
    }
    return 0;
}

int Bob() {
    //pass these in as parameters
    wchar_t COMportnum = L'6';   // check COM port in device manager
    char modemtype = 'B';                           // A for Alice, B for Bob
    unsigned epsilon = 5;                             // local ring down guard time in ms (default 5)

    // these parameters are optional (Bob gets them from ALice)
    char chirpdurationindex = '1';                       // duration index - see chirp.cpp for values (default 1)
    char chirpguardindex = '1';                          // guard index - see chirp.cpp for values    (default 1)
    char chirptype = 'D';                                // Alice up (U), bob down (D)
    unsigned nrepetitions = 0;

    //Chirp chirpinfo(chirpdurationindex, chirpguardindex, chirptype);

    Modem bob(COMportnum);                 // configure alice
    char systimeflag = 'D';                  // 'E' for enabled or 'D' for disabled

    // ****** enable or clear system time 
    int  systime = bob.SysTimeGet(systimeflag);
    if (systime < 0) {
        return -1;
    }
    if (systimeflag == 'D') {
        systime = bob.SysTimeEnable(systimeflag);
    }
    else {
        systime = bob.SysTimeClear(systimeflag);
    }
    if (systime < 0) {
        return -1;
    }

    // ****** wait for chirp info from Alice and parse message
    char rxmessage[1000];
    systime = bob.UnicastListen(rxmessage);
    sscanf_s(rxmessage, "%c%c%2u%c", &chirptype, 1, &chirpdurationindex, 1, &nrepetitions, &chirpguardindex, 1);
    Chirp chirpinfo(chirpdurationindex, chirpguardindex, chirptype);

    // ****** Schedule probe signal to Alice
    unsigned probetxtime = systime + MsToCounter(bobdelayms, SYSTIMECLOCKHZ);

    //int testsystime = bob.SysTimeGet(systimeflag);
    int txdurationms = bob.Probe(nrepetitions, chirpinfo, probetxtime);
    if (txdurationms < 0) {
        return -1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(bobdelayms + txdurationms)); // wait for probe to be sent

    // ****** disable system time
    systime = bob.SysTimeDisable(systimeflag);
    if (systime < 0) {
        return -1;
    }
    return 0;
}

void Test() {
    steady_clock::time_point t1 = steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    steady_clock::time_point t2 = steady_clock::now();
    duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    double diff = time_span.count();

    int stop = 0;
}

int main()
{
    bool test = 0;

    if (test) {
        Test();
    }
    else {
        int err = Bob();
        if (err < 0) {
            std::cout << "\n**** Error in execution. See above messages for clarification ****\n";
        }
        else {
            std::cout << "\n**** Programme execution complete ****\n";
        }
    }
}
