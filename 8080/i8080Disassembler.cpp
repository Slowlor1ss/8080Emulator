#include "i8080Disassembler.h"
#include <bitset>
#include <fstream>
#include <cassert>
#include <format>
#include <iomanip>

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
	m_CurrRomSize = file.tellg();
	file.seekg(0, std::ios::beg);

	if (m_CurrRomSize > ROM_SIZE)
	{
		std::cerr << "Rom is too big";
		exit(EXIT_FAILURE);
	}

	m_Memory = new uint8_t[MEMORY_SIZE];

	file.read(reinterpret_cast<char*>(m_Memory), m_CurrRomSize);

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
//#ifdef _DEBUG
//	std::cout << "opcode " << std::setw(2) << std::setfill('0') << std::hex <<
//		static_cast<int>(m_CurrentOpcode) << '\t' << OPCODES[m_CurrentOpcode].mnemonic << '\n';
//#endif
	(this->*OPCODES[m_CurrentOpcode].opcode)();
	return true;
}

//Used for debugging
void i8080Emulator::PrintRegister() const
{
	std::cout << "Main registers\n";
	std::cout << "A: " << std::bitset<8>(m_Cpu->a) << " | " << "Flags: " << std::bitset<8>((m_Cpu->ConditionBits.s << 7) | (m_Cpu->ConditionBits.z << 6) | (m_Cpu->ConditionBits.ac << 4) | (m_Cpu->ConditionBits.p << 2) | (m_Cpu->ConditionBits.c << 0)) << '\n';
	std::cout << "B: " << std::bitset<8>(m_Cpu->b) << " | " << "C: " << std::bitset<8>(m_Cpu->c)  <<'\n';
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

	while (pc < m_CurrRomSize)
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

uint8_t i8080Emulator::ReadFlags() const
{
	return static_cast<uint8_t>(
		  (m_Cpu->ConditionBits.s << 7)
		| (m_Cpu->ConditionBits.z << 6)
		| (m_Cpu->ConditionBits.ac << 4)
		| (m_Cpu->ConditionBits.p << 2)
		| (m_Cpu->ConditionBits.c << 0));
}

void i8080Emulator::SetFlags(uint8_t data)
{
	m_Cpu->ConditionBits.s = data & 0b10000000;
	m_Cpu->ConditionBits.z = data & 0b01000000;
	m_Cpu->ConditionBits.ac = data & 0b00010000;
	m_Cpu->ConditionBits.p = data & 0b0100;
	m_Cpu->ConditionBits.c = data & 0b0001;
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
	const uint8_t MSByte = (value & 0xFF00) >> 8;
	const uint8_t LSByte = (value & 0x00FF);

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

uint8_t i8080Emulator::ReadRegister(Registers8080 reg) const
{
	switch (reg)
	{
	case Registers8080::B:
		return m_Cpu->b;
	case Registers8080::C:
		return m_Cpu->c;
	case Registers8080::D:
		return m_Cpu->d;
	case Registers8080::E:
		return m_Cpu->e;
	case Registers8080::H:
		return m_Cpu->h;
	case Registers8080::L:
		return m_Cpu->l;
	case Registers8080::MEM:
		return (m_Memory[ReadRegisterPair(RegisterPairs8080::HL)]);
	case Registers8080::A:
		return m_Cpu->a;
	default:
		assert(!"should never get here!");
		return 0;
	}


}///////////////////////

void i8080Emulator::SetRegister(Registers8080 reg, uint8_t value)
{
	switch (reg)
	{
	case Registers8080::B:
		m_Cpu->b = value;
		break;
	case Registers8080::C:
		m_Cpu->c = value;
		break;
	case Registers8080::D:
		m_Cpu->d = value;
		break;
	case Registers8080::E:
		m_Cpu->e = value;
		break;
	case Registers8080::H:
		m_Cpu->h = value;
		break;
	case Registers8080::L:
		m_Cpu->l = value;
		break;
	case Registers8080::MEM:
		MemWrite(ReadRegisterPair(RegisterPairs8080::HL), value);
		break;
	case Registers8080::A:
		m_Cpu->a = value;
		break;
	default:
		assert(!"should never get here!");
		break;
	}
}

RegisterPairs8080 i8080Emulator::GetRegisterPairFromOpcode(const uint8_t opcode) const
{
	//based 8080-Programmers-Manual page 25
	return static_cast<RegisterPairs8080>((opcode & 0b00110000) >> 4);
}

Registers8080 i8080Emulator::GetRegisterFromOpcode(uint8_t opcode, uint8_t shift) const
{
	//based 8080-Programmers-Manual page 16-17
	//shift can be specified as some functions like INR DCR MVI use the second 3 bits instead of the first 3
	return static_cast<Registers8080>((opcode & (0b00000111 << shift)) >> shift);
}

//opcode implementations
#pragma region OpcodeFunctions

#pragma region GenericOpcodeFunctions
//fetches a PC from the stack
void i8080Emulator::RETURN(bool condition)
{
	if (condition) {
		m_Cpu->pc = uint16_t(m_Memory[m_Cpu->sp + 1] << 8) | m_Memory[m_Cpu->sp];
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

void i8080Emulator::POP()
{
	const RegisterPairs8080 pair = GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint16_t value = uint16_t(m_Memory[m_Cpu->sp + 1] << 8) | m_Memory[m_Cpu->sp];

	if (pair == RegisterPairs8080::SP){ //special case in the pop operation sp is replaced by a and has special calculations see page 23 8080-Programmers-Manual
		m_Cpu->a = (value >> 8);
		SetFlags(value & 0x00FF);
	}
	else
		SetRegisterPair(pair, value);

	m_Cpu->sp += 2;
	m_Cpu->pc += 1;
}

//decrement register pair
void i8080Emulator::DCX()
{
	const RegisterPairs8080 pair = GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint16_t result = ReadRegisterPair(pair) - 1;
	SetRegisterPair(pair, result);
	m_Cpu->pc += 1;
}

//add register pair to HL
void i8080Emulator::DAD()
{
	const RegisterPairs8080 pair = GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint32_t result = ReadRegisterPair(RegisterPairs8080::HL) + ReadRegisterPair(pair);
	SetRegisterPair(RegisterPairs8080::HL, uint16_t(result));
	m_Cpu->ConditionBits.c = (result > 0xFFFF);
	m_Cpu->pc += 1;
}

//decrements register
void i8080Emulator::DCR()
{
	const Registers8080 reg = GetRegisterFromOpcode(m_CurrentOpcode, 3);
	const uint8_t result = ReadRegister(reg) - 1;
	SetRegister(reg, result);
	UpdateFlags(result);
	m_Cpu->pc += 1;
}

//sets a register to the byte after PC
void i8080Emulator::MVI()
{
	const Registers8080 reg = GetRegisterFromOpcode(m_CurrentOpcode, 3);
	SetRegister(reg, m_Memory[m_Cpu->pc + 1]);
	m_Cpu->pc += 2;
}

//see page 3 8080-Programmers-Manual [STACK PUSH OPERATION]
void i8080Emulator::PUSH()
{
	const RegisterPairs8080 pair = GetRegisterPairFromOpcode(m_CurrentOpcode);

	uint16_t value{};
	if (pair == RegisterPairs8080::SP) //special case in the push operation sp is replaced by a and has special calculations see page 22-23 8080-Programmers-Manual
		value = uint16_t(m_Cpu->a << 8) | ReadFlags();
	else
		value = ReadRegisterPair(pair);
		

	MemWrite((m_Cpu->sp - 1), (value & 0xFF00) >> 8);
	MemWrite((m_Cpu->sp - 2), (value & 0x00FF));
	m_Cpu->sp -= 2;
	m_Cpu->pc += 1;
}

//loads immediate into register pair
void i8080Emulator::LXI()
{
	//SetRegisterPair(pair, 
	//	(uint16_t(m_Memory[m_Cpu->pc + 1] << 8) | m_Memory[m_Cpu->pc + 2])
	//);

	const RegisterPairs8080 pair = GetRegisterPairFromOpcode(m_CurrentOpcode);
	SetRegisterPair(pair,m_Memory[m_Cpu->pc + 1], m_Memory[m_Cpu->pc + 2]);

	m_Cpu->pc += 3;
}

//increments register pair
void i8080Emulator::INX()
{
	const RegisterPairs8080 pair = GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint16_t result = ReadRegisterPair(pair) + 1;
	SetRegisterPair(pair, result);
	m_Cpu->pc += 1;
}

//increments given register by one
void i8080Emulator::INR()
{
	const Registers8080 reg = GetRegisterFromOpcode(m_CurrentOpcode, 3);
	const uint8_t result = ReadRegister(reg) + 1;
	SetRegister(reg, result);
	UpdateFlags(result);
	m_Cpu->pc += 1;
}
#pragma endregion GenericOpcodeFunctions

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void i8080Emulator::NOP() {
	m_Cpu->pc += OPCODES[0x00].sizeBytes;
}

void i8080Emulator::STAXB() {
	MemWrite(ReadRegisterPair(RegisterPairs8080::BC), m_Cpu->a);
	m_Cpu->pc += OPCODES[0x02].sizeBytes;
}

void i8080Emulator::LDAXB() {
	m_Cpu->a = m_Memory[ReadRegisterPair(RegisterPairs8080::BC)];
	m_Cpu->pc += 1;
}

void i8080Emulator::RRC() {
	uint8_t oldBit0 = (m_Cpu->a & 0x01);
	m_Cpu->a >>= 1;
	m_Cpu->a = m_Cpu->a | (oldBit0 << 7);
	m_Cpu->ConditionBits.c = (0x01 == oldBit0);
	m_Cpu->pc += OPCODES[0x0F].sizeBytes;
}

void i8080Emulator::STAXD() {
	MemWrite(ReadRegisterPair(RegisterPairs8080::DE), m_Cpu->a);
	m_Cpu->pc += OPCODES[0x12].sizeBytes;
}

void i8080Emulator::LDAXD() {
	m_Cpu->a = m_Memory[ReadRegisterPair(RegisterPairs8080::DE)];
	m_Cpu->pc += OPCODES[0x1A].sizeBytes;
}

void i8080Emulator::STA() {
	uint16_t address = uint16_t(m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1];
	MemWrite(address, m_Cpu->a);
	m_Cpu->pc += OPCODES[0x32].sizeBytes;
}

void i8080Emulator::LDA() {
	m_Cpu->a = m_Memory[((m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1])];
	m_Cpu->pc += OPCODES[0x3A].sizeBytes;
}

void i8080Emulator::MOV() {
	const auto src = GetRegisterFromOpcode(m_CurrentOpcode);// static_cast<Registers8080>(m_CurrentOpcode & 0b00000111);
	const auto dest = GetRegisterFromOpcode(m_CurrentOpcode, 3);//static_cast<Registers8080>((m_CurrentOpcode & 0b00111000) >> 3);

	SetRegister(dest, ReadRegister(src));
	m_Cpu->pc += 1;
}

void i8080Emulator::SBB() {
	const Registers8080 reg = GetRegisterFromOpcode(m_CurrentOpcode);

	// 2s complement subtraction, done in 4bit amounts, for AC
	const uint16_t result = ~ReadRegister(reg);
	uint8_t lowerDec = result & 0x0F;
	uint8_t lowerA = m_Cpu->a & 0x0F;
	uint8_t lowerSum = lowerDec + lowerA + 1 + (uint8_t)m_Cpu->ConditionBits.c;
	m_Cpu->ConditionBits.ac = lowerSum > 0xF;

	uint8_t upperDec = (result & 0xF0) >> 4;
	uint8_t upperA = (m_Cpu->a & 0xF0) >> 4;
	uint8_t upperSum = upperDec + upperA + ((lowerSum & 0x10) >> 4);
	m_Cpu->ConditionBits.c = (0x10 != (upperSum & 0x10));
	m_Cpu->a = (upperSum << 4) + lowerSum;
	UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

void i8080Emulator::ANA() {
	const Registers8080 reg = GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a & ReadRegister(reg);
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = false;
	UpdateFlags(m_Cpu->a);

	m_Cpu->pc += 1;
}

void i8080Emulator::XRA() {
	const Registers8080 reg = GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a ^ ReadRegister(reg);
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = false;
	UpdateFlags(m_Cpu->a);

	m_Cpu->pc += 1;
}

void i8080Emulator::ORA() {
	const Registers8080 reg = GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a | ReadRegister(reg);
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = false;
	UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

void i8080Emulator::CMP() {
	const Registers8080 reg = GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a - ReadRegister(reg);
	m_Cpu->ConditionBits.c = (result & 0xFF00) == 0;
	UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

void i8080Emulator::RNZ() {
	RETURN((m_Cpu->ConditionBits.z == false));
}

void i8080Emulator::JNZ() {
	JUMP(!m_Cpu->ConditionBits.z);
}

void i8080Emulator::JMP() {
	JUMP(true);
}

void i8080Emulator::ADI() {
	uint16_t sum = m_Cpu->a + m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->ConditionBits.c = sum > 0xFF00;
	UpdateFlags(m_Cpu->a);
	m_Cpu->pc += OPCODES[0xC6].sizeBytes;
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

//https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#dedicated-shift-hardware
void i8080Emulator::OUT() {
	uint8_t port = m_Memory[m_Cpu->pc + 1];

	if (port == 2){
		m_Cpu->shiftOffset = m_Cpu->a & 7;
	}
	else if (port == 4){
		m_Cpu->regShift >>= 8;
		m_Cpu->regShift |= (m_Cpu->a << 8);
	}
	else{
		m_Cpu->outPort[port] = m_Cpu->a;
	}

	m_Cpu->pc += OPCODES[0xD3].sizeBytes;
}

//https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#inputs
void i8080Emulator::IN() {
	uint8_t port = m_Memory[m_Cpu->pc + 1];

	if (port == 3){
		m_Cpu->a = uint8_t(m_Cpu->regShift >> (8 - m_Cpu->shiftOffset));
	}
	else{
		m_Cpu->a = m_Cpu->inPort[port];
	}

	m_Cpu->pc += OPCODES[0xDB].sizeBytes;
}

void i8080Emulator::ANI() {
	uint16_t result = m_Cpu->a & m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = result > 0xFF00;
	UpdateFlags(m_Cpu->a);
	m_Cpu->pc += OPCODES[0xE6].sizeBytes;
}

void i8080Emulator::XCHG() {
	uint16_t oldDE = ReadRegisterPair(RegisterPairs8080::DE);
	SetRegisterPair(RegisterPairs8080::DE, m_Cpu->l, m_Cpu->h);//uint16_t(m_Cpu->h << 8) | m_Cpu->l);
	SetRegisterPair(RegisterPairs8080::HL, oldDE);
	m_Cpu->pc += OPCODES[0xEB].sizeBytes;
}

void i8080Emulator::DI() {
	m_Cpu->interruptsEnabled = false;
	m_Cpu->pc += 1;
}
//
//call if positive
void i8080Emulator::CP() {
	CALLif(m_Cpu->ConditionBits.s == false);
}

void i8080Emulator::EI() {
	m_Cpu->interruptsEnabled = true;
	m_Cpu->pc += 1;
}

//ComPare Immediate with Accumulator
//See page 29 8080-Programmers-Manual
void i8080Emulator::CPI() {
	uint8_t result = m_Cpu->a - m_Memory[m_Cpu->pc + 1];
	m_Cpu->ConditionBits.c = m_Memory[m_Cpu->pc + 1] > m_Cpu->a;
	UpdateFlags(result);
	m_Cpu->pc += OPCODES[0xFE].sizeBytes;
}

#pragma endregion OpcodeFunctions

