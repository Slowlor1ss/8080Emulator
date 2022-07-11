#include "i8080Disassembler.h"
#include <bitset>
#include <fstream>
#include <cassert>
#include <format>
#include <iomanip>

i8080Emulator::i8080Emulator(const char* path) : m_Cpu(new CPU{this})
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

void i8080Emulator::CycleCpu()
{
	if (m_Cpu->pc >= MEMORY_SIZE) {
		std::cout << "Program counter overflow\n";
		m_Cpu->PrintRegister();
		m_Cpu->halt = true;
	}

	m_CurrentOpcode = m_Memory[m_Cpu->pc];
//#ifdef _DEBUG
//	std::cout << "opcode " << std::setw(2) << std::setfill('0') << std::hex <<
//		static_cast<int>(m_CurrentOpcode) << '\t' << OPCODES[m_CurrentOpcode].mnemonic << '\n';
//#endif

	if (m_Cpu->printDebug)
	{
		std::cout << "PC | opcode " << std::setw(2) << std::setfill('0') << std::hex <<
			static_cast<int>(m_Cpu->pc) << '\t' << OPCODES[m_CurrentOpcode].mnemonic << '\n';
	}

	(this->*OPCODES[m_CurrentOpcode].opcode)();

	m_Cpu->clockCount += InstructionCycles[m_CurrentOpcode];
}

void i8080Emulator::Interrupt(uint8_t ID)
{
	if (!m_Cpu->interruptsEnabled)
		return;

	m_Cpu->interruptsEnabled = false; //Disable Interrupts

	switch (ID)
	{
	case 0: // Mid Screen - RST 1
		m_CurrentOpcode = 0xCF;
		break;
	case 1: // End Screen - RST 2
		m_CurrentOpcode = 0xD7;
		break;
	default:
		assert(!"should never get here!");
		break;
	}

	//RST assumes current operation call was RST but as this is an interrupt thats not the case
	//and thus current operation still has to be called
	//so by decrementing the pc the correct operation will be saved to sp
	m_Cpu->pc -= 1;

	//call correct RST according to opcode set above
	RST();
}

//used for debugging
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

//safely writes to RAM
void i8080Emulator::MemWrite(uint16_t address, uint8_t data)
{
	if (address >= MEMORY_SIZE) {
		address = address % ROM_SIZE + ROM_SIZE;
	}
	if (address < ROM_SIZE) {
		std::cout << "Illegal address write:" << address << '\n';
		//__debugbreak();
	}
	else {
		m_Memory[address] = data;
	}

}

uint8_t i8080Emulator::ReadMem(uint16_t address)
{
	return m_Memory[address];
}

void i8080Emulator::defaultOpcode()
{
	std::cout << "Couldn't find opcode " << std::setw(2) << std::setfill('0') << std::hex << 
		static_cast<int>(m_CurrentOpcode) << '\t' << OPCODES[m_CurrentOpcode].mnemonic << '\n';
	m_Cpu->PrintRegister();
	__debugbreak();
}

//opcode implementations
#pragma region OpcodeFunctions

#pragma region GenericOpcodeFunctions
//fetches a PC from the stack
void i8080Emulator::RETURN(bool condition)
{
	if (condition) {
		//std::cout << "OP: " << std::hex << m_Cpu->pc << " | ";
		m_Cpu->pc = uint16_t(m_Memory[m_Cpu->sp + 1] << 8) | m_Memory[m_Cpu->sp];
		//std::cout << "return to: " << std::hex << m_Cpu->pc << '\n';
		m_Cpu->sp += 2;
	}
	else {
		m_Cpu->pc += 1;
	}
}

//behaves like call (saves pc to stack), but jumps to a specified number
void i8080Emulator::RESET(uint16_t callAddress)
{
	MemWrite((m_Cpu->sp - 1), ((m_Cpu->pc + 1) >> 8));
	MemWrite((m_Cpu->sp - 2), uint8_t(m_Cpu->pc + 1));
	m_Cpu->sp -= 2;
	m_Cpu->pc = callAddress;
}

