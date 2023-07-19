#include "modem.h"


//**************************************************************************************************
//*** Modem class constructor
//**************************************************************************************************

Modem::Modem(wchar_t portnum) {
	port_[7] = portnum;
}

//**************************************************************************************************
//*** Function to get a 2-way propagation time (ping) from Nanomodem
//**************************************************************************************************

int64_t Modem::Ping(uint16_t address) {
	//open Serial port
	HANDLE hCom;                                                                    //serial port stuff 
	COMMTIMEOUTS localtimeout = { 0,0,100,0,0 }, remotetimeout = { 0,0,4500,0,0 };    // for serial    
	//COMMTIMEOUTS localtimeout = { 0,0,200,0,0 }, remotetimeout = { 0,0,4500,0,0 };    // for bluetooth
	DCB dcb;

	char rxbuf[1000], command[100];            // define rx and command buffers
	DWORD no_bytes = 0;
	uint16_t TmpAddress;
	int64_t propagation = -1;                             //returns -1 if no range measured (timed out)

	propagation = ConfigureSerial(hCom, dcb);
	if (propagation < 0) { // configure serial port, checks for connection errors
		return propagation;
	}

	sprintf_s(command, "$P%03u", address);               // create command
	std::cout << "Command: " << command << "\n";        // display command

	SetCommTimeouts(hCom, &localtimeout);               //timeouts for local response
	WriteFile(hCom, command, 5, &no_bytes, NULL);       //send command to modem
	ReadFile(hCom, rxbuf, 7, &no_bytes, NULL);          //read local response

	if ((no_bytes == 7) && (rxbuf[0] == '$')) {         // check modem response to ping command

		PrintChars(&rxbuf[0], no_bytes);
		SetCommTimeouts(hCom, &remotetimeout);          //timeouts for acoustic response
		ReadFile(hCom, rxbuf, 13, &no_bytes, NULL);     //read response or time out after 4.5 s

		if ((no_bytes == 13) && (rxbuf[0] == '#') && (rxbuf[1] == 'R') && (rxbuf[5] == 'T') && (rxbuf[11] == '\r') && (rxbuf[12] == '\n')) { // check acoustic response is correct format
			PrintChars(&rxbuf[0], no_bytes);
			//decode packet
			sscanf_s(rxbuf, "#R%3huT%5llu\r\n", &TmpAddress, &propagation);    // retrieve adress and propagation time from acknowledgement
			if (TmpAddress != address) propagation = ENM3UnexpectedErr;                    // incorrect address
		}
		else {                                  // no acoustic response - timeout
			PrintChars(&rxbuf[0], no_bytes);
		}
	}
	else {
		propagation = ErrorCheck(rxbuf[0], no_bytes);
	}

	CloseHandle(hCom);
	return(propagation);
}

//**************************************************************************************************
//*** Function to enable modems system time
//**************************************************************************************************

int64_t Modem::SysTimeEnable(char& flag) {
	int64_t  systime = SysTimeGet(flag);
	if (systime < 0) return systime;
	if (flag == 'D') {
		systime = SysTimeCommon(flag, 'E');
	}
	else {
		systime = SysTimeClear(flag);
	}
	//if (systime < 0) return systime;
	return systime;
}

//**************************************************************************************************
//*** Function to get modems system time - flag indicates with system time is enabled or diabled
//**************************************************************************************************

int64_t Modem::SysTimeGet(char& flag) {
	return SysTimeCommon(flag, 'G');
}

//**************************************************************************************************
//*** Function to disable modems system time
//**************************************************************************************************

int64_t Modem::SysTimeDisable(char& flag) {
	return SysTimeCommon(flag, 'D');
}

//**************************************************************************************************
//*** Function to clear modems system time
//**************************************************************************************************

int64_t Modem::SysTimeClear(char& flag) {
	return SysTimeCommon(flag, 'C');
}

//**************************************************************************************************
//*** Function to send unicast message to given address
//**************************************************************************************************

