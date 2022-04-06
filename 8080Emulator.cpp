#include <fstream>
#include <iostream>
#include "i8080Disassembler.h"

int main(int /*argc*/, char** argv)
{
	std::ifstream file(argv[1], std::ios::in | std::ios::binary);
    if (!file)
    {
        std::cerr << "Couldn't open: " << argv[1] << '\n';
        exit(1);
    }

    //Get the file size and read it into a memory buffer    
    file.seekg(0, std::ios::end);
	const int64_t fsize = file.tellg();
    file.seekg(0, std::ios::beg);

	auto* buffer = new uint8_t [fsize];

    file.read(reinterpret_cast<char*>(buffer), fsize);

    file.close();

    int pc = 0;

    while (pc < fsize)
    {
        pc += i8080Emulator::printDisassembledRom(buffer, pc);
    }

    delete[] buffer;
    return 0;
}
