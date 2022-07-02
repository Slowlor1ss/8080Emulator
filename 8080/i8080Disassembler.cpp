#include "i8080Disassembler.h"
#include <bitset>
#include <cassert>
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
		exit(3);
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

bool i8080Emulator::CycleCpu()
{
	if (m_Cpu->pc >= MEMORY_SIZE) {
		std::cout << "Program counter overflow\n";
		PrintRegister();
		return false;
	}

	m_CurrentOpcode = m_Memory[m_Cpu->pc];
	(this->*OPCODES[m_CurrentOpcode].opcode)();
	return true;
}

//Used for debugging
void i8080Emulator::PrintRegister() const
{
	std::cout << "Main registers\n";
	std::cout << "A: " << std::bitset<8>(m_Cpu->a) << " | " << "Flags: " << std::bitset<8>((m_Cpu->ConditionBits.s << 7) | (m_Cpu->ConditionBits.z << 6) | (m_Cpu->ConditionBits.ac << 4) | (m_Cpu->ConditionBits.p << 2) | (m_Cpu->ConditionBits.c << 0)) << '\n';
	std::cout << "m_Cpu->b(): " << std::bitset<8>(m_Cpu->b) << " | " << "C: " << std::bitset<8>(m_Cpu->c)  <<'\n';
	std::cout << "D: " << std::bitset<8>(m_Cpu->d) << " | " << "E: " << std::bitset<8>(m_Cpu->e)  <<'\n';
	std::cout << "H: " << std::bitset<8>(m_Cpu->h) << " | " << "L: " << std::bitset<8>(m_Cpu->l)  <<'\n';
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

void i8080Emulator::UpdateFlags(const uint8_t& value) const
{
	m_Cpu->ConditionBits.s = (value & 128); //set if negative
	m_Cpu->ConditionBits.z = (value == 0); //set if zero

	//TODO: clean up
	//find if the parity bit for 8bits (amount of true bits even or odd)
	//https://www.geeksforgeeks.org/finding-the-parity-of-a-number-efficiently/
	//uint8_t y = value ^ (value >> 1);
	//y = y ^ (y >> 2);
	//y = y ^ (y >> 4);
	//m_Cpu->ConditionBits.p = !(y&1); //true if number of true bits is even

	//find if the parity bit for 8bits (amount of true bits even or odd)
	uint8_t ones = 0;
	for (int shift = 0; shift < 8; ++shift) {
		if ((value >> shift) & 1) 
			++ones;
	}
	m_Cpu->ConditionBits.p = !(ones & 1);
}

//safely writes to RAM
void i8080Emulator::MemWrite(uint16_t address, uint8_t data)
{
	if (address >= MEMORY_SIZE) {
		address = address % ROM_SIZE + ROM_SIZE;
	}
	if (address < ROM_SIZE) {
		std::cout << "Illegal address write:" << address << '\n';
	}
	else {
		m_Memory[address] = data;
	}

}

uint8_t i8080Emulator::readPSW() const
{
	return static_cast<uint8_t>(
		  (m_Cpu->ConditionBits.c << 7)
		| (m_Cpu->ConditionBits.p << 6)
		| (m_Cpu->ConditionBits.ac << 4)
		| (m_Cpu->ConditionBits.z << 2)
		| (m_Cpu->ConditionBits.s << 0));
}

uint16_t i8080Emulator::ReadRegisterPair(RegisterPairs8080 pair) const
{
	uint8_t LSByte{}, MSByte{};
	switch (pair)
	{
	case RegisterPairs8080::BC:
		MSByte = m_Cpu->b;
		LSByte = m_Cpu->c;
		break;
	case RegisterPairs8080::DE:
		 MSByte = m_Cpu->d;
		 LSByte = m_Cpu->e;
		break;
	case RegisterPairs8080::HL:
		MSByte = m_Cpu->h;
		LSByte = m_Cpu->l;
		break;
	default:
		assert(!"should never get here!");
	}

	return (uint16_t(MSByte << 8) | LSByte);
}

void i8080Emulator::SetRegisterPair(RegisterPairs8080 pair, uint16_t value)
{
	//little-endian so MSbyte is last
	const uint8_t LSByte = (value & 0xFF00) >> 8;
	const uint8_t MSByte = (value & 0x00FF);

	SetRegisterPair(pair, LSByte, MSByte);
}

void i8080Emulator::SetRegisterPair(RegisterPairs8080 pair, uint8_t LSByte, uint8_t MSByte)
{
	switch (pair)
	{
	case RegisterPairs8080::BC:
		m_Cpu->b = MSByte;
		m_Cpu->c = LSByte;
		break;
	case RegisterPairs8080::DE:
		m_Cpu->d = MSByte;
		m_Cpu->e = LSByte;
		break;
	case RegisterPairs8080::HL:
		m_Cpu->h = MSByte;
		m_Cpu->l = LSByte;
		break;
	case RegisterPairs8080::SP:
		m_Cpu->sp = uint16_t(m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1];
		break;
	default:
		assert(!"should never get here!");
	}
}

//opcode implementations
#pragma region OpcodeFunctions

#pragma region GenericOpcodeFunctions
//fetches a PC from the stack
void i8080Emulator::RETURN(bool condition)
{
	if (condition) {
		m_Cpu->pc = uint16_t(m_Memory[m_Cpu->sp] << 8) | m_Memory[m_Cpu->sp + 1];
		m_Cpu->sp += 2;
	}
	else {
		m_Cpu->pc += 1;
	}
}

//behaves like call (saves pc to stack), but jumps to a specified number
void i8080Emulator::RESET(uint16_t callAddress)
{
	MemWrite((m_Cpu->sp - 1), uint8_t(m_Cpu->pc + 1));
	MemWrite((m_Cpu->sp - 2), ((m_Cpu->pc + 1) >> 8));
	m_Cpu->sp -= 2;
	m_Cpu->pc = callAddress;
}

//pushes PC, then sets PC
void i8080Emulator::CALLif(bool condition)
{
	if (condition) {
		MemWrite((m_Cpu->sp - 1), ((m_Cpu->pc + 3) & 0xFF00) >> 8);
		MemWrite((m_Cpu->sp - 2), ((m_Cpu->pc + 3) & 0x00FF));
		m_Cpu->sp -= 2;

		m_Cpu->pc = uint16_t(m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1]; //TODO: check if this works
	}
	else {
		m_Cpu->pc += 3;
	}
}

void i8080Emulator::JUMP(bool condition)
{
	if (condition) {
		const uint16_t address = uint16_t(m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1]; //TODO: check if this works
		m_Cpu->pc = address;
	}
	else {
		m_Cpu->pc += 3;
	}
}

uint16_t i8080Emulator::POP()
{
	uint16_t value = uint16_t(m_Memory[m_Cpu->sp] << 8) | m_Memory[m_Cpu->sp + 1];
	m_Cpu->sp += 2;
	m_Cpu->pc += 1;
	return value;
}

//decrement register pair
void i8080Emulator::DCX(RegisterPairs8080 pair)
{
	const uint16_t result = ReadRegisterPair(pair) - 1;
	SetRegisterPair(pair, result);
	m_Cpu->pc += 1;
}

//add register pair to HL
void i8080Emulator::DAD(RegisterPairs8080 pair)
{
	const uint32_t result = ReadRegisterPair(RegisterPairs8080::HL) + ReadRegisterPair(pair);
	SetRegisterPair(RegisterPairs8080::HL, uint16_t(result));
	m_Cpu->ConditionBits.c = (result >= 0x00010000);
	m_Cpu->pc += 1;
}

//decrements register
void i8080Emulator::DCR(uint8_t& reg)
{
	reg -= 1;
	UpdateFlags(reg);
	m_Cpu->pc += 1;
}

//sets a register to the byte after PC
void i8080Emulator::MVI(uint8_t& reg)
{
	reg = m_Memory[m_Cpu->pc + 1];
	m_Cpu->pc += 2;
}

//see page 3 8080-Programmers-Manual [STACK PUSH OPERATION]
void i8080Emulator::PUSH(uint16_t value)
{
	MemWrite((m_Cpu->sp - 1), (value & 0x00FF));
	MemWrite((m_Cpu->sp - 2), (value & 0xFF00) >> 8);
	m_Cpu->sp -= 2;
	m_Cpu->pc += 1;
}

//loads immediate into register pair
void i8080Emulator::LXI(RegisterPairs8080 pair)
{
	//SetRegisterPair(pair, 
	//	(uint16_t(m_Memory[m_Cpu->pc + 1] << 8) | m_Memory[m_Cpu->pc + 2])
	//);

	//little-endian so MSbyte is last
	SetRegisterPair(pair,m_Memory[m_Cpu->pc + 1], m_Memory[m_Cpu->pc + 2]);

	m_Cpu->pc += 3;
}

//increments register pair
void i8080Emulator::INX(RegisterPairs8080 pair)
{
	const uint16_t result = ReadRegisterPair(pair) + 1;
	SetRegisterPair(pair, result);
	m_Cpu->pc += 1;
}

//increments given register by one
void i8080Emulator::INR(uint8_t& reg)
{
	reg += 1;
	UpdateFlags(reg);
	m_Cpu->pc += 1;
}
#pragma endregion GenericOpcodeFunctions

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void i8080Emulator::NOP() {
	m_Cpu->pc += 1;
}

void i8080Emulator::LXIB() {
	LXI(RegisterPairs8080::BC);
}

void i8080Emulator::INXB() {
	INX(RegisterPairs8080::BC);
}

void i8080Emulator::INRB() {
	INR(m_Cpu->b);
}

void i8080Emulator::DCRB() {
	DCR(m_Cpu->b);
}

void i8080Emulator::MVIB() {
	MVI(m_Cpu->b);
}

void i8080Emulator::DCRC() {
	DCR(m_Cpu->c);
}

void i8080Emulator::LXID() {
	LXI(RegisterPairs8080::DE);
}

void i8080Emulator::INXD() {
	INX(RegisterPairs8080::DE);
}

void i8080Emulator::DCRD() {
	DCR(m_Cpu->d);
}

void i8080Emulator::LDAXD() {
	m_Cpu->a = m_Memory[ReadRegisterPair(RegisterPairs8080::DE)];
	m_Cpu->pc += 1;
}

void i8080Emulator::DCRE() {
	DCR(m_Cpu->e);
}

void i8080Emulator::LXIH() {
	LXI(RegisterPairs8080::HL);
}

void i8080Emulator::INXH() {
	INX(RegisterPairs8080::HL);
}

void i8080Emulator::DCRH() {
	DCR(m_Cpu->h);
}

void i8080Emulator::DCRL() {
	DCR(m_Cpu->l);
}

void i8080Emulator::LXISP() {
	LXI(RegisterPairs8080::SP);
}

void i8080Emulator::INXSP() {
	INX(RegisterPairs8080::SP);
}

void i8080Emulator::DCRM() {
	uint16_t address = ReadRegisterPair(RegisterPairs8080::HL);
	uint8_t newValue = m_Memory[address] - 1;
	MemWrite(address, newValue);
	UpdateFlags(newValue);
	m_Cpu->pc += 1;
}

void i8080Emulator::DCRA() {
	DCR(m_Cpu->a);
}


//based 8080-Programmers-Manual page 16-17 [MOV Instruction]
// encoding	| 8bit reg| 
// 000		| B		  | 
// 001		| C		  | 
// 010		| D		  | 
// 011		| E		  | 
// 100		| H		  | 
// 101		| L		  | 
// 110		| M		  | memory reference M, the addressed location is specified by the HL reg
// 111		| A		  | 
uint8_t* i8080Emulator::DecodeOpcodeRegister(uint8_t opcode3bit)
{
	switch (opcode3bit & 0b111)
	{
	case 0b000:
		return &m_Cpu->b;
	case 0b001:
		return &m_Cpu->c;
	case 0b010:
		return &m_Cpu->d;
	case 0b011:
		return &m_Cpu->e;
	case 0b100:
		return &m_Cpu->h;
	case 0b101:
		return &m_Cpu->l;
	case 0b110:
		return &(m_Memory[ReadRegisterPair(RegisterPairs8080::HL)]);
	case 0b111:
		return &m_Cpu->a;
	default:
		assert(!"should never get here!");
		return nullptr;
	}
}

//// (0x40 - 0x75), (0x77 - 0x7F) | MOV
void i8080Emulator::MOV() {
	uint8_t src = (m_CurrentOpcode & 0b00000111);
	uint8_t dest = (m_CurrentOpcode & 0b00111000) >> 3;

	*DecodeOpcodeRegister(dest) = *DecodeOpcodeRegister(src);

	m_Cpu->pc += 1;
}

void i8080Emulator::JNZ() {
	JUMP(!m_Cpu->ConditionBits.z);
}

void i8080Emulator::JMP() {
	JUMP(true);
}

void i8080Emulator::PUSHB() {
	PUSH(ReadRegisterPair(RegisterPairs8080::BC));
}

void i8080Emulator::RET() {
	RETURN(true);
}

void i8080Emulator::JZ() {
	JUMP(m_Cpu->ConditionBits.z);
}

void i8080Emulator::CALL() {
	CALLif(true);
}

void i8080Emulator::PUSHD() {
	PUSH(ReadRegisterPair(RegisterPairs8080::DE));
}

void i8080Emulator::PUSHH() {
	PUSH(ReadRegisterPair(RegisterPairs8080::HL));
}

void i8080Emulator::PUSHPSW() {
	uint16_t data = uint8_t(m_Cpu->a << 8) | readPSW();
	PUSH(data);
}


#pragma endregion OpcodeFunctions