int64_t Modem::Unicast(uint16_t address, char message[], uint16_t messagelength, uint64_t txtime) {
	//open Serial port
	HANDLE hCom;                                    //serial port stuff
	COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };    // for serial
	//COMMTIMEOUTS localtimeout = { 0,0,200,0,0 };    // for bluetooth
	DCB dcb;

	char rxbuf[1000], command[100];            // define rx and command buffers
	DWORD no_bytes = 0;
	int64_t txduration = -1;                             //returns -1 if no time measured (timed out)

	txduration = ConfigureSerial(hCom, dcb);
	if (txduration < 0) { // configure serial port, checks for connection errors
		return txduration;
	}

	DWORD commandlength = 0;
	if (txtime == 0) {
		commandlength = 7 + messagelength;
		sprintf_s(command, "$U%03u%02u%s", address, messagelength, message);  // create command
	}
	else {
		commandlength = 22 + messagelength;
		sprintf_s(command, "$U%03u%02u%sT%014llu", address, messagelength, message, txtime);  // create command
	}
	std::cout << "Command: " << command << "\n";                                    // display command

	SetCommTimeouts(hCom, &localtimeout);                                           //timeouts for local response
	WriteFile(hCom, command, commandlength, &no_bytes, NULL);                       //send command to modem
	ReadFile(hCom, rxbuf, 9, &no_bytes, NULL);                                      //read local response

	if ((no_bytes == 9) && (rxbuf[0] == '$') && (rxbuf[1] == 'U')) {
		PrintChars(&rxbuf[0], no_bytes);
		txduration = int(((0.105 + ((messagelength + 16.0) * 2.0 * 50.0 / 8000.0)) * 1000) + 0.5);
	}
	else {
		txduration = ErrorCheck(rxbuf[0], no_bytes);
	}

	CloseHandle(hCom);
	return(txduration);
}

//**************************************************************************************************
//*** Function to send unicast message to given address and wait for an acknowledgement
//**************************************************************************************************

int64_t Modem::UnicastWithAck(uint16_t address, char message[], uint16_t messagelength, uint64_t txtime) {
	//open Serial port
	HANDLE hCom;                                                                    //serial port stuff
	COMMTIMEOUTS localtimeout = { 0,0,100,0,0 }, remotetimeout = { 0,0,4500,0,0 };    // for serial    
	//COMMTIMEOUTS localtimeout = { 0,0,200,0,0 }, remotetimeout = { 0,0,4500,0,0 };    // for bluetooth
	DCB dcb;

	char rxbuf[1000], command[100];            // define rx and command buffers
	DWORD no_bytes = 0;
	uint16_t TmpAddress;
	int64_t propagation = -1;                             //returns -1 if no time measured (timed out)

	propagation = ConfigureSerial(hCom, dcb);
	if (propagation < 0) { // configure serial port, checks for connection errors
		return propagation;
	}

	DWORD commandlength = 0;
	if (txtime == 0) {
		commandlength = 7 + messagelength;
		sprintf_s(command, "$M%03u%02u%s", address, messagelength, message);  // create command
	}
	else {
		commandlength = 22 + messagelength;
		sprintf_s(command, "$M%03u%02u%sT%014llu", address, messagelength, message, txtime);  // create command
	}
	std::cout << "Command: " << command << "\n";                                    // display command

	SetCommTimeouts(hCom, &localtimeout);               //timeouts for local response
	WriteFile(hCom, command, 12, &no_bytes, NULL);       //send command to modem
	ReadFile(hCom, rxbuf, 9, &no_bytes, NULL);          //read local response

	if ((no_bytes == 9) && (rxbuf[0] == '$')) {         // check modem response to ping command

		PrintChars(&rxbuf[0], no_bytes);
		SetCommTimeouts(hCom, &remotetimeout);          //timeouts for acoustic response
		ReadFile(hCom, rxbuf, 13, &no_bytes, NULL);     //read response or time out after 4.5 s

		if ((no_bytes == 13) && (rxbuf[0] == '#') && (rxbuf[1] == 'R') && (rxbuf[5] == 'T') && (rxbuf[11] == '\r') && (rxbuf[12] == '\n')) { // check acoustic response is correct format
			PrintChars(&rxbuf[0], no_bytes);
			//decode packet
			sscanf_s(rxbuf, "#R%3huT%5llu\r\n", &TmpAddress, &propagation);    // retrieve adress and propagation time from acknowledgement
			if (TmpAddress != address) propagation = ENM3UnexpectedErr;                    // incorrect address
		}
		else {                                  // no acoustic response - timeout
			PrintChars(&rxbuf[0], no_bytes);
		}
	}
	else {
		propagation = ErrorCheck(rxbuf[0], no_bytes);
	}

	CloseHandle(hCom);
	return(propagation);
}

//**************************************************************************************************
//*** Function to send broadcast message
//**************************************************************************************************

