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

//
//// 0x02 | Write A to mem at address BC 
//void i8080Emulator::STAXB() {
//	writeMem(readRegisterPair('b'), A);
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 7;
//}
//
//// 0x03 | Increase the m_Cpu->b() register pair
//void i8080Emulator::INXB() {
//	INX('b');
//}
//

void i8080Emulator::INRB() {
	INR(m_Cpu->b());
}

//
//// 0x05 | subtracts one from m_Cpu->b() and alters S, Z, AC, P flags
//void i8080Emulator::DCRB() {
//	DCR(m_Cpu->b());
//}
//
//// 0x06 | moves next byte into m_Cpu->b() register
//void i8080Emulator::MVIB() {
//	MVI(m_Cpu->b());
//}
//
//// 0x07 | bit shift A left, bit 0 & Cy = prev bit 7
//void i8080Emulator::RLC() {
//	uint8_t oldBit7 = (A & 0x80) >> 7;
//	A <<= 1;
//	A = A | oldBit7;
//	flags.CY = (1 == oldBit7);
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 4;
//}
//
////0x08 | unused NOP (0x00)
//
//// 0x09 | adds BC onto HL
//void i8080Emulator::DADB() {
//	DAD('b');
//}
//
//// 0x0A | set register A to the contents or memory pointed by BC
//void i8080Emulator::LDAXB() {
//	A = memory[readRegisterPair('m_Cpu->b()')];
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 7;
//}
//
//// 0x0B | decrease m_Cpu->b() pair by 1
//void i8080Emulator::DCXB() {
//	DCX('m_Cpu->b()');
//}
//
//// 0x0C | increases register C by 1, alters S, Z, AC, P flags
//void i8080Emulator::INRC() {
//	INR(C);
//}
//
//// 0x0D | decreases C by 1
//void i8080Emulator::DCRC() {
//	DCR(C);
//}
//
//// 0x0E | stores byte after m_Cpu->pc into C
//void i8080Emulator::MVIC() {
//	MVI(C);
//}
//
//// 0x0F | rotates A right 1, bit 7 & CY = prev bit 0
//void i8080Emulator::RRC() {
//	uint8_t oldBit0 = (A & 0x01);
//	A >>= 1;
//	A = A | (oldBit0 << 7);
//	flags.CY = (0x01 == oldBit0);
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 4;
//}
//
//// 0x10 | not an 8080 instruction, use as NOP
//
//// 0x11 | writes the bytes after m_Cpu->pc into the D register pair, E=pc+1, D=pc+2
//void i8080Emulator::LXID() {
//	LXI('D');
//}
//
//// 0x12 | stores A into the address pointed to by the D reg pair
//// XXX no opcode count for stax
//void i8080Emulator::STAXD() {
//	writeMem(readRegisterPair('D'), A);
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 7;
//}
//
//// 0x13 | increases the D reg pair by one
//void i8080Emulator::INXD() {
//	INX('D');
//}
//
//// 0x14 | increases the D register by one
//void i8080Emulator::INRD() {
//	INR(D);
//}
//
//// 0x15 | decreases register D by one
//void i8080Emulator::DCRD() {
//	DCR(D);
//}
//
//// 0x16 | set reg D to the byte following m_Cpu->pc
//void i8080Emulator::MVID() {
//	MVI(D);
//}
//
//// 0x17 | shifts A Left 1, bit 0 = prev CY, CY = prev bit 7
//// XXX ternary
//void i8080Emulator::RAL() {
//	uint8_t oldA = A;
//	A <<= 1;
//	A = A | ((flags.CY) ? 0x01 : 0x00);
//	flags.CY = (0x80 == (oldA & 0x80));
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 4;
//}
//
//// 0x18 | not an 8080 code, use as NOP
//
//// 0x19 | add reg pair D onto reg pair H
//void i8080Emulator::DADD() {
//	DAD('D');
//}
//
//// 0x1A | store the value at the memory referenced by DE in A
//void i8080Emulator::LDAXD() {
//	A = memory[readRegisterPair('D')];
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 7;
//}
//
//// 0x1B | decrease DE pair by 1
//void i8080Emulator::DCXD() {
//	DCX('D');
//}
//
//// 0x1C | increase reg E by 1
//void i8080Emulator::INRE() {
//	INR(E);
//}
//
//// 0x1D | decrease reg E by 1
//void i8080Emulator::DCRE() {
//	DCR(E);
//}
//
//// 0x1E | move byte after m_Cpu->pc into reg E
//void i8080Emulator::MVIE() {
//	MVI(E);
//}
//
//// 0x1F | rotate A right one, bit 7 = prev CY, CY = prevbit0
//void i8080Emulator::RAR() {
//	uint8_t oldA = A;
//	A >>= 1;
//	A = A | ((flags.CY) ? 0x80 : 0x00);
//	flags.CY = (0x01 == (oldA & 0x01));
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 4;
//}
//
//// 0x20 | not an 8080 code, use as NOP
//
//// 0x21 | loads data after m_Cpu->pc into H pair
//void i8080Emulator::LXIH() {
//	LXI('H');
//}
//
//// 0x22 | stores HL into adress listed after m_Cpu->pc
//// adr <- L , adr+1 <- H
//void i8080Emulator::SHLD() {
//	uint16_t address = (memory[m_Cpu->pc + 2] << 8) | memory[m_Cpu->pc + 1];
//	writeMem(address, L);
//	writeMem(address + 1, H);
//	m_Cpu->pc += 3;
//	opcodeCycleCount = 16;
//}
//
//// 0x23 | Increase the H register pair
//void i8080Emulator::INXH() {
//	INX('H');
//}
//
//// 0x24 | adds one to H and alters S, Z, AC, P flags
//void i8080Emulator::INRH() {
//	INR(H);
//}
//
//// 0x25 | subtracts one from H and alters S, Z, AC, P flags
//void i8080Emulator::DCRH() {
//	DCR(H);
//}
//
//// 0x26 | moves next byte into H register
//void i8080Emulator::MVIH() {
//	MVI(H);
//}
//
//// 0x27 | convert accumulator into 2x 4bit BCD
//// XXX overflow issue?
//void i8080Emulator::DAA() {
//	if (((A & 0x0F) > 0x09) || flags.AC) {
//		flags.AC = true;
//		A += 0x06;
//	}
//	else {
//		flags.AC = false;
//	}
//
//	if (((A & 0xF0) > 0x90) || flags.CY) {
//		flags.CY = true;
//		A += 0x60;
//	}
//	else {
//		flags.CY = false;
//	}
//	updateFlags(A);
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 4;
//}
//
//// 0x28 | not 8080 code use as NOP
//
//// 0x29 | add HL onto HL
//void i8080Emulator::DADH() {
//	DAD('H');
//}
//
//// 0x2A | read memory from 2bytes after m_Cpu->pc into HL
//// L <- adr, H <- adr+1
//void i8080Emulator::LHLD() {
//	uint16_t address = ((memory[m_Cpu->pc + 2] << 8) | memory[m_Cpu->pc + 1]);
//	L = memory[address];
//	H = memory[address + 1];
//	m_Cpu->pc += 3;
//	opcodeCycleCount = 16;
//}
//
//// 0x2B | decrease HL by 1
//void i8080Emulator::DCXH() {
//	DCX('H');
//}
//
//// 0x2C | adds one to L and alters S, Z, AC, P flags
//void i8080Emulator::INRL() {
//	INR(L);
//}
//
//// 0x2D | subtracts one from L and alters S, Z, AC, P flags
//void i8080Emulator::DCRL() {
//	DCR(L);
//}
//
//// 0x2E | moves next byte into L register
//void i8080Emulator::MVIL() {
//	MVI(L);
//}
//
//// 0x2F | invert A
//void i8080Emulator::CMA() {
//	A = ~A;
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 4;
//}
//
//// 0x30 | unimplemented in 8080, treat as NOP
//
//// 0x31 | load data after m_Cpu->pc as new Stack pointer
//void i8080Emulator::LXISP() {
//	LXI('S');
//}
//
//// 0x32 | stores A into the address from bytes after m_Cpu->pc
//// adr.low = m_Cpu->pc+1, adr.hi = m_Cpu->pc+2
//void i8080Emulator::STA() {
//	uint16_t address = (memory[m_Cpu->pc + 2] << 8) | memory[m_Cpu->pc + 1];
//	writeMem(address, A);
//	m_Cpu->pc += 3;
//	opcodeCycleCount = 13;
//}
//
//// 0x33 | add one to SP
//void i8080Emulator::INXSP() {
//	INX('S');
//}
//
//// 0x34 | increase the value pointed to by HL
//void i8080Emulator::INRM() {
//	uint16_t address = readRegisterPair('H');
//	uint8_t newValue = memory[address] + 1;
//	writeMem(address, newValue);
//	updateFlags(newValue);
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 10;
//}
//
//// 0x35 | decrease the value pointed to by HL
//void i8080Emulator::DCRM() {
//	uint16_t address = readRegisterPair('H');
//	uint8_t newValue = memory[address] - 1;
//	writeMem(address, newValue);
//	updateFlags(newValue);
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 10;
//}
//
//// 0x36 | move the byte after m_Cpu->pc into memory pointed to by HL
//void i8080Emulator::MVIM() {
//	writeMem(readRegisterPair('H'), memory[m_Cpu->pc + 1]);
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 10;
//}
//
//// 0x37 | set carry flag to 1
//void i8080Emulator::STC() {
//	flags.CY = true;
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 4;
//}
//
//// 0x38 | unimplemented in 8080, use as alt NOP
//
//// 0x39 | add SP onto HL
//void i8080Emulator::DADSP() {
//	DAD('S');
//}
//
//// 0x3A | set reg A to the value pointed by bytes after m_Cpu->pc
//void i8080Emulator::LDA() {
//	A = memory[((memory[m_Cpu->pc + 2] << 8) | memory[m_Cpu->pc + 1])];
//	m_Cpu->pc += 3;
//	opcodeCycleCount = 13;
//}
//
//// 0x3B | decrease SP by 1
//void i8080Emulator::DCXSP() {
//	DCX('S');
//}
//
//// 0x3C | adds one to A and alters S, Z, AC, P flags
//void i8080Emulator::INRA() {
//	INR(A);
//}
//
//// 0x3D | subtracts one from A and alters S, Z, AC, P flags
//void i8080Emulator::DCRA() {
//	DCR(A);
//}
//
//// 0x3E | moves next byte into A register
//void i8080Emulator::MVIA() {
//	MVI(A);
//}
//
//// 0x3F | invert carry flag
//void i8080Emulator::CMC() {
//	flags.CY = !flags.CY;
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 5;
//}
//
//// (0x40 - 0x75), (0x77 - 0x7F) | MOV
//void i8080Emulator::MOV() {
//	uint8_t dest = (opcode & 0b00111000) >> 3;
//	uint8_t source = (opcode & 0b00000111);
//
//	// MOV's involving Mem use 7, not 5
//	opcodeCycleCount = 7;
//	/*
//	if(dest == 0b110) { // if dest is mem
//		writeMem(readRegisterPair('H'), *opcodeDecodeRegisterBits(source));
//	} else if(source == 0b110){ // if source is mem
//		*opcodeDecodeRegisterBits(dest) =  readRegisterPair('H'));
//	} else { // dest & source are normal registers
//		*opcodeDecodeRegisterBits(dest) = *opcodeDecodeRegisterBits(source);
//		opcodeCycleCount = 5;
//	}
//	*/
//	* opcodeDecodeRegisterBits(dest) = *opcodeDecodeRegisterBits(source);
//	if (dest == 0b110 || source == 0b110) {
//		opcodeCycleCount = 5;
//	}
//	m_Cpu->pc += 1;
//}
//
//// 0x76 | XXX not done yet
//void i8080Emulator::HLT() {
//	printf("HLT not implemented yet!\n");
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 7;
//}
//
//// (0x80 - 0x87) | Add a register/mem value onto A, and update all flags
//void i8080Emulator::ADD() {
//	uint8_t sourceAddress = (opcode & 0b00000111);
//	uint8_t increment = 0;
//	if (sourceAddress == 0b110) { //sourcing from mem pointed by HL
//		increment = memory[readRegisterPair('H')];
//		opcodeCycleCount = 7;
//	}
//	else { // not sourcing from mem
//		increment = *opcodeDecodeRegisterBits(sourceAddress); //XXX *odrp()?
//		opcodeCycleCount = 4;
//	}
//	uint16_t sum = A + increment;
//	A = (sum & 0xFF);
//	flags.CY = sum > 0xFF;
//	updateFlags(A);
//	m_Cpu->pc += 1;
//}
//
//// (0x88 - 0x8F) | Add a register/mem value + carry bit onto A, update registers
//void i8080Emulator::ADC() {
//	uint8_t sourceAddress = (opcode & 0b00000111);
//	uint8_t increment = 0;
//	if (sourceAddress == 0b110) { //sourcing from mem pointed by HL
//		increment = memory[readRegisterPair('H')];
//		opcodeCycleCount = 7;
//	}
//	else { // not sourcing from mem
//		increment = *opcodeDecodeRegisterBits(sourceAddress);
//		opcodeCycleCount = 4;
//	}
//	uint16_t sum = A + increment + (uint8_t)flags.CY;
//	A = (sum & 0xFF);
//	flags.CY = sum > 0xFF;
//	updateFlags(A);
//	m_Cpu->pc += 1;
//}
//
//// (0x90 - 0x97) | Sub a register/mem value from A, and update all flags
//void i8080Emulator::SUB() {
//	uint8_t sourceAddress = (opcode & 0b00000111);
//	uint8_t decrement = 0;
//	if (sourceAddress == 0b110) { //sourcing from mem pointed by HL
//		decrement = memory[readRegisterPair('H')];
//		opcodeCycleCount = 7;
//	}
//	else { // not sourcing from mem
//		decrement = *opcodeDecodeRegisterBits(sourceAddress);
//		opcodeCycleCount = 4;
//	}
//	// 2s complement subtraction, done in 4bit amounts, for AC
//	decrement = ~decrement;
//	uint8_t lowerDec = decrement & 0x0F;
//	uint8_t lowerA = A & 0x0F;
//	uint8_t lowerSum = lowerDec + lowerA + 1;
//	flags.AC = lowerSum > 0xF;
//
//	uint8_t upperDec = (decrement & 0xF0) >> 4;
//	uint8_t upperA = (A & 0xF0) >> 4;
//	uint8_t upperSum = upperDec + upperA + ((lowerSum & 0x10) >> 4);
//	flags.CY = (0x10 != (upperSum & 0x10));
//	A = (upperSum << 4) + lowerSum;
//	updateFlags(A);
//	m_Cpu->pc += 1;
//	/*
//		uint16_t sum = A - decrement;
//		A = (sum & 0xFF);
//		flags.CY = sum > 0xFF;
//		updateFlags(A);
//		m_Cpu->pc += 1;
//	*/
//}
//
//// (0x98 - 0x9F) | Add a register/mem value - carry bit onto A, update registers
//void i8080Emulator::SBB() {
//	uint8_t sourceAddress = (opcode & 0b00000111);
//	uint8_t decrement = 0;
//	if (sourceAddress == 0b110) { //sourcing from mem pointed by HL
//		decrement = memory[readRegisterPair('H')];
//		opcodeCycleCount = 7;
//	}
//	else { // not sourcing from mem
//		decrement = *opcodeDecodeRegisterBits(sourceAddress);
//		opcodeCycleCount = 4;
//	}
//
//	// 2s complement subtraction, done in 4bit amounts, for AC
//	decrement = ~decrement;
//	uint8_t lowerDec = decrement & 0x0F;
//	uint8_t lowerA = A & 0x0F;
//	uint8_t lowerSum = lowerDec + lowerA + 1 + (uint8_t)flags.CY;
//	flags.AC = lowerSum > 0xF;
//
//	uint8_t upperDec = (decrement & 0xF0) >> 4;
//	uint8_t upperA = (A & 0xF0) >> 4;
//	uint8_t upperSum = upperDec + upperA + ((lowerSum & 0x10) >> 4);
//	flags.CY = (0x10 != (upperSum & 0x10));
//	A = (upperSum << 4) + lowerSum;
//	updateFlags(A);
//	m_Cpu->pc += 1;
//	/*
//		uint16_t sum = A - decrement - (uint8_t)flags.CY;
//		A = (sum & 0xFF);
//		flags.CY = sum > 0xFF;
//		updateFlags(A);
//		m_Cpu->pc += 1;
//	*/
//}
//
//// (0xA0 - 0xA7) | Bitwise AND a register/mem value with A, update registers
//void i8080Emulator::ANA() {
//	uint8_t sourceAddress = (opcode & 0b00000111);
//	uint8_t modifier = 0;
//	if (sourceAddress == 0b110) { //sourcing from mem pointed by HL
//		modifier = memory[readRegisterPair('H')];
//		opcodeCycleCount = 7;
//	}
//	else { // not sourcing from mem
//		modifier = *opcodeDecodeRegisterBits(sourceAddress);
//		opcodeCycleCount = 4;
//	}
//	uint16_t result = A & modifier;
//	A = (result & 0xFF);
//	flags.CY = false;
//	updateFlags(A);
//	m_Cpu->pc += 1;
//}
//
//// (0xA8 - 0xAF) | Bitwise XOR a register/mem value with A, update registers
//void i8080Emulator::XRA() {
//	uint8_t sourceAddress = (opcode & 0b00000111);
//	uint8_t modifier = 0;
//	if (sourceAddress == 0b110) { //sourcing from mem pointed by HL
//		modifier = memory[readRegisterPair('H')];
//		opcodeCycleCount = 7;
//	}
//	else { // not sourcing from mem
//		modifier = *opcodeDecodeRegisterBits(sourceAddress);
//		opcodeCycleCount = 4;
//	}
//	uint16_t result = A ^ modifier;
//	A = (result & 0xFF);
//	flags.CY = false;
//	updateFlags(A);
//	m_Cpu->pc += 1;
//}
//
//// (0xB0 - 0xB7) | Bitwise OR a register/mem value with A, update registers
//void i8080Emulator::ORA() {
//	uint8_t sourceAddress = (opcode & 0b00000111);
//	uint8_t modifier = 0;
//	if (sourceAddress == 0b110) { //sourcing from mem pointed by HL
//		modifier = memory[readRegisterPair('H')];
//		opcodeCycleCount = 7;
//	}
//	else { // not sourcing from mem
//		modifier = *opcodeDecodeRegisterBits(sourceAddress);
//		opcodeCycleCount = 4;
//	}
//	uint16_t result = A | modifier;
//	A = (result & 0xFF);
//	flags.CY = false;
//	updateFlags(A);
//	m_Cpu->pc += 1;
//}
//
//// (0xB8 - 0xBF) | Compare a register/mem value with A, update registers
//void i8080Emulator::CMP() {
//	uint8_t sourceAddress = (opcode & 0b00000111);
//	uint8_t modifier = 0;
//	if (sourceAddress == 0b110) { //sourcing from mem pointed by HL
//		modifier = memory[readRegisterPair('H')];
//		opcodeCycleCount = 7;
//	}
//	else { // not sourcing from mem
//		modifier = *opcodeDecodeRegisterBits(sourceAddress);
//		opcodeCycleCount = 4;
//	}
//	uint16_t result = A - modifier;
//	flags.CY = (result & 0xFF00) == 0;
//	updateFlags(result);
//	m_Cpu->pc += 1;
//}
//
//// 0xC0 | return on nonzero
//void i8080Emulator::RNZ() {
//	RETURN((flags.Z == false));
//}
//
//// 0xC1 | pull BC pair from stack
//void i8080Emulator::POPB() {
//	writeRegisterPair('m_Cpu->b()', POP());
//}
//
//// 0xC2 | jump to address from after m_Cpu->pc if non zero
//void i8080Emulator::JNZ() {
//	JUMP(flags.Z == false);
//}
//
//// 0xC3 | normal jump
//void i8080Emulator::JMP() {
//	JUMP(true);
//}
//
//// 0xC4 | run call if non zero
//void i8080Emulator::CNZ() {
//	genericCALL(flags.Z == false);
//}
//
//// 0xC5 | store BC pair on stack
//void i8080Emulator::PUSHB() {
//	PUSH(readRegisterPair('m_Cpu->b()'));
//}
//
//// 0xC6 | adds a byte onto A, fetched after m_Cpu->pc
//void i8080Emulator::ADI() {
//	uint16_t sum = A + memory[m_Cpu->pc + 1];
//	A = (sum & 0xFF);
//	flags.CY = sum > 0xFF00;
//	updateFlags(A);
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 7;
//}
//
//// 0xC7 | calls to 0H 
//void i8080Emulator::RST0() {
//	RESET(0x0);
//}
//
//// 0xC8 | return if Z
//void i8080Emulator::RZ() {
//	RETURN((flags.Z));
//}
//
//// 0xC9 | standard return
//void i8080Emulator::RET() {
//	RETURN(true);
//}
//
//// 0xCA | 
//void i8080Emulator::JZ() {
//	JUMP(flags.Z);
//}
//
//// 0xCB | not 8080 code, treat as NOP
//
//// 0xCC | call if Z flag set
//void i8080Emulator::CZ() {
//	genericCALL(flags.Z);
//}
//
//// 0xCD | store m_Cpu->pc on stack, and jump to a new location
//void i8080Emulator::CALL() {
//	genericCALL(true);
//}
//
//// 0xCE | add carry bit and Byte onto A
//void i8080Emulator::ACI() {
//	uint16_t sum = A + memory[m_Cpu->pc + 1] + (uint8_t)flags.CY;
//	A = (sum & 0xFF);
//	flags.CY = sum > 0xFF00;
//	updateFlags(A);
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 7;
//}
//
//// 0xCF | resets to 8H
//void i8080Emulator::RST1() {
//	RESET(0x8);
//}
//
//// 0xD0 | return if Cy = 0
//void i8080Emulator::RNC() {
//	RETURN(flags.CY == false);
//}
//
//// 0xD1 | fetch data from the stack, and store in DE pair
//void i8080Emulator::POPD() {
//	writeRegisterPair('D', POP());
//}
//
//// 0xD2 | jump when carry = 0
//void i8080Emulator::JNC() {
//	JUMP(flags.CY == false);
//}
//
//// 0xD3 | Outputs data, technically on data bus, but we only care about shifts
//void i8080Emulator::OUT() {
//	uint8_t port = memory[m_Cpu->pc + 1];
//	switch (port) {
//	case(2):
//		//printf("out(2) shiftreg = %X, shiftamnt = %d\n", bitShifter, bitShifterAmount);
//		bitShifterAmount = A;
//		break;
//	case(3):
//		//printf("sound! %d\n", A);
//		break;
//	case(4):
//		bitShifter = bitShifter >> 8;
//		bitShifter |= (A << 8);
//		//printf("out(4) A=%X,  shiftreg = %X, shiftamnt = %d\n", A, bitShifter, bitShifterAmount);
//		break;
//	case(5):
//		//printf("sound %d\n", A);
//		break;
//	case(6):
//		// watchdog, not required for space invaders
//		break;
//	}
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 10;
//}
//
//// 0xD4 | call on carry = false
//void i8080Emulator::CNC() {
//	genericCALL((flags.CY == false));
//}
//
//// 0xD5 | store DE pair onto stack
//void i8080Emulator::PUSHD() {
//	PUSH(readRegisterPair('D'));
//}
//
//// 0xD6 | subtract a byte from A
//void i8080Emulator::SUI() {
//	uint16_t result = A - memory[m_Cpu->pc + 1];
//	A = (result & 0xFF);
//	flags.CY = result > 0xFF00;
//	updateFlags(A);
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 7;
//}
//
//// 0xD7 | reset to 10H
//void i8080Emulator::RST2() {
//	RESET(0x10);
//}
//
//// 0xD8 | return if CY = 1
//void i8080Emulator::RC() {
//	RETURN(flags.CY);
//}
//
//// 0xD9 | not an 8080 code
//
//// 0xDA | jump if cy = 1
//void i8080Emulator::JC() {
//	JUMP(flags.CY);
//}
//
//// 0xDB | IN, read data from bus in reg A
//void i8080Emulator::IN() {
//	uint8_t port = memory[m_Cpu->pc + 1];
//	switch (port) {
//	case(1):
//		A = port1;
//		break;
//	case(2):
//		A = port2;
//		break;
//	case(3):
//		A = (bitShifter & (0xFF00 >> bitShifterAmount)) >> (8 - bitShifterAmount);
//		//printf("IN(3) shiftreg = %X, shiftamnt = %d, A = %X\n", bitShifter, bitShifterAmount, A);
//		break;
//	}
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 10;
//}
//
//// 0xDC | call if cy =1
//void i8080Emulator::CC() {
//	genericCALL(flags.CY);
//}
//
//// 0xDD | not 8080 code
//
//// 0xDE | sub byte and cy from A
//void i8080Emulator::SBI() {
//	uint16_t sum = A - memory[m_Cpu->pc + 1] - (uint8_t)flags.CY;
//	A = (sum & 0xFF);
//	flags.CY = sum > 0xFF00;
//	updateFlags(A);
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 7;
//}
//
//// 0xDF | reset to 18H
//void i8080Emulator::RST3() {
//	RESET(0x18);
//}
//
//// 0xE0 | return if P = O
//void i8080Emulator::RPO() {
//	RETURN(flags.P == 0);
//}
//
//// 0xE1 | store values from stack in HL pair
//void i8080Emulator::POPH() {
//	writeRegisterPair('H', POP());
//}
//
//// 0xE2 | jump if p = 0
//void i8080Emulator::JPO() {
//	JUMP(flags.P == 0);
//}
//
//// 0xE3 | exchange HL and SP data
//// L <-> (SP) | H <-> (SP+1)
//void i8080Emulator::XTHL() {
//	uint16_t stackContents = (memory[SP + 1] << 8) | memory[SP];
//	writeMem(SP, L);
//	writeMem(SP + 1, H);
//	writeRegisterPair('H', stackContents);
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 18;
//}
//
//// 0xE4 | call if p = 0
//void i8080Emulator::CPO() {
//	genericCALL(flags.P == 0);
//}
//
//// 0xE5 | store HL on the stack
//void i8080Emulator::PUSHH() {
//	PUSH(readRegisterPair('H'));
//}
//
//// 0xE6 | bitwise AND byte with A
//void i8080Emulator::ANI() {
//	uint16_t sum = A & memory[m_Cpu->pc + 1];
//	A = (sum & 0xFF);
//	flags.CY = sum > 0xFF00;
//	updateFlags(A);
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 7;
//}
//
//// 0xE7 | reset to 20H
//void i8080Emulator::RST4() {
//	RESET(0x20);
//}
//
//// 0xE8 | return if p = 1
//void i8080Emulator::RPE() {
//	RETURN(flags.P);
//}
//
//// 0xE9 | set pc to HL
//void i8080Emulator::PCHL() {
//	m_Cpu->pc = readRegisterPair('H');
//}
//
//// 0xEA | jump if p = 1
//void i8080Emulator::JPE() {
//	JUMP(flags.P);
//}
//
//// 0xEB | exchange HL and DE
//void i8080Emulator::XCHG() {
//	uint16_t oldDE = readRegisterPair('D');
//	writeRegisterPair('D', (H << 8) | L);
//	writeRegisterPair('H', oldDE);
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 5;
//}
//
//// 0xEC | call if p = 1
//void i8080Emulator::CPE() {
//	genericCALL(flags.P);
//}
//
//// 0xED | not 8080 code
//
//// 0xEE | XOR A with a byte
//void i8080Emulator::XRI() {
//	uint16_t sum = A ^ memory[m_Cpu->pc + 1];
//	A = (sum & 0xFF);
//	flags.CY = sum > 0xFF00;
//	updateFlags(A);
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 7;
//}
//
//// 0xEF | reset to 28H
//void i8080Emulator::RST5() {
//	RESET(0x28);
//}
//
//// 0xF0 | return if positive
//void i8080Emulator::RP() {
//	RETURN(flags.S == false);
//}
//
//// 0xF1 | pops A and flags  off of stack
//void i8080Emulator::POPPSW() {
//	uint16_t data = (memory[SP + 1] << 8) | memory[SP];
//	SP += 2;
//	A = (data >> 8);
//	writePSW(data & 0x00FF);
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 10;
//}
//
//// 0xF2 | jump if positive
//void i8080Emulator::JP() {
//	JUMP(flags.S == false);
//}
//
//// 0xF3 | Disable Interrupts
//void i8080Emulator::DI() {
//	INTE = false;
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 4;
//}
//
//// 0xF4 | call if positive
//void i8080Emulator::CP() {
//	genericCALL(flags.S == false);
//}
//
//// 0xF5 | store PSW and A in the stack (PSW as low byte)
//void i8080Emulator::PUSHPSW() {
//	uint16_t data = (A << 8) | readPSW();
//	PUSH(data);
//}
//
//// 0xF6 | biwise OR A with a byte
//void i8080Emulator::ORI() {
//	uint16_t sum = A | memory[m_Cpu->pc + 1];
//	A = (sum & 0xFF);
//	flags.CY = sum > 0xFF00;
//	updateFlags(A);
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 7;
//}
//
//// 0xF7 | rest to 30H
//void i8080Emulator::RST6() {
//	RESET(0x30);
//}
//
//// 0xF8 | return if minus
//void i8080Emulator::RM() {
//	RETURN(flags.S == true);
//}
//
//// 0xF9 | sets SP to HL
//void i8080Emulator::SPHL() {
//	SP = readRegisterPair('H');
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 5;
//}
//
//// 0xFA | jump if minus
//void i8080Emulator::JM() {
//	JUMP(flags.S == true);
//}
//
//// 0xFB | Enable Interrrupts
//void i8080Emulator::EI() {
//	INTE = true;
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 4;
//}
//
//// 0xFC | call if minus
//void i8080Emulator::CM() {
//	genericCALL(flags.S == true);
//}
//
//// 0xFD | not 8080 code
//
//// 0xFE | compare A to byte
//void i8080Emulator::CPI() {
//	uint8_t sum = A - memory[m_Cpu->pc + 1];
//	//uint16_t sum = A - memory[m_Cpu->pc+1];
//	//printf("A: %X - byte: %X  sum = %X\n",A, memory[m_Cpu->pc+1],  sum);
//	//A = (sum & 0xFF);
//	flags.CY = A < memory[m_Cpu->pc + 1];
//	updateFlags(sum);
//	m_Cpu->pc += 2;
//	opcodeCycleCount = 7;
//}
//
//// 0xFF | reset to 38H
//void i8080Emulator::RST7() {
//	RESET(0x38);
//}

#pragma endregion OpcodeFunctions

