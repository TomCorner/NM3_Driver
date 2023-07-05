// NM3_Driver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define SYSTIMECLOCKHZ 1000000 // sys time clock at 1MHz
#define PROPTIMECOUNTHZ 16000 // propagation time is a 16kHz counter

const unsigned alicedelayms = 250;    // in ms
const unsigned bobdelayms = 500;      // in ms

#include <iostream>
#include "modem.h"
#include "maths.h"
#include "chirp.h"
#include <chrono>
#include <thread>

using namespace std;

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
    int propagationtime = alice.Ping(bobaddress);
    if (propagationtime < 0) {
        return -1;
    }
    double onewaytimems = 0;
    if (propagationtime > 0) {
        onewaytimems = CounterToMs(propagationtime, PROPTIMECOUNTHZ) / 2.0;
    }
    unsigned nrepetitions = CalculateChirpRepetitions(onewaytimems, epsilon, chirpinfo);

    // ****** Get system time and calculate unicast tx time    
    systime = alice.SysTimeGet(systimeflag);
    if (systime < 0) {
        return -1;
    }
    unsigned unitxtime = systime + MsToCounter(alicedelayms, SYSTIMECLOCKHZ);

    // ****** Send Chirp info to bob
    if (chirptype == 'U' || chirptype == 'u') {
        messagelength = sprintf_s(messagebob, "%c%c%02u%c", 'D', chirpinfo.GetDurationChar(), nrepetitions, chirpinfo.GetGuardChar());  // create message
    }
    else if (chirptype == 'D' || chirptype == 'd') {
        messagelength = sprintf_s(messagebob, "%c%c%02u%c", 'U', chirpinfo.GetDurationChar(), nrepetitions, chirpinfo.GetGuardChar());  // create message
    }
    else {
        cout << "Error: invalid chirp type. Please enter 'U'/'u' for an up-chirp or 'D'/'d' for a down chirp." << "\n";
    }
    int txdurationms = alice.SendUnicast(bobaddress, messagebob, messagelength, unitxtime); // add time later...
    if (txdurationms < 0) {
        return -1;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(alicedelayms + txdurationms)); // wait for acosutic message to be sent

    // ****** Schedule probe signal to bob
    unsigned probetxtime = unitxtime + MsToCounter(onewaytimems, SYSTIMECLOCKHZ) + MsToCounter(bobdelayms, SYSTIMECLOCKHZ) + MsToCounter(txdurationms, SYSTIMECLOCKHZ);
    txdurationms = alice.Probe(nrepetitions, chirpinfo, probetxtime);
    if (txdurationms < 0) {
        return -1;
    }    
    std::this_thread::sleep_for(std::chrono::milliseconds((unsigned)(onewaytimems + 0.5) + bobdelayms + txdurationms)); // wait for probe to be sent

    // ****** disable system time
    systime = alice.SysTimeDisable(systimeflag);
    if (systime < 0) {
        return -1;
    }
    return 0;
}

int Bob() {
    //pass these in as parameters
    wchar_t COMportnum = L'3';   // check COM port in device manager
    char modemtype = 'A';                           // A for Alice, B for Bob
    unsigned epsilon = 5;                             // local ring down guard time in ms (default 5)

    // these parameters are optional (Bob gets them from ALice)
    //char chirpdurationindex = '0';                       // duration index - see chirp.cpp for values (default 1)
    //char chirpguardindex = '0';                          // guard index - see chirp.cpp for values    (default 1)
    //char chirptype = 'U';                                // Alice up (U), bob down (D)

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

    char rxmessage[1000];
    systime = bob.UnicastListen(rxmessage);

    // ****** disable system time
    systime = bob.SysTimeDisable(systimeflag);
    if (systime < 0) {
        return -1;
    }
    return 0;
}

void Test() {
  
    


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
