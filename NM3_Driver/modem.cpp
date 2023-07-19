#include "modem.h"

//**************************************************************************************************
//*** Function to print an error message based on its enum value
//**************************************************************************************************

void PrintError(ErrNum error) {
	switch (error) {
	case ECOMErr:
		std::cout << "\n*****\nError: Cannot open Modem COM port.\n*****\n";
		break;
	case EGetDCBErr:
		std::cout << "\n*****\nError: Cannot get DCB for Modem COM port.\n*****\n";
		break;
	case ESetDCBErr:
		std::cout << "\n*****\nError: Cannot set DCB for Modem COM port.\n*****\n";
		break;
	case ENM3CommandErr:
		std::cout << "\n*****\nError: Modem given unrecognised command.\n*****\n";
		break;
	case ENM3TimeoutErr:
		std::cout << "\n*****\nError: Timed out waiting for modem response.\n*****\n";
		break;
	case ENM3UnexpectedErr:
		std::cout << "\n*****\nError: Modem received unexpected message.\n*****\n";
		break;
	case ENoErr:
	default:
		std::cout << "\n**** Programme execution complete ****\n";
		break;
	}
}

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
	DWORD no_bytes = 0;
	uint16_t TmpAddress;
	int64_t propagation = ConfigureSerial();
	if (propagation < 0) return propagation;

	sprintf_s(commandstring_, "$P%03u", address);               // create command
	std::cout << "Command: " << commandstring_ << "\n";        // display command

	SetCommTimeouts(hCom_, &localtimeout_);               //timeouts for local response
	WriteFile(hCom_, commandstring_, 5, &no_bytes, NULL);       //send command to modem
	ReadFile(hCom_, rxbuf_, 7, &no_bytes, NULL);          //read local response

	if ((no_bytes == 7) && (rxbuf_[0] == '$')) {         // check modem response to ping command

		PrintChars(&rxbuf_[0], no_bytes);
		SetCommTimeouts(hCom_, &remotetimeout_);          //timeouts for acoustic response
		ReadFile(hCom_, rxbuf_, 13, &no_bytes, NULL);     //read response or time out after 4.5 s

		if ((no_bytes == 13) && (rxbuf_[0] == '#') && (rxbuf_[1] == 'R') && (rxbuf_[5] == 'T') && (rxbuf_[11] == '\r') && (rxbuf_[12] == '\n')) { // check acoustic response is correct format
			PrintChars(&rxbuf_[0], no_bytes);
			//decode packet
			sscanf_s(rxbuf_, "#R%3huT%5llu\r\n", &TmpAddress, &propagation);    // retrieve adress and propagation time from acknowledgement
			if (TmpAddress != address) propagation = ENM3UnexpectedErr;                    // incorrect address
		}
		else {                                  // no acoustic response - timeout
			PrintChars(&rxbuf_[0], no_bytes);
			propagation = ErrorCheck(no_bytes);
		}
	}
	else {
		propagation = ErrorCheck(no_bytes);
	}

	CloseHandle(hCom_);
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
	DWORD no_bytes = 0;
	int64_t txduration = ConfigureSerial();
	if (txduration < 0) return txduration;

	DWORD commandlength = 0;
	if (txtime == 0) {
		commandlength = 7 + messagelength;
		sprintf_s(commandstring_, "$U%03u%02u%s", address, messagelength, message);  // create command
	}
	else {
		commandlength = 22 + messagelength;
		sprintf_s(commandstring_, "$U%03u%02u%sT%014llu", address, messagelength, message, txtime);  // create command
	}
	std::cout << "Command: " << commandstring_ << "\n";                                    // display command

	SetCommTimeouts(hCom_, &localtimeout_);                                           //timeouts for local response
	WriteFile(hCom_, commandstring_, commandlength, &no_bytes, NULL);                       //send command to modem
	ReadFile(hCom_, rxbuf_, 9, &no_bytes, NULL);                                      //read local response

	if ((no_bytes == 9) && (rxbuf_[0] == '$') && (rxbuf_[1] == 'U')) {
		PrintChars(&rxbuf_[0], no_bytes);
		txduration = int(((0.105 + ((messagelength + 16.0) * 2.0 * 50.0 / 8000.0)) * 1000) + 0.5);
	}
	else {
		txduration = ErrorCheck(no_bytes);
	}

	CloseHandle(hCom_);
	return(txduration);
}

//**************************************************************************************************
//*** Function to send unicast message to given address and wait for an acknowledgement
//**************************************************************************************************

