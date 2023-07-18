#pragma once

#include<windows.h>
#include "io.h"
#include "chirp.h"

//**************************************************************************************************
//*** Class for configuring serial port and sending commands to a Nanomodem
//**************************************************************************************************

class Modem
{
 public:
	 Modem(wchar_t portnum);

	 int Ping(unsigned Address);

	 int SysTimeEnable(char& flag);

	 int SysTimeGet(char& flag);

	 int SysTimeDisable(char& flag);

	 int SysTimeClear(char& flag);

	 int Unicast(unsigned Address, char message[], unsigned messagelength, unsigned txtime = 0);

	 int UnicastWithAck(unsigned Address, char message[], unsigned messagelength, unsigned txtime = 0);

	 int Broadcast(char message[], unsigned messagelength, unsigned txtime = 0);

	 int Probe(unsigned chirprepetitions, Chirp chirpinfo, unsigned txtime = 0);

	 int UnicastListen(char *message);

 private:
 
	 wchar_t port_[9] = {L"\\\\.\\COM3"};	// default COM3
	 const unsigned long baudrate_ = 9600;	// default 9600	

	 int ConfigureSerial(HANDLE& hCom, DCB& dcb);

	 int SysTimeCommon(char& flag, char commandchar);
};