int64_t Modem::Broadcast(char message[], uint16_t messagelength, uint64_t txtime) {
	//open Serial port
	HANDLE hCom;                                    //serial port stuff
	COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };    // for serial
	//COMMTIMEOUTS localtimeout = { 0,0,200,0,0 };    // for bluetooth
	DCB dcb;

	char rxbuf[1000], command[100];            // define rx and command buffers
	DWORD no_bytes = 0;
	int64_t txduration = -1;                             //returns -1 if no time measured (timed out)

	txduration = ConfigureSerial(hCom, dcb);
	if (txduration < 0) { // configure serial port, checks for connection errors
		return txduration;
	}

	DWORD commandlength = 0;
	if (txtime == 0) {
		commandlength = 4 + messagelength;
		sprintf_s(command, "$B%02u%s", messagelength, message);  // create command
	}
	else {
		commandlength = 19 + messagelength;
		sprintf_s(command, "$B%02u%sT%014llu", messagelength, message, txtime);  // create command
	}
	std::cout << "Command: " << command << "\n";                                    // display command

	SetCommTimeouts(hCom, &localtimeout);                                           //timeouts for local response
	WriteFile(hCom, command, commandlength, &no_bytes, NULL);                       //send command to modem
	ReadFile(hCom, rxbuf, 6, &no_bytes, NULL);                                      //read local response

	if ((no_bytes == 6) && (rxbuf[0] == '$') && (rxbuf[1] == 'B')) {
		PrintChars(&rxbuf[0], no_bytes);
		txduration = int(((0.105 + ((messagelength + 16.0) * 2.0 * 50.0 / 8000.0)) * 1000) + 0.5);
	}
	else {
		txduration = ErrorCheck(rxbuf[0], no_bytes);
	}

	CloseHandle(hCom);
	return(txduration);
}

//**************************************************************************************************
//*** Function to send channel probe
//**************************************************************************************************

int64_t Modem::Probe(uint16_t chirprepetitions, Chirp chirpinfo, uint64_t txtime) {
	//open Serial port
	HANDLE hCom;                                    //serial port stuff
	COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };    // for serial
	//COMMTIMEOUTS localtimeout = { 0,0,200,0,0 };    // for bluetooth
	DCB dcb;

	char rxbuf[1000], command[100];            // define rx and command buffers
	DWORD no_bytes = 0;
	int64_t txduration = -1;                             //returns -1 if no time measured (timed out)

	txduration = ConfigureSerial(hCom, dcb);
	if (txduration < 0) { // configure serial port, checks for connection errors
		return txduration;
	}

	DWORD commandlength = 0;
	if (txtime == 0) {
		commandlength = 8;
		sprintf_s(command, "$XP%c%c%02u%c", chirpinfo.GetType(), chirpinfo.GetDurationChar(), chirprepetitions, chirpinfo.GetGuardChar());  // create command
	}
	else {
		commandlength = 23;
		sprintf_s(command, "$XP%c%c%02u%cT%014llu", chirpinfo.GetType(), chirpinfo.GetDurationChar(), chirprepetitions, chirpinfo.GetGuardChar(), txtime);  // create command
	}
	std::cout << "Command: " << command << "\n";                                    // display command

	SetCommTimeouts(hCom, &localtimeout);                                           //timeouts for local response
	WriteFile(hCom, command, commandlength, &no_bytes, NULL);                       //send command to modem
	ReadFile(hCom, rxbuf, 5, &no_bytes, NULL);                                      //read local response

	if ((no_bytes == 5) && (rxbuf[0] == '$') && (rxbuf[1] == 'X') && (rxbuf[2] == 'P')) {
		PrintChars(&rxbuf[0], no_bytes);
		txduration = chirprepetitions * (chirpinfo.GetGuardVal() + chirpinfo.GetDurationVal()) + chirpinfo.GetDurationVal();
	}
	else {
		txduration = ErrorCheck(rxbuf[0], no_bytes);
	}

	CloseHandle(hCom);
	return(txduration);
}

//**************************************************************************************************
//*** Function to listen for unicast messages
//**************************************************************************************************

int64_t Modem::UnicastListen(char* message) {
	//open Serial port
	HANDLE hCom;                                    //serial port stuff
	COMMTIMEOUTS localtimeout = { 0,0,60000,0,0 };  // listen timesout after a minute
	DCB dcb;

	char rxbuf[1000];            // define rx buffer
	DWORD no_bytes = 0;
	int64_t rxtime = -1;             //returns -1 if no time measured (timed out)

	int64_t serialcheck = ConfigureSerial(hCom, dcb);
	if (serialcheck < 0) { // configure serial port, checks for connection errors
		return serialcheck;
	}

	SetCommTimeouts(hCom, &localtimeout);                                           //timeouts for local response
	ReadFile(hCom, rxbuf, 26, &no_bytes, NULL);                                      //read local response

	if ((rxbuf[0] == '#') && (rxbuf[1] == 'U') && (rxbuf[2] == '0') && (rxbuf[3] == '5')) {
		PrintChars(&rxbuf[0], no_bytes);
		//decode packet
		uint16_t messagelength = 6; // expected length + terminating char
		if (rxbuf[9] == 'T') {
			sscanf_s(rxbuf, "#U05%[^T]T%14llu\r\n", message, messagelength, &rxtime);
		}
		else {
			sscanf_s(rxbuf, "#U05%s\r\n", message, messagelength);
			rxtime = ENoErr;
		}
	}
	else {
		rxtime = ErrorCheck(rxbuf[0], no_bytes);
	}

	CloseHandle(hCom);
	return(rxtime);
}

