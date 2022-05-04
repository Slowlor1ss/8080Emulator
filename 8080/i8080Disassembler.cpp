#include "i8080Disassembler.h"
#include <bitset>
#include <iomanip>
#include <iostream>
#include <format>
#include <fstream>

i8080Emulator::i8080Emulator(const char* path) : m_Cpu(new State8080{})
{
	std::ifstream file(path, std::ios::in | std::ios::binary);

	if (!file)
	{
		std::cerr << "Couldn't open file" << '\n';
		exit(1);
	}

	//Get the file size and read it into a memory buffer    
	file.seekg(0, std::ios::end);
	m_Fsize = file.tellg();
	file.seekg(0, std::ios::beg);

	m_Memory = new uint8_t[m_Fsize];

	file.read(reinterpret_cast<char*>(m_Memory), m_Fsize);

	file.close();


	//initialize CPU
	m_Cpu->pc = PROGRAM_START;
	m_Cpu->sp = STACK_START;
}

i8080Emulator::~i8080Emulator()
{
	delete m_Cpu;
	m_Cpu = nullptr;

	delete[] m_Memory;
}

void i8080Emulator::CycleCpu()
{
	if (m_Cpu->pc >= MEMORY_SIZE) {
		std::cout << "Program counter overflow\n";
		PrintRegister();
	}

	m_CurrentOpcode = m_Memory[m_Cpu->pc];
	(this->*OPCODES[m_CurrentOpcode].opcode)();
}

//Used for debugging
void i8080Emulator::PrintRegister() const
{
	std::cout << "Main registers\n";
	std::cout << "A: " << std::bitset<8>(m_Cpu->a) << " | " << "Flags: " << std::bitset<8>(m_Cpu->psw)  <<'\n';
	std::cout << "m_Cpu->b(): " << std::bitset<8>(m_Cpu->b()) << " | " << "C: " << std::bitset<8>(m_Cpu->c())  <<'\n';
	std::cout << "D: " << std::bitset<8>(m_Cpu->d()) << " | " << "E: " << std::bitset<8>(m_Cpu->e())  <<'\n';
	std::cout << "H: " << std::bitset<8>(m_Cpu->h()) << " | " << "L: " << std::bitset<8>(m_Cpu->l())  <<'\n';
	std::cout << '\n';
	std::cout << "Index registers (Stack Pointer)\n";
	std::cout << "SP: " << std::setw(4) << std::setfill('0') << std::hex << m_Cpu->sp << '\n';
	std::cout << '\n';
	std::cout << "Program counter\n";
	std::cout << "m_Cpu->pc: " << std::setw(4) << std::setfill('0') << std::hex << m_Cpu->pc << '\n';
	std::cout << '\n';
}

void i8080Emulator::PrintDisassembledRom() const
{
	//not using m_Cpu->pc because we just want to print the rom not actually move the pc
	auto pc = PROGRAM_START;

	while (pc < m_Fsize)
	{
		const unsigned char* code = &m_Memory[pc];
		const Opcode op = OPCODES[*code];
		std::cout << std::setw(4) << std::setfill('0') << std::hex << pc << ' ';
		std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(*code) << '\t';

		std::cout << op.mnemonic << '\t';

		for (int i = op.sizeBytes - 1; i >= 1; --i)
			std::cout << std::format("{:02X}", code[i]);

		std::cout << '\n';

		pc += op.sizeBytes;
	}
}

void i8080Emulator::defaultOpcode()
{
	std::cout << "Couldn't find opcode " << std::setw(2) << std::setfill('0') << std::hex << 
		static_cast<int>(m_CurrentOpcode) << '\t' << OPCODES[m_CurrentOpcode].mnemonic << '\n';
	PrintRegister();
	__debugbreak();
}

//Opcode implementations
#pragma region OpcodeFunctions

#pragma region GenericOpcodeFunctions
void i8080Emulator::RETURN(bool)
{
}

void i8080Emulator::RESET(uint16_t)
{
}

void i8080Emulator::CALLif(bool)
{
}

void i8080Emulator::JUMP(bool)
{
}

uint16_t i8080Emulator::POP()
{
	return uint16_t();
}

void i8080Emulator::DCX(char)
{
}

void i8080Emulator::DAD(char)
{
}

void i8080Emulator::DCR(uint8_t&)
{
}

void i8080Emulator::MVI(uint8_t&)
{
}

void i8080Emulator::PUSH(uint16_t)
{
}

void i8080Emulator::LXI(char)
{
}

void i8080Emulator::INX(char)
{
}

template <std::size_t Offset>
void i8080Emulator::INR(byte_ref<uint16_t, Offset> reg)
{
	reg += 1;
	m_Cpu->pc += 1;
}
#pragma endregion GenericOpcodeFunctions

void i8080Emulator::NOP() {
	m_Cpu->pc += 1;
}

void i8080Emulator::LXIB() {
	LXI('b');
}

void i8080Emulator::INRB() {
	INR(m_Cpu->b());
}

#pragma endregion OpcodeFunctions