int64_t Modem::UnicastWithAck(uint16_t address, char message[], uint16_t messagelength, uint64_t txtime) {
	DWORD no_bytes = 0;
	uint16_t TmpAddress;
	int64_t propagation = ConfigureSerial();
	if (propagation < 0) return propagation;

	DWORD commandlength = 0;
	if (txtime == 0) {
		commandlength = 7 + messagelength;
		sprintf_s(commandstring_, "$M%03u%02u%s", address, messagelength, message);  // create command
	}
	else {
		commandlength = 22 + messagelength;
		sprintf_s(commandstring_, "$M%03u%02u%sT%014llu", address, messagelength, message, txtime);  // create command
	}
	std::cout << "Command: " << commandstring_ << "\n";                                    // display command

	SetCommTimeouts(hCom_, &localtimeout_);               //timeouts for local response
	WriteFile(hCom_, commandstring_, 12, &no_bytes, NULL);       //send command to modem
	ReadFile(hCom_, rxbuf_, 9, &no_bytes, NULL);          //read local response

	if ((no_bytes == 9) && (rxbuf_[0] == '$')) {         // check modem response to ping command

		PrintChars(&rxbuf_[0], no_bytes);
		SetCommTimeouts(hCom_, &remotetimeout_);          //timeouts for acoustic response
		ReadFile(hCom_, rxbuf_, 13, &no_bytes, NULL);     //read response or time out after 4.5 s

		if ((no_bytes == 13) && (rxbuf_[0] == '#') && (rxbuf_[1] == 'R') && (rxbuf_[5] == 'T') && (rxbuf_[11] == '\r') && (rxbuf_[12] == '\n')) { // check acoustic response is correct format
			PrintChars(&rxbuf_[0], no_bytes);
			//decode packet
			sscanf_s(rxbuf_, "#R%3huT%5llu\r\n", &TmpAddress, &propagation);    // retrieve adress and propagation time from acknowledgement
			if (TmpAddress != address) propagation = ENM3UnexpectedErr;                    // incorrect address
		}
		else {                                  // no acoustic response - timeout
			PrintChars(&rxbuf_[0], no_bytes);
			propagation = ErrorCheck(no_bytes);
		}
	}
	else {
		propagation = ErrorCheck(no_bytes);
	}

	CloseHandle(hCom_);
	return(propagation);
}

//**************************************************************************************************
//*** Function to send broadcast message
//**************************************************************************************************

int64_t Modem::Broadcast(char message[], uint16_t messagelength, uint64_t txtime) {
	DWORD no_bytes = 0;
	int64_t txduration = ConfigureSerial();
	if (txduration < 0) return txduration;

	DWORD commandlength = 0;
	if (txtime == 0) {
		commandlength = 4 + messagelength;
		sprintf_s(commandstring_, "$B%02u%s", messagelength, message);  // create command
	}
	else {
		commandlength = 19 + messagelength;
		sprintf_s(commandstring_, "$B%02u%sT%014llu", messagelength, message, txtime);  // create command
	}
	std::cout << "Command: " << commandstring_ << "\n";                                    // display command

	SetCommTimeouts(hCom_, &localtimeout_);                                           //timeouts for local response
	WriteFile(hCom_, commandstring_, commandlength, &no_bytes, NULL);                       //send command to modem
	ReadFile(hCom_, rxbuf_, 6, &no_bytes, NULL);                                      //read local response

	if ((no_bytes == 6) && (rxbuf_[0] == '$') && (rxbuf_[1] == 'B')) {
		PrintChars(&rxbuf_[0], no_bytes);
		txduration = int(((0.105 + ((messagelength + 16.0) * 2.0 * 50.0 / 8000.0)) * 1000) + 0.5);
	}
	else {
		txduration = ErrorCheck(no_bytes);
	}

	CloseHandle(hCom_);
	return(txduration);
}

//**************************************************************************************************
//*** Function to send channel probe
//**************************************************************************************************

int64_t Modem::Probe(uint16_t chirprepetitions, Chirp chirpinfo, uint64_t txtime) {
	DWORD no_bytes = 0;
	int64_t txduration = ConfigureSerial();
	if (txduration < 0) return txduration;

	DWORD commandlength = 0;
	if (txtime == 0) {
		commandlength = 8;
		sprintf_s(commandstring_, "$XP%c%c%02u%c", chirpinfo.GetType(), chirpinfo.GetDurationChar(), chirprepetitions, chirpinfo.GetGuardChar());  // create command
	}
	else {
		commandlength = 23;
		sprintf_s(commandstring_, "$XP%c%c%02u%cT%014llu", chirpinfo.GetType(), chirpinfo.GetDurationChar(), chirprepetitions, chirpinfo.GetGuardChar(), txtime);  // create command
	}
	std::cout << "Command: " << commandstring_ << "\n";                                    // display command

	SetCommTimeouts(hCom_, &localtimeout_);                                           //timeouts for local response
	WriteFile(hCom_, commandstring_, commandlength, &no_bytes, NULL);                       //send command to modem
	ReadFile(hCom_, rxbuf_, 5, &no_bytes, NULL);                                      //read local response

	if ((no_bytes == 5) && (rxbuf_[0] == '$') && (rxbuf_[1] == 'X') && (rxbuf_[2] == 'P')) {
		PrintChars(&rxbuf_[0], no_bytes);
		txduration = chirprepetitions * (chirpinfo.GetGuardVal() + chirpinfo.GetDurationVal()) + chirpinfo.GetDurationVal();
	}
	else {
		txduration = ErrorCheck(no_bytes);
	}

	CloseHandle(hCom_);
	return(txduration);
}