//**************************************************************************************************
//*** Function to configure serial port (common across modem commands)
//**************************************************************************************************

int64_t Modem::ConfigureSerial(HANDLE& hCom, DCB& dcb) {

	hCom = CreateFile(port_,
		GENERIC_READ | GENERIC_WRITE, 0,    /* comm devices must be opened w/exclusive-access */
		NULL,                               /* no security attrs */
		OPEN_EXISTING,                      /* comm devices must use OPEN_EXISTING */
		0,                                  /* not overlapped I/O */
		NULL                                /* hTemplate must be NULL for comm devices */
	);

	if (hCom == INVALID_HANDLE_VALUE) {
		std::cout << "\n*****\nError: Cannot open Modem COM port.\n*****\n";
		return(ECOMErr);
	}

	if (!GetCommState(hCom, &dcb)) {
		std::cout << "\n*****\nError: Cannot get DCB for Modem COM port.\n*****\n";
		return(EGetDCBErr);
	}

	dcb.BaudRate = baudrate_;                                         //for Nanomodem (via RS232)
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if (!SetCommState(hCom, &dcb)) {
		std::cout << "\n*****\nError: Cannot set DCB for Modem COM port.\n*****\n";
		return(ESetDCBErr);
	}

	PurgeComm(hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	return(ENoErr);
}

//**************************************************************************************************
//*** Function to send modem required system time command
//**************************************************************************************************

int64_t Modem::SysTimeCommon(char& flag, char commandchar) {
	//open Serial port
	HANDLE hCom;                                    //serial port stuff
	COMMTIMEOUTS localtimeout = { 0,0,100,0,0 };    // for serial
	//COMMTIMEOUTS localtimeout = { 0,0,200,0,0 };    // for bluetooth
	DCB dcb;

	char rxbuf[1000], commandstring[100];            // define rx and command buffers
	DWORD no_bytes = 0;
	int64_t systime = -1;                             //returns -1 if no time measured (timed out)

	systime = ConfigureSerial(hCom, dcb);
	if (systime < 0) { // configure serial port, checks for connection errors
		return(systime);
	}

	sprintf_s(commandstring, "$XT%c", commandchar);                         // create command
	std::cout << "Command: " << commandstring << "\n";        // dsiplay command

	SetCommTimeouts(hCom, &localtimeout);              //timeouts for local response
	WriteFile(hCom, commandstring, 4, &no_bytes, NULL);      //send command to modem
	ReadFile(hCom, rxbuf, 20, &no_bytes, NULL);        //read local response

	if ((no_bytes == 20) && (rxbuf[0] == '#') && (rxbuf[1] == 'X') && (rxbuf[2] == 'T') && (rxbuf[18] == '\r') && (rxbuf[19] == '\n')) { // check local response is correct format
		PrintChars(&rxbuf[0], no_bytes);
		sscanf_s(rxbuf, "#XT%c%14llu\r\n", &flag, 1, &systime);      //retrieve time from local response
	}
	else {
		systime = ErrorCheck(rxbuf[0], no_bytes);
	}

	CloseHandle(hCom);
	return(systime);
}


//**************************************************************************************************
//*** Function to perform error checks on received bytes
//**************************************************************************************************

int64_t Modem::ErrorCheck(char firstbyte, int64_t numbytes) {
	int64_t result;
	if (firstbyte == 'E') {
		PrintChars(&firstbyte, numbytes);
		std::cout << "\n*****\nError: Modem given unrecognised command.\n*****\n";
		result = ENM3CommandErr;
	}
	else if (numbytes == 0) {
		std::cout << "\n*****\nError: Timed out waiting for modem response.\n*****\n";
		result = ENM3TimeoutErr;
	}
	else {
		std::cout << "\n*****\nError: Modem received unexpected message.\n*****\n";
		result = ENM3UnexpectedErr;
	}
	return(result);
}