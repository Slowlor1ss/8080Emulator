#include "i8080Disassembler.h"
#include <iomanip>
#include <iostream>
#include <format>

int i8080Emulator::printDisassembledRom(const uint8_t* codebuffer, const int pc)
{
	const unsigned char* code = &codebuffer[pc];
	const opcode op = Opcodes[*code];
	std::cout << std::setw(4) << std::setfill('0') << std::hex << pc << ' ';
	std::cout << std::setw(2) << std::setfill('0') << std::hex << op.code << '\t';

	std::cout << op.mnemonic << '\t';

	for (int i = op.sizeBytes - 1; i >= 1; --i)
		std::cout << std::format("{:02X}", code[i]);

	std::cout << '\n';

	return op.sizeBytes;
}
