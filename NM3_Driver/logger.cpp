#include "logger.h"

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
}

//**************************************************************************************************
//*** Function to print the log to a timestamped text file
//**************************************************************************************************

void Logger::PrintToFile() {

}