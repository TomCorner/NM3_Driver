#include "io.h"

void PrintChars(char* start, unsigned length) {
	for (unsigned i = 0; i < length; i++) {
		std::cout << *(start + i);
	}
}