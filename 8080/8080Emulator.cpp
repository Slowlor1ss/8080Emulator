#include <fstream>
#include <iostream>
#include "i8080Disassembler.h"

int main(int argc, char** argv)
{
	i8080Emulator* i8080;
    if (argc == 2)
        i8080 = new i8080Emulator{ argv[1] };
    else
        i8080 = new i8080Emulator{ "../Roms/invaders.rom" };

    //i8080->PrintDisassembledRom();
	do{}
	while (i8080->CycleCpu());

    return 0;
}