//pushes PC, then sets PC
void i8080Emulator::CALLif(bool condition)
{
	if (condition) {
		//std::cout << "want to write: " << std::hex << m_Cpu->pc + 3;
		MemWrite((m_Cpu->sp - 1), ((m_Cpu->pc + 3) & 0xFF00) >> 8);
		MemWrite((m_Cpu->sp - 2), ((m_Cpu->pc + 3) & 0x00FF));
		//std::cout << " | wrote: : " << std::hex << (uint16_t(m_Memory[m_Cpu->sp - 1] << 8) | m_Memory[m_Cpu->sp - 2]) << '\n';
		m_Cpu->sp -= 2;

		m_Cpu->pc = uint16_t(m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1];
	}
	else {
		m_Cpu->pc += 3;
	}
}

void i8080Emulator::JUMP(bool condition)
{
	if (condition) {
		const uint16_t address = uint16_t(m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1];
		m_Cpu->pc = address;
	}
	else {
		m_Cpu->pc += 3;
	}
}

void i8080Emulator::POP()
{
	const RegisterPairs8080 pair = m_Cpu->GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint16_t value = uint16_t(m_Memory[m_Cpu->sp + 1] << 8) | m_Memory[m_Cpu->sp];

	if (pair == RegisterPairs8080::SP){ //special case in the pop operation sp is replaced by a and has special calculations see page 23 8080-Programmers-Manual
		m_Cpu->a = (value >> 8);
		m_Cpu->SetFlags(value & 0x00FF);
	}
	else
		m_Cpu->SetRegisterPair(pair, value);

	m_Cpu->sp += 2;
	m_Cpu->pc += 1;
}

//decrement register pair
void i8080Emulator::DCX()
{
	const RegisterPairs8080 pair = m_Cpu->GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint16_t result = m_Cpu->ReadRegisterPair(pair) - 1;
	m_Cpu->SetRegisterPair(pair, result);
	m_Cpu->pc += 1;
}

//add register pair to HL
void i8080Emulator::DAD()
{
	const RegisterPairs8080 pair = m_Cpu->GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint32_t result = m_Cpu->ReadRegisterPair(RegisterPairs8080::HL) + m_Cpu->ReadRegisterPair(pair);
	m_Cpu->SetRegisterPair(RegisterPairs8080::HL, uint16_t(result));
	m_Cpu->ConditionBits.c = (result > 0xFFFF);
	m_Cpu->pc += 1;
}

//decrements register
void i8080Emulator::DCR()
{
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode, 3);
	const uint8_t result = m_Cpu->ReadRegister(reg) - 1;
	m_Cpu->SetRegister(reg, result);
	m_Cpu->UpdateFlags(result);
	m_Cpu->pc += 1;
}

//sets a register to the byte after PC
void i8080Emulator::MVI()
{
	if (m_Cpu->pc == 0x151F)
	{
		std::cout << "Killing alien" << '\n';
	}
	if (m_Cpu->pc == 0x0798)
	{
		std::cout << "Killing alien" << '\n';
	}

	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode, 3);
	m_Cpu->SetRegister(reg, m_Memory[m_Cpu->pc + 1]);
	if(m_Cpu->pc == 0x1512 && m_Cpu->a == 0x05)
	{
		std::cout << "Flag alien explosion" << '\n';
	}
	m_Cpu->pc += 2;
}

//see page 3 8080-Programmers-Manual [STACK PUSH OPERATION]
void i8080Emulator::PUSH()
{
	const RegisterPairs8080 pair = m_Cpu->GetRegisterPairFromOpcode(m_CurrentOpcode);

	uint16_t value{};
	if (pair == RegisterPairs8080::SP) //special case in the push operation sp is replaced by a and has special calculations see page 22-23 8080-Programmers-Manual
		value = uint16_t(m_Cpu->a << 8) | m_Cpu->ReadFlags();
	else
		value = m_Cpu->ReadRegisterPair(pair);
		

	MemWrite((m_Cpu->sp - 1), (value & 0xFF00) >> 8);
	MemWrite((m_Cpu->sp - 2), (value & 0x00FF));
	m_Cpu->sp -= 2;
	m_Cpu->pc += 1;
}

