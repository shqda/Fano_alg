#include "cmd_flags.h"
#include <iostream>

/**
 * @details 
 * Existing flags:
 * - `-c` (**compress**) : compress a file
 * - `-d` (**decompress**) : decompress a file
 * - `-t` (**time**) : show execution time
 * - `-p` (**print**) : display symbols and codes after successful compression
*/


void print_flags() {
	std::cout << "Usage: fano <input> <output> [-c | -d] [flags]\n"
		<< "Flags:\n"
		<< "  -c   Compress file\n"
		<< "  -d   Decompress file\n"
		<< "  -t   Measure execution time\n"
		<< "  -p   Print code table\n";
}