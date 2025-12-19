#include "cmd_flags.h"
#include "FileCompressor.h"
#include <chrono>
#include <iostream>
#include <string>

enum MODES {
	COMPRESS,
	DECOMPRESS
};


/**
 * @brief Main func.
 * Using class FileCompressor for archiving/unarchiving files.
 * 
 * Existing flags:
 * - `-c` (**compress**) : compress a file
 * - `-d` (**decompress**) : decompress a file
 * - `-t` (**time**) : show execution time
 * - `-p` (**print**) : display symbols and codes after successful compression
 *
 * @return Returns 0 if no issues.
 */


int main(int argc, char* argv[]) {
	if (argc < 4) {
		print_flags();
		return 0;
	}
	std::string input = argv[1];
	std::string output = argv[2];
	bool showTime = false, printCodes = false;
	int mode = -1;

	for (int i = 3; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "-c") mode = COMPRESS;
		else if (arg == "-d") mode = DECOMPRESS;
		else if (arg == "-t") showTime = true;
		else if (arg == "-p") printCodes = true;
	}

	if (mode == -1) {
		std::cerr << "Specify mode: -c (compress) or -d (decompress)\n";
		return 1;
	}

	FileCompressor fc;

	auto start = std::chrono::high_resolution_clock::now();

	if (mode == COMPRESS) {
		try {
			fc.compress(input, output);
			if (printCodes) fc.print_codes();
		}
		catch (const std::exception& e) {
			std::cout << e.what();
		}
	}
	else {
		try {
			fc.decompress(input, output);
		}
		catch (const std::exception& e) {
			std::cout << e.what();
		}
	}
	auto end = std::chrono::high_resolution_clock::now();

	if (showTime) {
		std::chrono::duration<double> duration = end - start;
		std::cout << "Execution time: " << duration.count() << "s\n";
	}

	return 0;
}