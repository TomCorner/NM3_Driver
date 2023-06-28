#pragma once

#include<windows.h>
#include <iostream>

//**************************************************************************************************
//*** Class for configuring serial port and sending commands to a Nanomodem
//**************************************************************************************************

class Modem
{
 public:
	 Modem(wchar_t port[4],  unsigned long baudrate);

	 int Ping(unsigned Address);

	 int SysTimeEnable();

 private:
	 wchar_t port_[4] = {L'C', L'O', L'M', L'3'};
	 unsigned long baudrate_;
};