//loads immediate into register pair
void i8080Emulator::LXI()
{
	//SetRegisterPair(pair, 
	//	(uint16_t(m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1])
	//);

	const RegisterPairs8080 pair = m_Cpu->GetRegisterPairFromOpcode(m_CurrentOpcode);
	m_Cpu->SetRegisterPair(pair,m_Memory[m_Cpu->pc + 1], m_Memory[m_Cpu->pc + 2]);

	m_Cpu->pc += 3;
}

//increments register pair
void i8080Emulator::INX()
{
	const RegisterPairs8080 pair = m_Cpu->GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint16_t result = m_Cpu->ReadRegisterPair(pair) + 1;
	m_Cpu->SetRegisterPair(pair, result);
	m_Cpu->pc += 1;
}

//increments given register by one
void i8080Emulator::INR()
{
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode, 3);
	const uint8_t result = m_Cpu->ReadRegister(reg) + 1;
	m_Cpu->SetRegister(reg, result);
	m_Cpu->UpdateFlags(result);
	m_Cpu->pc += 1;
}
#pragma endregion GenericOpcodeFunctions

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void i8080Emulator::NOP() {
	m_Cpu->pc += OPCODES[0x00].sizeBytes;
}

//
//// 0x02 | Write A to mem at address BC 
void i8080Emulator::STAXB() {
	MemWrite(m_Cpu->ReadRegisterPair(RegisterPairs8080::BC), m_Cpu->a);
	m_Cpu->pc += OPCODES[0x02].sizeBytes;
}

//// 0x07 | bit shift A left, bit 0 & Cy = prev bit 7
void i8080Emulator::RLC() {
	uint8_t oldBit7 = (m_Cpu->a & 0x80) >> 7;
	m_Cpu->a <<= 1;
	m_Cpu->a = m_Cpu->a | oldBit7;
	m_Cpu->ConditionBits.c = (1 == oldBit7);
	m_Cpu->pc += 1;
}

//// 0x0A | set register A to the contents or memory pointed by BC
void i8080Emulator::LDAXB() {
	m_Cpu->a = m_Memory[m_Cpu->ReadRegisterPair(RegisterPairs8080::BC)];
	m_Cpu->pc += 1;
}

//// 0x0F | rotates A right 1, bit 7 & CY = prev bit 0
void i8080Emulator::RRC() {
	uint8_t oldBit0 = (m_Cpu->a & 0x01);
	m_Cpu->a >>= 1;
	m_Cpu->a = m_Cpu->a | (oldBit0 << 7);
	m_Cpu->ConditionBits.c = (0x01 == oldBit0);
	m_Cpu->pc += OPCODES[0x0F].sizeBytes;
}
//
//// 0x10 | not an 8080 instruction, use as NOP
//
//// 0x11 | writes the bytes after m_Cpu->pc into the D register pair, E=pc+1, D=pc+2
//void i8080Emulator::LXID() {
//	LXI(RegisterPairs8080::DE);
//}
//
//// 0x12 | stores A into the address pointed to by the D reg pair
//// XXX no opcode count for stax
void i8080Emulator::STAXD() {
	MemWrite(m_Cpu->ReadRegisterPair(RegisterPairs8080::DE), m_Cpu->a);
	m_Cpu->pc += OPCODES[0x12].sizeBytes;
}

//// 0x1A | store the value at the memory referenced by DE in A
void i8080Emulator::LDAXD() {
	m_Cpu->a = m_Memory[m_Cpu->ReadRegisterPair(RegisterPairs8080::DE)];
	m_Cpu->pc += OPCODES[0x1A].sizeBytes;
}
//
//// 0x1B | decrease DE pair by 1
//void i8080Emulator::DCXD() {
//	DCX('D');
//}
//

