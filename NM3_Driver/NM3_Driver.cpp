// NM3_Driver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define SYSTIMECLOCKHZ 1000000 // sys time clock at 1MHz
#define PROPTIMECOUNTHZ 16000 // propagation time is a 16kHz counter

#define ALICEDELAYMS 250    // in ms
#define BOBDELAYMS 2000     // in ms
#define FSOFFSET 30			// in ms
#define ALICEUNICASTPREP 7  // in ms - for tx encoding

#include "chirp.h"
#include "cmd_parse.h"
#include "maths.h"
#include "modem.h"
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono;

int64_t Alice(Modem alice, uint32_t epsilon, Chirp chirpinfo) {

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
	double onewaytimems = CounterToMs(twowaytimecount, PROPTIMECOUNTHZ) / 2.0;
	uint16_t nrepetitions = CalculateChirpRepetitions(onewaytimems, epsilon, chirpinfo);

	// ****** prepare Chirp message
	if (chirpinfo.GetType() == 'U' || chirpinfo.GetType() == 'u') {
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

int main(int argc, char* argv[])
{
	char** argument_list = &argv[0];
	int64_t err = ArgumentCheck(argc, argument_list); // checks number of arguments is correct
	if (err < 0) return -1;

	char modem_type;
	err = ParseType(argument_list, modem_type);
	if (err < 0) return -1;


	if ((modem_type == 'a') || (modem_type == 'A')) {
		wchar_t COMportnum;
		err = ParsePortNumber(argument_list, COMportnum);	// parses com port number
		if (err < 0) return -1;
		Modem alice(COMportnum, modem_type);	// configure alice modem

		cout << endl << "Alice configured to COM port " << COMportnum - '0' << endl;

		uint32_t epsilon;
		err = ParseEpsilon(argument_list, epsilon);	// parses ring down guard
		if (err < 0) return -1;

		char chirp_duration_index;
		err = ParseChirpDuration(argument_list, chirp_duration_index);	// parses and checks chirp duration
		if (err < 0) return -1;

		char chirp_guard_index;
		err = ParseChirpGuard(argument_list, chirp_guard_index);	// parses and checks chirp guard
		if (err < 0) return -1;

		char chirptype = 'U';			// Alice up (U), bob down (D)
		Chirp chirpinfo(chirp_duration_index, chirp_guard_index, chirptype);	// configures chirp

		cout << endl << "- Alice Probe signal -" << endl;
		cout << "Chirp Type(Up / Down) : " << chirpinfo.GetType() << endl;
		cout << "Chirp Duration : " << chirpinfo.GetDurationVal() << "ms" << endl;
		cout << "Chirp Guard : " << chirpinfo.GetGuardVal() << "ms" << endl;
		cout << "Ringdown guard (epsilon): " << epsilon << "ms" << endl;

		while (1) {
			err = Alice(alice, epsilon, chirpinfo);	// execute probe cycle
			this_thread::sleep_for(milliseconds(3000)); // arbitrary delay until next cycle
		}

	}
	else if ((modem_type == 'b') || (modem_type == 'B')) {
		wchar_t COMportnum;
		err = ParsePortNumber(argument_list, COMportnum);
		if (err < 0) return -1;
		Modem bob(COMportnum, modem_type);	// configure bob

		cout << endl << "Bob configured to COM port " << COMportnum - '0' << endl;
		cout << "Waiting for Alice..." << endl;

		while (1) {
			err = Bob(bob);
		}
	}
	else {
		return -1;
	}
}
