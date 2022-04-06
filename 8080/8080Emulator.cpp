#include <fstream>
#include <iostream>
#include "i8080Disassembler.h"

int main(int argc, char** argv)
{
	std::ifstream file{};
    if (argc == 2)
	    file.open(argv[1], std::ios::in | std::ios::binary);
    else
	    file.open("../Roms/invaders.rom", std::ios::in | std::ios::binary);

	if (!file)
    {
        std::cerr << "Couldn't open file" << '\n';
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