//
//// 0x1D | decrease reg E by 1
//void i8080Emulator::DCRE() {
//	DCR(m_Cpu->e);
//}
////
////// 0x1E | move byte after m_Cpu->pc into reg E
//void i8080Emulator::MVIE() {
//	MVI(m_Cpu->e);
//}
//
//// 0x1F | rotate A right one, bit 7 = prev CY, CY = prevbit0
void i8080Emulator::RAR() {
	uint8_t oldA = m_Cpu->a;
	m_Cpu->a >>= 1;
	m_Cpu->a = m_Cpu->a | (m_Cpu->ConditionBits.c ? 0x80 : 0x00);
	m_Cpu->ConditionBits.c = (0x01 == (oldA & 0x01));
	m_Cpu->pc += 1;
}
//
//// 0x20 | not an 8080 code, use as NOP
//
//// 0x21 | loads data after m_Cpu->pc into H pair
//void i8080Emulator::LXIH() {
//	LXI(RegisterPairs8080::HL);
//}
//
//// 0x22 | stores HL into adress listed after m_Cpu->pc
//// adr <- L , adr+1 <- H
void i8080Emulator::SHLD() {
	uint16_t address = (m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1];
	MemWrite(address, m_Cpu->l);
	MemWrite(address + 1, m_Cpu->h);
	m_Cpu->pc += 3;
}

void i8080Emulator::DAA() {

	if (m_Cpu->pc == 0x099C)
	{
		std::cout << "DAA\n";
	}

	if (((m_Cpu->a & 0x0F) > 0x09) || m_Cpu->ConditionBits.ac) {
		//m_Cpu->ConditionBits.ac = true;
		m_Cpu->a += 0x06;
	}
	//else {
	//	m_Cpu->ConditionBits.ac = false;
	//}

	if (((m_Cpu->a & 0xF0) > 0x90) || m_Cpu->ConditionBits.c) { /*((m_Cpu->a >> 4) >= 9 && (m_Cpu->a & 0xF) > 9) || (m_Cpu->a >> 4) > 9*/
		m_Cpu->ConditionBits.c = true;
		m_Cpu->a += 0x60;
	}
	//else {
	//	m_Cpu->ConditionBits.c = false;
	//}
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

void i8080Emulator::LHLD() {
	uint16_t address = ((m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1]);
	m_Cpu->l = m_Memory[address];
	m_Cpu->h = m_Memory[address + 1];
	m_Cpu->pc += 3;
}


//// 0x2F | invert A
void i8080Emulator::CMA() {
	m_Cpu->a = ~m_Cpu->a;
	m_Cpu->pc += 1;
}
//
//// 0x30 | unimplemented in 8080, treat as NOP
//
//// 0x31 | load data after m_Cpu->pc as new Stack pointer
//void i8080Emulator::LXISP() {
//	LXI(RegisterPairs8080::SP);
//}
//
//// 0x32 | stores A into the address from bytes after m_Cpu->pc
//// adr.low = m_Cpu->pc+1, adr.hi = m_Cpu->pc+2
void i8080Emulator::STA() {
	const uint16_t address = uint16_t(m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1];
	MemWrite(address, m_Cpu->a);

	if (m_Cpu->pc == 0x0426 && m_Cpu->a != 0)
	{
		//__debugbreak();
		//m_Cpu->printDebug = true;
		std::cout << "Collision with alien" << "\n";
	}

	m_Cpu->pc += OPCODES[0x32].sizeBytes;
}

//// 0x37 | set carry flag to 1
void i8080Emulator::STC() {
	m_Cpu->ConditionBits.c = true;
	m_Cpu->pc += 1;
}

//// 0x3A | set reg A to the value pointed by bytes after m_Cpu->pc
void i8080Emulator::LDA() {
	m_Cpu->a = m_Memory[((m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1])];
	if (m_Cpu->pc == 0x14EA && m_Cpu->a == 1)
	{
		std::cout << "Alien is blowing up" << "\n";
	}
	m_Cpu->pc += OPCODES[0x3A].sizeBytes;
}
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
//	DCR(m_Cpu->a);
//}
////
////// 0x3E | moves next byte into A register
//void i8080Emulator::MVIA() {
//	MVI(m_Cpu->a);
//}
//
//// 0x3F | invert carry flag
//void i8080Emulator::CMC() {
//	flags.CY = !flags.CY;
//	m_Cpu->pc += 1;
//	opcodeCycleCount = 5;
//}
//

//TODO: clean up
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
//uint8_t* i8080Emulator::DecodeOpcodeRegister(uint8_t opcode3bit)
//{
//	switch (opcode3bit & 0b111)
//	{
//	case Registers8080::B:
//		return &m_Cpu->b;
//	case 0b001:
//		return &m_Cpu->c;
//	case 0b010:
//		return &m_Cpu->d;
//	case 0b011:
//		return &m_Cpu->e;
//	case 0b100:
//		return &m_Cpu->h;
//	case 0b101:
//		return &m_Cpu->l;
//	case 0b110:
//		return &(m_Memory[ReadRegisterPair(RegisterPairs8080::HL)]);
//	case 0b111:
//		return &m_Cpu->a;
//	default:
//		assert(!"should never get here!");
//		return nullptr;
//	}
//}

//// (0x40 - 0x75), (0x77 - 0x7F) | MOV
void i8080Emulator::MOV() {

	const auto src = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);
	const auto dest = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode, 3);

	if (m_Cpu->pc == 0x099A)
	{
		std::cout << "set score to: " << int(m_Cpu->ReadRegister(src)) << '\n';
	}

	m_Cpu->SetRegister(dest, m_Cpu->ReadRegister(src));
	m_Cpu->pc += 1;
}
////
//////////////////////
//////////////
//// 0x76 | XXX not done yet
void i8080Emulator::HLT() {
	m_Cpu->halt = true;
	m_Cpu->pc += 1;
}

