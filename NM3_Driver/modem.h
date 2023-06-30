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

	 int SysTimeEnable();

	 int SysTimeGet(char& flag);

	 int SysTimeDisable();

	 int SendUnicast(unsigned Address, char message[], unsigned messagelength, int txtime = 0);

	 int SendBroadcast(char message[], unsigned messagelength, int txtime = 0);

	 int Probe(unsigned chirpcount, Chirp chirpinfo, int txtime = 0);

 private:
 
	 wchar_t port_[9] = {L"\\\\.\\COM4"};	// default COM3
	 const unsigned long baudrate_ = 9600;				// default 9600	

	 int ConfigureSerial(HANDLE& hCom, DCB& dcb);
};
