// NM3_Driver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define SYSTIMECLOCKHZ 1000000 // sys time clock at 1MHz
#define PROPTIMECOUNTHZ 16000 // propagation time is a 16kHz counter

#define ALICEDELAYMS 250    // in ms
#define BOBDELAYMS 2000     // in ms
#define FSOFFSET 30			// in ms
#define ALICEUNICASTPREP 7  // in ms - for tx encoding

#include "chirp.h"
#include "maths.h"
#include "modem.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono;

int64_t Alice(Modem alice, uint16_t epsilon, Chirp chirpinfo) {

	uint16_t bobaddress = 102;		// Bobs address
	char messagebob[64];			// unicast configuration message to bob
	uint16_t messagelength = 0;
	char systimeflag = 'D';             // 'E' for enabled or 'D' for disabled

	// ****** Ping bob
	int64_t twowaytimecount = alice.Ping(bobaddress);
	if (twowaytimecount < 0) {
		alice.PrintLogs(ErrNum(twowaytimecount));
		return twowaytimecount;
	}
	double onewaytimems = CounterToMs(twowaytimecount, PROPTIMECOUNTHZ);
	uint16_t nrepetitions = CalculateChirpRepetitions(onewaytimems, epsilon, chirpinfo);

	// ****** prepare Chirp message
	if (chirpinfo.GetType() == 'U' || chirpinfo.GetType() == 'u') {	// TODO - Add this to command line parameter checks as well
		messagelength = sprintf_s(messagebob, "%c%c%02u%c", 'D', chirpinfo.GetDurationChar(), nrepetitions, chirpinfo.GetGuardChar());  // create message
	}
	else if (chirpinfo.GetType() == 'D' || chirpinfo.GetType() == 'd') {
		messagelength = sprintf_s(messagebob, "%c%c%02u%c", 'U', chirpinfo.GetDurationChar(), nrepetitions, chirpinfo.GetGuardChar());  // create message
	}
	else {
		cout << "Error: invalid chirp type. Please enter 'U'/'u' for an up-chirp or 'D'/'d' for a down chirp." << "\n";
	}

	// ****** enable or clear system time 
	int64_t systime = alice.SysTimeEnable(systimeflag);
	if (systime < 0) {
		alice.PrintLogs(ErrNum(systime));
		return systime;
	}
	uint64_t unicasttxtime = systime + MsToCounter(ALICEDELAYMS, SYSTIMECLOCKHZ);

	// ****** schedule chirp info message to bob
	int64_t txdurationms = alice.Unicast(bobaddress, messagebob, messagelength, unicasttxtime); // add time later...
	if (txdurationms < 0) {
		alice.PrintLogs(ErrNum(txdurationms));
		return txdurationms;
	}
	this_thread::sleep_for(milliseconds(ALICEDELAYMS + txdurationms)); // wait for Unicast to be sent

	uint64_t probetxtime = (unicasttxtime + MsToCounter(onewaytimems + BOBDELAYMS + FSOFFSET + ALICEUNICASTPREP, SYSTIMECLOCKHZ));

	// ****** Schedule probe signal to bob    
	txdurationms = alice.Probe(nrepetitions, chirpinfo, probetxtime);
	if (txdurationms < 0) {
		alice.PrintLogs(ErrNum(txdurationms));
		return txdurationms;
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(uint64_t(CounterToMs(probetxtime - unicasttxtime, SYSTIMECLOCKHZ)) + txdurationms)); // wait for probe to be sent
	this_thread::sleep_for(milliseconds(BOBDELAYMS + txdurationms)); // wait for probe to be sent

	// ****** disable system time
	systime = alice.SysTimeDisable(systimeflag);
	alice.PrintLogs(ErrNum(systime));
	return systime;
}

int64_t Bob(Modem bob) {
	// these parameters are optional (Bob gets them from ALice)
	char chirpdurationindex = '1';		// duration index - see chirp.cpp for values (default 1)
	char chirpguardindex = '1';			// guard index - see chirp.cpp for values    (default 1)
	char chirptype = 'D';				// Alice up (U), bob down (D)
	uint16_t nrepetitions = 0;
	char systimeflag = 'D';				// 'E' for enabled or 'D' for disabled

	// ****** enable or clear system time 
	int64_t systime = bob.SysTimeEnable(systimeflag);
	if (systime < 0) {
		bob.PrintLogs(ErrNum(systime));
		return systime;
	}

	// ****** wait for chirp info from Alice and parse message
	char rxmessage[1000];
	systime = bob.UnicastListen(rxmessage);
	if (systime < 0) {
		bob.PrintLogs(ErrNum(systime));
		return systime;
	}
	sscanf_s(rxmessage, "%c%c%2hu%c", &chirptype, 1, &chirpdurationindex, 1, &nrepetitions, &chirpguardindex, 1);
	Chirp chirpinfo(chirpdurationindex, chirpguardindex, chirptype);

	// ****** Schedule probe signal to Alice
	uint64_t probetxtime = systime + MsToCounter(BOBDELAYMS, SYSTIMECLOCKHZ);
	int64_t txdurationms = bob.Probe(nrepetitions, chirpinfo, probetxtime);
	if (txdurationms < 0) {
		bob.PrintLogs(ErrNum(txdurationms));
		return txdurationms;
	}
	this_thread::sleep_for(milliseconds(BOBDELAYMS + txdurationms)); // wait for probe to be sent

	// ****** disable system time
	systime = bob.SysTimeDisable(systimeflag);
	bob.PrintLogs(ErrNum(systime));
	return systime;
}

int main()
{
	char type = 'B';
	int64_t err = 0;

	if ((type == 'a') || (type == 'A')) {
		//pass these in as parameters
		wchar_t COMportnum = L'3';  // check COM port in device manager
		char modemtype = 'A';		// A for Alice, B for Bob
		uint16_t epsilon = 0;       // local ring down guard time in ms (default 5)
		Modem alice(COMportnum, modemtype);	// configure alice

		// these parameters are optional
		char chirpdurationindex = '0';	// duration index - see chirp.cpp for values (default 1)
		char chirpguardindex = '0';     // guard index - see chirp.cpp for values    (default 1)
		char chirptype = 'U';			// Alice up (U), bob down (D)
		Chirp chirpinfo(chirpdurationindex, chirpguardindex, chirptype);

		while (1) {
			err = Alice(alice, epsilon, chirpinfo);
			this_thread::sleep_for(milliseconds(3000)); // wait for probe to be sent
		}

	}
	else if ((type == 'b') || (type == 'B')) {
		//pass these in as parameters
		wchar_t COMportnum = L'7';	// check COM port in device manager
		char modemtype = 'B';		// A for Alice, B for Bob
		Modem bob(COMportnum, modemtype);	// configure bob

		while (1) {
			err = Bob(bob);
		}
	}
	else {
		return -1;
	}
}