void i8080Emulator::ADD() {
	if (m_Cpu->pc == 0x099B)
	{
		std::cout << "add\n";
	}

	Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	uint16_t sum = m_Cpu->a + m_Cpu->ReadRegister(reg);
	m_Cpu->ConditionBits.ac = (sum ^ m_Cpu->a ^ m_Cpu->ReadRegister(reg)) & (1 << 4);
	m_Cpu->ConditionBits.c = sum > 0xFF;
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

void i8080Emulator::ADC() {
	Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	uint16_t sum = m_Cpu->a + m_Cpu->ReadRegister(reg) + (uint8_t)m_Cpu->ConditionBits.c;
	m_Cpu->ConditionBits.ac = (sum ^ m_Cpu->a ^ m_Cpu->ReadRegister(reg)) & (1 << 4);
	m_Cpu->ConditionBits.c = sum > 0xFF;
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

void i8080Emulator::SUB() {
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);


	// 2s complement subtraction, done in 4bit amounts, for AC
	const uint8_t decrement = ~m_Cpu->ReadRegister(reg);
	uint8_t lowerDec = decrement & 0x0F;
	uint8_t lowerA = m_Cpu->a & 0x0F;
	uint8_t lowerSum = lowerDec + lowerA + 1;
	m_Cpu->ConditionBits.ac = (lowerSum & (1 << 4)) != 0;//lowerSum > 0xF;

	uint8_t upperDec = (decrement & 0xF0) >> 4;
	uint8_t upperA = (m_Cpu->a & 0xF0) >> 4;
	uint8_t upperSum = upperDec + upperA + ((lowerSum & 0x10) >> 4);
	m_Cpu->ConditionBits.c = (0x10 != (upperSum & 0x10));
	m_Cpu->a = (upperSum << 4) + lowerSum;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

void i8080Emulator::SBB() {
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	// 2s complement subtraction, done in 4bit amounts, for AC
	const uint8_t result = ~m_Cpu->ReadRegister(reg);
	uint8_t lowerDec = result & 0x0F;
	uint8_t lowerA = m_Cpu->a & 0x0F;
	uint8_t lowerSum = lowerDec + lowerA + 1 + (uint8_t)m_Cpu->ConditionBits.c;
	m_Cpu->ConditionBits.ac = (lowerSum & (1 << 4)) != 0; //> 0xF;

	uint8_t upperDec = (result & 0xF0) >> 4;
	uint8_t upperA = (m_Cpu->a & 0xF0) >> 4;
	uint8_t upperSum = upperDec + upperA + ((lowerSum & 0x10) >> 4);
	m_Cpu->ConditionBits.c = (0x10 != (upperSum & 0x10));
	m_Cpu->a = (upperSum << 4) + lowerSum;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

void i8080Emulator::ANA() {
	if(m_Cpu->pc == 0x151B)
	{
		std::cout << "Checking if alien is alive" << "\n";
	}

	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a & m_Cpu->ReadRegister(reg);
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = false;
	m_Cpu->UpdateFlags(m_Cpu->a);

	m_Cpu->pc += 1;
}

void i8080Emulator::XRA() {
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a ^ m_Cpu->ReadRegister(reg);
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = false;
	m_Cpu->UpdateFlags(m_Cpu->a);

	m_Cpu->pc += 1;
}

void i8080Emulator::ORA() {
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a | m_Cpu->ReadRegister(reg);
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = false;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

void i8080Emulator::CMP() {
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a - m_Cpu->ReadRegister(reg);
	m_Cpu->ConditionBits.ac = (result ^ m_Cpu->a ^ m_Cpu->ReadRegister(reg)) & (1 << 4);
	m_Cpu->ConditionBits.c = (result & 0xFF00) != 0;
	m_Cpu->UpdateFlags(uint8_t(result));

	if (m_Cpu->pc == 0x1500)
	{
		std::cout << "Compaire to shots cord's \n";
	}

	m_Cpu->pc += 1;
}

// 0xC0 | return on nonzero
void i8080Emulator::RNZ() {
	RETURN((m_Cpu->ConditionBits.z == false));
}

void i8080Emulator::JNZ() {
	JUMP(m_Cpu->ConditionBits.z == false);
}

void i8080Emulator::JMP() {
	JUMP(true);
}

void i8080Emulator::CNZ() {
	CALLif(m_Cpu->ConditionBits.z == false);
}

void i8080Emulator::ADI() {
	uint16_t sum = m_Cpu->a + m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->ConditionBits.c = sum > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += OPCODES[0xC6].sizeBytes;
}
//
//// 0xC7 | calls to 0H 
void i8080Emulator::RST() {
	const uint8_t address = (m_CurrentOpcode & 0b00111000);
	RESET(address);
}
//
//// 0xC8 | return if Z
void i8080Emulator::RZ() {
	RETURN(m_Cpu->ConditionBits.z);
}
//
//// 0xC9 | standard return
void i8080Emulator::RET() {
	RETURN(true);
}
//
//// 0xCA |
//Jump if zero
void i8080Emulator::JZ() {
	JUMP(m_Cpu->ConditionBits.z);
}

void i8080Emulator::CZ() {
	CALLif(m_Cpu->ConditionBits.z);
}
//
//// 0xCD | store m_Cpu->pc on stack, and jump to a new location
void i8080Emulator::CALL() {
	CALLif(true);
}

void i8080Emulator::RNC() {
	RETURN(m_Cpu->ConditionBits.c == false);
}

void i8080Emulator::JNC() {
	JUMP(m_Cpu->ConditionBits.c == false);
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
//
//// 0xD4 | call on carry = false
void i8080Emulator::CNC() {
	CALLif((m_Cpu->ConditionBits.c == false));
}

void i8080Emulator::SUI() {
	uint16_t result = m_Cpu->a - m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (result & 0xFF);
	m_Cpu->ConditionBits.c = result > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 2;
}

void i8080Emulator::RC() {
	RETURN(m_Cpu->ConditionBits.c);
}

void i8080Emulator::JC() {
	JUMP(m_Cpu->ConditionBits.c);
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
//
//// 0xDC | call if cy =1
void i8080Emulator::CC() {
	CALLif(m_Cpu->ConditionBits.c);
}
//
//// 0xDD | not 8080 code
//
//// 0xDE | sub byte and cy from A
void i8080Emulator::SBI() {
	uint16_t sum = m_Cpu->a - m_Memory[m_Cpu->pc + 1] - (uint8_t)m_Cpu->ConditionBits.c;
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->ConditionBits.c = sum > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 2;
}

void i8080Emulator::RPO() {
	RETURN(m_Cpu->ConditionBits.p == 0);
}

void i8080Emulator::JPO() {
	JUMP(m_Cpu->ConditionBits.p == 0);
}
//
//// 0xE3 | exchange HL and SP data
//// L <-> (SP) | H <-> (SP+1)
void i8080Emulator::XTHL() {
	const uint16_t stackContents = (m_Memory[m_Cpu->sp + 1] << 8) | m_Memory[m_Cpu->sp];
	MemWrite(m_Cpu->sp, m_Cpu->l);
	MemWrite(m_Cpu->sp + 1, m_Cpu->h);
	m_Cpu->SetRegisterPair(RegisterPairs8080::HL, stackContents);
	m_Cpu->pc += 1;
}
//
//// 0xE4 | call if p = 0
void i8080Emulator::CPO() {
	CALLif(m_Cpu->ConditionBits.p == 0);
}

void i8080Emulator::ANI() {
	uint16_t result = m_Cpu->a & m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = result > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += OPCODES[0xE6].sizeBytes;
}

void i8080Emulator::RPE() {
	RETURN(m_Cpu->ConditionBits.p);
}

void i8080Emulator::PCHL() {
	m_Cpu->pc = m_Cpu->ReadRegisterPair(RegisterPairs8080::HL);
}

void i8080Emulator::JPE() {
	JUMP(m_Cpu->ConditionBits.p);
}
//
//// 0xEB | exchange HL and DE
void i8080Emulator::XCHG() {
	uint16_t oldDE = m_Cpu->ReadRegisterPair(RegisterPairs8080::DE);
	m_Cpu->SetRegisterPair(RegisterPairs8080::DE, m_Cpu->l, m_Cpu->h);//uint16_t(m_Cpu->h << 8) | m_Cpu->l);
	m_Cpu->SetRegisterPair(RegisterPairs8080::HL, oldDE);
	m_Cpu->pc += OPCODES[0xEB].sizeBytes;
}
//
//// 0xEC | call if p = 1
void i8080Emulator::CPE() {
	CALLif(m_Cpu->ConditionBits.p);
}

void i8080Emulator::RP() {
	RETURN(m_Cpu->ConditionBits.s == false);
}


void i8080Emulator::JP() {
	JUMP(m_Cpu->ConditionBits.s == false);
}
//
//// 0xF3 | Disable Interrupts
void i8080Emulator::DI() {
	m_Cpu->interruptsEnabled = false;
	m_Cpu->pc += 1;
}
//
//call if positive
void i8080Emulator::CP() {
	CALLif(m_Cpu->ConditionBits.s == false);
}

void i8080Emulator::ORI() {
	const uint16_t result = m_Cpu->a | m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (result & 0xFF);
	m_Cpu->ConditionBits.c = result > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 2;
}

void i8080Emulator::RM() {
	RETURN(m_Cpu->ConditionBits.s == true);
}

void i8080Emulator::JM() {
	JUMP(m_Cpu->ConditionBits.s == true);
}
//
//// 0xFB | Enable Interrrupts
void i8080Emulator::EI() {
	m_Cpu->interruptsEnabled = true;
	m_Cpu->pc += 1;
}
//
//// 0xFC | call if minus
void i8080Emulator::CM() {
	CALLif(m_Cpu->ConditionBits.s == true);
}
//
//// 0xFD | not 8080 code

//ComPare Immediate with Accumulator
//See page 29 8080-Programmers-Manual
void i8080Emulator::CPI() {
	if (m_Cpu->pc == 0x03D2 && m_Cpu->a == 5)
	{
		std::cout << "Shot blowing up not bc of alien 03D2" << "\n";
	}

	const uint8_t result = m_Cpu->a - m_Memory[m_Cpu->pc + 1];
	m_Cpu->ConditionBits.c = m_Cpu->a < m_Memory[m_Cpu->pc + 1];
	m_Cpu->UpdateFlags(result);
	if (m_Cpu->pc == 0x14FB)
	{
		std::cout << "Alien down in shield? 14FB" << std::boolalpha << bool(!m_Cpu->ConditionBits.c) << "\n";
	}
	m_Cpu->pc += OPCODES[0xFE].sizeBytes;
}


#pragma endregion OpcodeFunctions

