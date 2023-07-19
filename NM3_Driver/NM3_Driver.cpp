// NM3_Driver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define SYSTIMECLOCKHZ 1000000 // sys time clock at 1MHz
#define PROPTIMECOUNTHZ 16000 // propagation time is a 16kHz counter

#define ALICEDELAYMS 100    // in ms
#define BOBDELAYMS 1500     // in ms

#include "modem.h"
#include "maths.h"
#include "chirp.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono;

int64_t Alice() {
	//pass these in as parameters
	wchar_t COMportnum = L'3';   // check COM port in device manager
	char modemtype = 'A';                           // A for Alice, B for Bob
	uint16_t epsilon = 5;                             // local ring down guard time in ms (default 5)

	// these parameters are optional
	char chirpdurationindex = '0';                       // duration index - see chirp.cpp for values (default 1)
	char chirpguardindex = '0';                          // guard index - see chirp.cpp for values    (default 1)
	char chirptype = 'U';                                // Alice up (U), bob down (D)

	Chirp chirpinfo(chirpdurationindex, chirpguardindex, chirptype);

	uint16_t bobaddress = 2;            // Bobs address
	char messagebob[64];                // unicast configuration message to bob
	uint16_t messagelength = 0;

	Modem alice(COMportnum);                 // configure alice
	char systimeflag = 'D';                  // 'E' for enabled or 'D' for disabled

	// ****** enable or clear system time 
	int64_t systime = alice.SysTimeEnable(systimeflag);
	if (systime < 0) {
		alice.PrintLogs();
		return systime;
	}

	// ****** Ping bob
	int64_t twowaytimecount = alice.Ping(bobaddress);
	if (twowaytimecount < 0) return twowaytimecount;
	double onewaytimems = CounterToMs(twowaytimecount, PROPTIMECOUNTHZ) / 2.0;
	uint16_t nrepetitions = CalculateChirpRepetitions(onewaytimems, epsilon, chirpinfo);

	// ****** prepare Chirp message
	if (chirptype == 'U' || chirptype == 'u') {	// TODO - Add this to command line parameter checks as well
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
		alice.PrintLogs();
		return systime;
	}
	uint64_t unicasttxtime = systime + MsToCounter(ALICEDELAYMS, SYSTIMECLOCKHZ);

	// ****** schedule chirp info message to bob
	int64_t txdurationms = alice.UnicastWithAck(bobaddress, messagebob, messagelength); // add time later...
	if (txdurationms < 0) {
		alice.PrintLogs();
		return txdurationms;
	}
	uint64_t probetxtime = unicasttxtime + MsToCounter(onewaytimems, SYSTIMECLOCKHZ) + MsToCounter(BOBDELAYMS + 30, SYSTIMECLOCKHZ);

	// ****** Schedule probe signal to bob    
	txdurationms = alice.Probe(nrepetitions, chirpinfo, probetxtime);
	if (txdurationms < 0) {
		alice.PrintLogs();
		return txdurationms;
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(uint64_t(CounterToMs(probetxtime - unicasttxtime, SYSTIMECLOCKHZ)) + txdurationms)); // wait for probe to be sent
	this_thread::sleep_for(milliseconds(BOBDELAYMS + txdurationms)); // wait for probe to be sent

	// ****** disable system time
	systime = alice.SysTimeDisable(systimeflag);
	alice.PrintLogs();
	if (systime < 0) return systime;	

	return ENoErr;
}

int64_t Bob() {
	//pass these in as parameters
	wchar_t COMportnum = L'6';   // check COM port in device manager
	char modemtype = 'B';                           // A for Alice, B for Bob
	//uint16_t epsilon = 5;                             // local ring down guard time in ms (default 5)

	// these parameters are optional (Bob gets them from ALice)
	char chirpdurationindex = '1';                       // duration index - see chirp.cpp for values (default 1)
	char chirpguardindex = '1';                          // guard index - see chirp.cpp for values    (default 1)
	char chirptype = 'D';                                // Alice up (U), bob down (D)
	uint16_t nrepetitions = 0;

	//Chirp chirpinfo(chirpdurationindex, chirpguardindex, chirptype);

	Modem bob(COMportnum);                 // configure alice
	char systimeflag = 'D';                // 'E' for enabled or 'D' for disabled

	// ****** enable or clear system time 
	int64_t systime = bob.SysTimeEnable(systimeflag);
	if (systime < 0) {
		bob.PrintLogs();
		return systime;
	}

	// ****** wait for chirp info from Alice and parse message
	char rxmessage[1000];
	systime = bob.UnicastListen(rxmessage);
	sscanf_s(rxmessage, "%c%c%2hu%c", &chirptype, 1, &chirpdurationindex, 1, &nrepetitions, &chirpguardindex, 1);
	Chirp chirpinfo(chirpdurationindex, chirpguardindex, chirptype);

	// ****** Schedule probe signal to Alice
	uint64_t probetxtime = systime + MsToCounter(BOBDELAYMS, SYSTIMECLOCKHZ);
	int64_t txdurationms = bob.Probe(nrepetitions, chirpinfo, probetxtime);
	if (txdurationms < 0) {
		bob.PrintLogs();
		return txdurationms;
	}
	this_thread::sleep_for(milliseconds(BOBDELAYMS + txdurationms)); // wait for probe to be sent

	// ****** disable system time
	systime = bob.SysTimeDisable(systimeflag);
	bob.PrintLogs();
	if (systime < 0) return systime;

	return ENoErr;
}

void Test() {
	steady_clock::time_point t1 = steady_clock::now();
	this_thread::sleep_for(milliseconds(500));
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	double diff = time_span.count();

	int64_t stop = 0;
}

int main()
{
	bool test = 0;

	if (test) {
		Test();
	}
	else {
		int64_t err = Bob();
		PrintError(ErrNum(err));
	}
}
