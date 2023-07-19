#pragma once

#include<windows.h>
#include "io.h"
#include "chirp.h"

//**************************************************************************************************
//*** List of enums for error control
//**************************************************************************************************

enum ErrNum {
	ENoErr = 0,				// No errors
	ECOMErr = -1,			// Cannot open Modem COM port
	EGetDCBErr = -2,		// Cannot get DCB for Modem COM port
	ESetDCBErr = -3,		// Cannot set DCB for Modem COM port
	ENM3CommandErr = -4,	// Modem given unrecognised command
	ENM3TimeoutErr = -5,	// Timed out waiting for modem response
	ENM3UnexpectedErr = -6	// Modem received unexpected message
};

//**************************************************************************************************
//*** Class for configuring serial port and sending commands to a Nanomodem
//**************************************************************************************************

class Modem
{
public:
	Modem(wchar_t portnum);

	int64_t Ping(uint16_t Address);

	int64_t SysTimeEnable(char& flag);

	int64_t SysTimeGet(char& flag);

	int64_t SysTimeDisable(char& flag);

	int64_t SysTimeClear(char& flag);

	int64_t Unicast(uint16_t Address, char message[], uint16_t messagelength, uint64_t txtime = 0);

	int64_t UnicastWithAck(uint16_t Address, char message[], uint16_t messagelength, uint64_t txtime = 0);

	int64_t Broadcast(char message[], uint16_t messagelength, uint64_t txtime = 0);

	int64_t Probe(uint16_t chirprepetitions, Chirp chirpinfo, uint64_t txtime = 0);

	int64_t UnicastListen(char* message);

private:

	wchar_t port_[9] = { L"\\\\.\\COM3" };	// default COM3
	const uint16_t baudrate_ = 9600;	// default 9600	

	int64_t ConfigureSerial(HANDLE& hCom, DCB& dcb);

	int64_t SysTimeCommon(char& flag, char commandchar);

	int64_t ErrorCheck(char firstbyte, int64_t numbytes);
};
