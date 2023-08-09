#include "logger.h"

Logger::Logger(char type) {
	SetFileName(type);
	CreateTextFile();
	Clear();
}

//**************************************************************************************************
//*** Function to append bytes to the log, stores bytes pointed to by start up to either a '\0' or '\r'
//**************************************************************************************************

void Logger::Append(char* start) {
	uint16_t count = 0;
	while (true) {
		if (*(start + count) == '\0') {
			break;
		}
		else if (*(start + count) == '\r') {
			break;
		}
		else {
			buf_.push_back(*(start + count));
			count++;
		}
	}
}

//**************************************************************************************************
//*** Function to append an error message for the provided error enum
//**************************************************************************************************

void Logger::AppendError(ErrNum err) {
	string message = GetErrorMessage(err);
	string::iterator it;
	for (it = message.begin(); it != message.end(); it++) {
		buf_.push_back(*it);
	}
}

//**************************************************************************************************
//*** Function to clear the log
//**************************************************************************************************

void Logger::Clear() {
	buf_.clear();
}

//**************************************************************************************************
//*** Function to print the log to the console window
//**************************************************************************************************

void Logger::PrintToScreen() {
	for (uint16_t i = 0; i < buf_.size(); i++) {
		if ((buf_.at(i) == '$') || (buf_.at(i) == '#')) {
			cout << '\n' << buf_.at(i);
		}
		else {
			cout << buf_.at(i);
		}
	}
	cout << '\n';
}

//**************************************************************************************************
//*** Function to print the log to a timestamped text file
//**************************************************************************************************

void Logger::PrintToFile() {
	std::ofstream file;
	file.open(filename_, std::ios_base::app); // append instead of overwrite
	for (uint16_t i = 0; i < buf_.size(); i++) {
		if ((buf_.at(i) == '$') || (buf_.at(i) == '#')) {
			file << '\n' << buf_.at(i);
		}
		else {
			file << buf_.at(i);
		}
	}
	file << '\n';
}

//**************************************************************************************************
//*** Function to set the log file name
//**************************************************************************************************

void Logger::SetFileName(char type) {
	time_t t = time(0);
	tm now{};
	localtime_s(&now, &t);

	if ((type == 'A') || (type == 'a')) {
		sprintf_s(filename_, "%04u%02u%02u_%02u%02u%02u_Alice_log.txt", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
	}
	else {
		sprintf_s(filename_, "%02u%02u%02u_%04u%02u%02u_Bob_log.txt", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday, now.tm_hour, now.tm_min, now.tm_sec);
	}

}

//**************************************************************************************************
//*** Function to create the log file
//**************************************************************************************************

void Logger::CreateTextFile() {
	ofstream o(filename_);
}

//**************************************************************************************************
//*** Function to return error message based on enum
//**************************************************************************************************
string Logger::GetErrorMessage(ErrNum error) {
	switch (error) {
	case ECOMErr:
		return "\n***** Error: Cannot open Modem COM port *****\n";
		break;
	case EGetDCBErr:
		return "\n***** Error: Cannot get DCB for Modem COM port *****\n";
		break;
	case ESetDCBErr:
		return "\n***** Error: Cannot set DCB for Modem COM port *****\n";
		break;
	case ENM3CommandErr:
		return "\n***** Error: Modem given unrecognised command *****\n";
		break;
	case ENM3TimeoutErr:
		return "\n***** Error: Timed out waiting for modem response *****\n";
		break;
	case ENM3UnexpectedErr:
		return "\n***** Error: Modem received unexpected message *****\n";
		break;
	case ENoErr:
	default:
		return "\n**** Cycle execution complete ****\n";
		break;
	}
}