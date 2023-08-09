#pragma once

#include<windows.h>
#include "chirp.h"
#include "logger.h"

//**************************************************************************************************
//*** Class for configuring serial port and sending commands to a Nanomodem
//**************************************************************************************************

class Modem
{
public:
	Modem(wchar_t portnum, char type);

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

	void PrintLogs(ErrNum err);

private:

	wchar_t port_[9] = { L"\\\\.\\COM3" };	// default COM3
	const uint16_t baudrate_ = 9600;	// default 9600
	HANDLE hCom_;
	DCB dcb_;
	COMMTIMEOUTS listentimeout_ = { 0,0,60000,0,0 }, localtimeout_ = { 0,0,100,0,0 }, remotetimeout_ = { 0,0,4500,0,0 };  // listen timesout after a minute
	char rxbuf_[1000], commandstring_[100];            // define rx and command buffers
	Logger seriallog_;

	int64_t ConfigureSerial();

	int64_t SysTimeCommon(char& flag, char commandchar);

	int64_t ErrorCheck(int64_t numbytes);
};
