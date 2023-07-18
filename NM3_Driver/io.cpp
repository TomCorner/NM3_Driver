#include "io.h"

void PrintChars(char* start, uint64_t length) {
	for (uint64_t i = 0; i < length; i++) {
		std::cout << *(start + i);
	}
}