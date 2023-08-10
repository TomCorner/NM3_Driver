#include "cmd_parse.h"

//**************************************************************************************************
//*** Function to check the number of arguments matches what is expected for either alice or bob
//**************************************************************************************************

int16_t ArgumentCheck(int argc, char** argv) {
	int16_t result = -1;
	char modem_type;
	switch (argc)
	{
	case 3:
		modem_type = *argv[1];
		if ((modem_type == 'b') || (modem_type == 'B')) {
			result = 0;
		}
		else {
			cout << endl << "Error: Modem type must be A/a for Alice or B/b for Bob" << endl << endl;
		}
		break;
	case 6:
		modem_type = *argv[1];
		if ((modem_type == 'a') || (modem_type == 'A')) {
			result = 0;
		}
		else {
			cout << endl << "Error: Modem type must be A/a for Alice or B/b for Bob" << endl << endl;
		}
		break;
	default:
		cout << endl << "Error: Incorrect number of input arguments" << endl << endl;
		break;
	}
	return result;
}

//**************************************************************************************************
//*** Function to parse the modem type from the argument list and checks it is acceptable
//**************************************************************************************************

int16_t ParseType(char** argv, char& type) {
	sscanf_s(argv[1], "%c,", &type, 1);

	if ((type == 'A') || (type == 'a') || (type == 'B') || (type == 'b')) {
		return 0;
	}
	else {
		cout << endl << "Error: Modem type must be A/a for Alice or B/b for Bob" << endl << endl;
		return -1;
	}
}

//**************************************************************************************************
//*** Function to parse the com port number from the argument list
//**************************************************************************************************

int16_t ParsePortNumber(char** argv, wchar_t& port_number) {
	char input_pnum;
	sscanf_s(argv[2], "%c,", &input_pnum, 1);
	port_number = input_pnum;
	return 0;
}

//**************************************************************************************************
//*** Function to parse epsilon (ring down guard) from the argument list
//**************************************************************************************************

int16_t ParseEpsilon(char** argv, uint32_t& epsilon) {
	sscanf_s(argv[3], "%u,", &epsilon);
	if (epsilon > 1000) {
		cout << endl << "Error: Ring down guard (epsilon) needs to be between 0 and 1000ms" << endl << endl;
	}
	return 0;
}

//**************************************************************************************************
//*** Function to parse chirp duration from the argument list and checks it is acceptable
//**************************************************************************************************

int16_t ParseChirpDuration(char** argv, char& chirp_duration_index) {
	uint16_t result = 0;
	uint32_t duration_test;
	sscanf_s(argv[4], "%u,", &duration_test);
	switch (duration_test)
	{
	case 2:
		chirp_duration_index = '0';
		break;
	case 5:
		chirp_duration_index = '1';
		break;
	case 10:
		chirp_duration_index = '2';
		break;
	case 20:
		chirp_duration_index = '3';
		break;
	case 30:
		chirp_duration_index = '4';
		break;
	case 40:
		chirp_duration_index = '5';
		break;
	case 50:
		chirp_duration_index = '6';
		break;
	case 100:
		chirp_duration_index = '7';
		break;
	case 200:
		chirp_duration_index = '8';
		break;
	default:
		cout << endl << "Error: Incorrect chirp duration, please choose from the following...\n2/5/10/20/30/40/50/100/200/200 ms" << endl << endl;
		result = -1;
		break;
	}
	return result;
}

//**************************************************************************************************
//*** Function to parse chirp guard from the argument list and checks it is acceptable
//**************************************************************************************************

int16_t ParseChirpGuard(char** argv, char& chirp_guard_index) {
	uint16_t result = 0;
	uint32_t guard_test;
	sscanf_s(argv[5], "%u,", &guard_test);
	switch (guard_test)
	{
	case 0:
		chirp_guard_index = '0';
		break;
	case 5:
		chirp_guard_index = '1';
		break;
	case 10:
		chirp_guard_index = '2';
		break;
	case 20:
		chirp_guard_index = '3';
		break;
	case 30:
		chirp_guard_index = '4';
		break;
	case 40:
		chirp_guard_index = '5';
		break;
	case 50:
		chirp_guard_index = '6';
		break;
	case 100:
		chirp_guard_index = '7';
		break;
	case 200:
		chirp_guard_index = '8';
		break;
	case 500:
		chirp_guard_index = '9';
		break;
	default:
		cout << endl << "Error: Incorrect chirp guard time, please choose from the following...\n0/5/10/20/30/40/50/100/200/500 ms" << endl << endl;
		result = -1;
		break;
	}
	return result;
}