//**************************************************************************************************
//*** Function to listen for unicast messages
//**************************************************************************************************

int64_t Modem::UnicastListen(char* message) {
	DWORD no_bytes = 0;
	int64_t rxtime = ConfigureSerial();
	if (rxtime < 0) return rxtime;

	SetCommTimeouts(hCom_, &listentimeout_);                                           //timeouts for local response
	ReadFile(hCom_, rxbuf_, 26, &no_bytes, NULL);                                      //read local response

	if ((rxbuf_[0] == '#') && (rxbuf_[1] == 'U') && (rxbuf_[2] == '0') && (rxbuf_[3] == '5')) {
		PrintChars(&rxbuf_[0], no_bytes);
		//decode packet
		uint16_t messagelength = 6; // expected length + terminating char
		if (rxbuf_[9] == 'T') {
			sscanf_s(rxbuf_, "#U05%[^T]T%14llu\r\n", message, messagelength, &rxtime);
		}
		else {
			sscanf_s(rxbuf_, "#U05%s\r\n", message, messagelength);
			rxtime = ENoErr;
		}
	}
	else {
		rxtime = ErrorCheck(no_bytes);
	}

	CloseHandle(hCom_);
	return(rxtime);
}

//**************************************************************************************************
//*** Function to configure serial port (common across modem commands)
//**************************************************************************************************

int64_t Modem::ConfigureSerial() {

	hCom_ = CreateFile(port_,
		GENERIC_READ | GENERIC_WRITE, 0,    /* comm devices must be opened w/exclusive-access */
		NULL,                               /* no security attrs */
		OPEN_EXISTING,                      /* comm devices must use OPEN_EXISTING */
		0,                                  /* not overlapped I/O */
		NULL                                /* hTemplate must be NULL for comm devices */
	);

	if (hCom_ == INVALID_HANDLE_VALUE) {
		return(ECOMErr);
	}

	if (!GetCommState(hCom_, &dcb_)) {
		return(EGetDCBErr);
	}

	dcb_.BaudRate = baudrate_;                                         //for Nanomodem (via RS232)
	dcb_.ByteSize = 8;
	dcb_.Parity = NOPARITY;
	dcb_.StopBits = ONESTOPBIT;

	if (!SetCommState(hCom_, &dcb_)) {
		return(ESetDCBErr);
	}

	PurgeComm(hCom_, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	return(ENoErr);
}

//**************************************************************************************************
//*** Function to send modem required system time command
//**************************************************************************************************

int64_t Modem::SysTimeCommon(char& flag, char commandchar) {
	DWORD no_bytes = 0;
	int64_t systime = ConfigureSerial();
	if (systime < 0) return(systime);

	sprintf_s(commandstring_, "$XT%c", commandchar);                         // create command
	std::cout << "Command: " << commandstring_ << "\n";        // dsiplay command

	SetCommTimeouts(hCom_, &localtimeout_);              //timeouts for local response
	WriteFile(hCom_, commandstring_, 4, &no_bytes, NULL);      //send command to modem
	ReadFile(hCom_, rxbuf_, 20, &no_bytes, NULL);        //read local response

	if ((no_bytes == 20) && (rxbuf_[0] == '#') && (rxbuf_[1] == 'X') && (rxbuf_[2] == 'T') && (rxbuf_[18] == '\r') && (rxbuf_[19] == '\n')) { // check local response is correct format
		PrintChars(&rxbuf_[0], no_bytes);
		sscanf_s(rxbuf_, "#XT%c%14llu\r\n", &flag, 1, &systime);      //retrieve time from local response
	}
	else {
		systime = ErrorCheck(no_bytes);
	}

	CloseHandle(hCom_);
	return(systime);
}


//**************************************************************************************************
//*** Function to perform error checks on received bytes
//**************************************************************************************************

int64_t Modem::ErrorCheck(int64_t numbytes) {
	int64_t result;
	if (rxbuf_[0] == 'E') {
		PrintChars(&rxbuf_[0], numbytes);
		result = ENM3CommandErr;
	}
	else if (numbytes == 0) {
		result = ENM3TimeoutErr;
	}
	else {
		result = ENM3UnexpectedErr;
	}
	return(result);
}