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
		__debugbreak();
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
		m_Cpu->pc = uint16_t(m_Memory[m_Cpu->sp + 1] << 8) | m_Memory[m_Cpu->sp];
		m_Cpu->sp += 2;
	}
	else {
		m_Cpu->pc += 1;
	}
}

//the contents of the program counter
//are pushed onto the stack, providing a return address for
//later use by a RETURN instruction
void i8080Emulator::RESTART(uint16_t callAddress)
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
		MemWrite((m_Cpu->sp - 1), ((m_Cpu->pc + 3) & 0xFF00) >> 8);
		MemWrite((m_Cpu->sp - 2), ((m_Cpu->pc + 3) & 0x00FF));
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
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode, 3);
	m_Cpu->SetRegister(reg, m_Memory[m_Cpu->pc + 1]);
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

//write A to mem at address BC 
void i8080Emulator::STAXB() {
	MemWrite(m_Cpu->ReadRegisterPair(RegisterPairs8080::BC), m_Cpu->a);
	m_Cpu->pc += OPCODES[0x02].sizeBytes;
}

//bit shift A left, bit 0 & Cy = prev bit 7
void i8080Emulator::RLC() {
	uint8_t oldBit7 = (m_Cpu->a & 128) >> 7;
	m_Cpu->a <<= 1;
	m_Cpu->a = m_Cpu->a | oldBit7;
	m_Cpu->ConditionBits.c = (1 == oldBit7);
	m_Cpu->pc += 1;
}

//set register A to the contents or memory pointed by BC
void i8080Emulator::LDAXB() {
	m_Cpu->a = m_Memory[m_Cpu->ReadRegisterPair(RegisterPairs8080::BC)];
	m_Cpu->pc += 1;
}

//rotates A right 1, bit 7 & CY = prev bit 0
void i8080Emulator::RRC() {
	uint8_t oldBit0 = (m_Cpu->a & 1);
	m_Cpu->a >>= 1;
	m_Cpu->a = m_Cpu->a | (oldBit0 << 7);
	m_Cpu->ConditionBits.c = (1 == oldBit0);
	m_Cpu->pc += OPCODES[0x0F].sizeBytes;
}

//stores A into the address pointed to by the D reg pair
void i8080Emulator::STAXD() {
	MemWrite(m_Cpu->ReadRegisterPair(RegisterPairs8080::DE), m_Cpu->a);
	m_Cpu->pc += OPCODES[0x12].sizeBytes;
}

//shifts A Left 1, bit 0 = prev CY, CY = prev bit 7
void i8080Emulator::RAL() {
	uint8_t oldA = m_Cpu->a;
	m_Cpu->a <<= 1;
	m_Cpu->a = m_Cpu->a | m_Cpu->ConditionBits.c;
	m_Cpu->ConditionBits.c = (oldA >= 128);
	m_Cpu->pc += OPCODES[0x17].sizeBytes;
}

//store the value at the memory referenced by DE in A
void i8080Emulator::LDAXD() {
	m_Cpu->a = m_Memory[m_Cpu->ReadRegisterPair(RegisterPairs8080::DE)];
	m_Cpu->pc += OPCODES[0x1A].sizeBytes;
}

//rotate A right one, bit 7 = prev CY, CY = prevbit0
void i8080Emulator::RAR() {
	uint8_t oldA = m_Cpu->a;
	m_Cpu->a >>= 1;
	m_Cpu->a = m_Cpu->a | (m_Cpu->ConditionBits.c ? 0x80 : 0x00);
	m_Cpu->ConditionBits.c = (0x01 == (oldA & 0x01));
	m_Cpu->pc += 1;
}

//stores HL into address listed after m_Cpu->pc
//adr <- L , adr+1 <- H
void i8080Emulator::SHLD() {
	uint16_t address = (m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1];
	MemWrite(address, m_Cpu->l);
	MemWrite(address + 1, m_Cpu->h);
	m_Cpu->pc += 3;
}

//The eight-bit hexadecimal number in the
//accumulator is adjusted to form two four-bit binary coded decimals
void i8080Emulator::DAA() {

	if (((m_Cpu->a & 0x0F) > 0x09) || m_Cpu->ConditionBits.ac) {
		m_Cpu->a += 0x06;
	}

	if (((m_Cpu->a & 0xF0) > 0x90) || m_Cpu->ConditionBits.c) {
		m_Cpu->ConditionBits.c = true;
		m_Cpu->a += 0x60;
	}

	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

//read memory from 2bytes after m_Cpu->pc into HL
// L <- adr, H <- adr+1
void i8080Emulator::LHLD() {
	uint16_t address = ((m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1]);
	m_Cpu->l = m_Memory[address];
	m_Cpu->h = m_Memory[address + 1];
	m_Cpu->pc += 3;
}

//invert A
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

	m_Cpu->pc += OPCODES[0x32].sizeBytes;
}

//set carry flag to 1
void i8080Emulator::STC() {
	m_Cpu->ConditionBits.c = true;
	m_Cpu->pc += 1;
}

//set reg A to the value pointed by bytes after m_Cpu->pc
void i8080Emulator::LDA() {
	m_Cpu->a = m_Memory[((m_Memory[m_Cpu->pc + 2] << 8) | m_Memory[m_Cpu->pc + 1])];
	m_Cpu->pc += OPCODES[0x3A].sizeBytes;
}

//invert carry flag
void i8080Emulator::CMC() {
	m_Cpu->ConditionBits.c = !m_Cpu->ConditionBits.c;
	m_Cpu->pc += 1;
}

void i8080Emulator::MOV() {

	const auto src = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);
	const auto dest = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode, 3);

	m_Cpu->SetRegister(dest, m_Cpu->ReadRegister(src));
	m_Cpu->pc += 1;
}

//halt
void i8080Emulator::HLT() {
	m_Cpu->halt = true;
	m_Cpu->pc += 1;
}

//add a register/mem value onto A, and update all flags
void i8080Emulator::ADD() {
	Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	uint16_t sum = m_Cpu->a + m_Cpu->ReadRegister(reg);
	m_Cpu->ConditionBits.ac = (sum ^ m_Cpu->a ^ m_Cpu->ReadRegister(reg)) & (1 << 4);
	m_Cpu->ConditionBits.c = sum > 0xFF;
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

//add a register/mem value + carry bit onto A, update registers
void i8080Emulator::ADC() {
	Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	uint16_t sum = m_Cpu->a + m_Cpu->ReadRegister(reg) + (uint8_t)m_Cpu->ConditionBits.c;
	m_Cpu->ConditionBits.ac = (sum ^ m_Cpu->a ^ m_Cpu->ReadRegister(reg)) & (1 << 4);
	m_Cpu->ConditionBits.c = sum > 0xFF;
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

//sub a register/mem value from A, and update all flags
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

//add a register/mem value - carry bit onto A
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

//bitwise AND a register/mem value with A
void i8080Emulator::ANA() {
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a & m_Cpu->ReadRegister(reg);
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = false;
	m_Cpu->UpdateFlags(m_Cpu->a);

	m_Cpu->pc += 1;
}

//bitwise XOR a register/mem value with A, update registers
void i8080Emulator::XRA() {
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a ^ m_Cpu->ReadRegister(reg);
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = false;
	m_Cpu->UpdateFlags(m_Cpu->a);

	m_Cpu->pc += 1;
}

//bitwise OR a register/mem value with A, update registers
void i8080Emulator::ORA() {
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a | m_Cpu->ReadRegister(reg);
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = false;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 1;
}

//compare a register/mem value with A, update registers
void i8080Emulator::CMP() {
	const Registers8080 reg = m_Cpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_Cpu->a - m_Cpu->ReadRegister(reg);
	m_Cpu->ConditionBits.ac = (result ^ m_Cpu->a ^ m_Cpu->ReadRegister(reg)) & (1 << 4);
	m_Cpu->ConditionBits.c = (result & 0xFF00) != 0;
	m_Cpu->UpdateFlags(uint8_t(result));

	m_Cpu->pc += 1;
}

//return on nonzero
void i8080Emulator::RNZ() {
	RETURN((m_Cpu->ConditionBits.z == false));
}

//jump to address from after m_Cpu->pc if non zero
void i8080Emulator::JNZ() {
	JUMP(m_Cpu->ConditionBits.z == false);
}

//normal jump
void i8080Emulator::JMP() {
	JUMP(true);
}

//call if not zero
void i8080Emulator::CNZ() {
	CALLif(m_Cpu->ConditionBits.z == false);
}
//adds a byte onto A, fetched after m_Cpu->pc
void i8080Emulator::ADI() {
	uint16_t sum = m_Cpu->a + m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->ConditionBits.c = sum > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += OPCODES[0xC6].sizeBytes;
}

void i8080Emulator::RST() {
	const uint8_t address = (m_CurrentOpcode & 0b00111000);
	RESTART(address);
}

//return if Z
void i8080Emulator::RZ() {
	RETURN(m_Cpu->ConditionBits.z);
}

//return
void i8080Emulator::RET() {
	RETURN(true);
}

//jump if zero
void i8080Emulator::JZ() {
	JUMP(m_Cpu->ConditionBits.z);
}

//call if Z flag set
void i8080Emulator::CZ() {
	CALLif(m_Cpu->ConditionBits.z);
}

//store m_Cpu->pc on stack, and jump to a new location
void i8080Emulator::CALL() {
	CALLif(true);
}

//add carry bit and Byte onto A
void i8080Emulator::ACI() {
	uint16_t sum = m_Cpu->a + m_Memory[m_Cpu->pc + 1] + (uint8_t)m_Cpu->ConditionBits.c;
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->ConditionBits.c = sum > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 2;
}

//return if Cy = 0
void i8080Emulator::RNC() {
	RETURN(m_Cpu->ConditionBits.c == false);
}

//jump when carry = 0
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

//call on carry = false
void i8080Emulator::CNC() {
	CALLif((m_Cpu->ConditionBits.c == false));
}

//subtract a byte from A
void i8080Emulator::SUI() {
	uint16_t result = m_Cpu->a - m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (result & 0xFF);
	m_Cpu->ConditionBits.c = result > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 2;
}

//return if CY = 1
void i8080Emulator::RC() {
	RETURN(m_Cpu->ConditionBits.c);
}

//jump if cy = 1
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

//call if cy =1
void i8080Emulator::CC() {
	CALLif(m_Cpu->ConditionBits.c);
}

//sub byte and cy from A
void i8080Emulator::SBI() {
	uint16_t sum = m_Cpu->a - m_Memory[m_Cpu->pc + 1] - (uint8_t)m_Cpu->ConditionBits.c;
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->ConditionBits.c = sum > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 2;
}

//return if P = 0
void i8080Emulator::RPO() {
	RETURN(m_Cpu->ConditionBits.p == 0);
}

//jump if p = 0
void i8080Emulator::JPO() {
	JUMP(m_Cpu->ConditionBits.p == 0);
}

//exchange HL and SP data
//L <-> (SP) | H <-> (SP+1)
void i8080Emulator::XTHL() {
	const uint16_t stackContents = (m_Memory[m_Cpu->sp + 1] << 8) | m_Memory[m_Cpu->sp];
	MemWrite(m_Cpu->sp, m_Cpu->l);
	MemWrite(m_Cpu->sp + 1, m_Cpu->h);
	m_Cpu->SetRegisterPair(RegisterPairs8080::HL, stackContents);
	m_Cpu->pc += 1;
}

//call if p = 0
void i8080Emulator::CPO() {
	CALLif(m_Cpu->ConditionBits.p == 0);
}

//bitwise AND byte with A
void i8080Emulator::ANI() {
	uint16_t result = m_Cpu->a & m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (result & 0xFF);

	m_Cpu->ConditionBits.c = result > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += OPCODES[0xE6].sizeBytes;
}

//return if p = 1
void i8080Emulator::RPE() {
	RETURN(m_Cpu->ConditionBits.p);
}

//set pc to HL
void i8080Emulator::PCHL() {
	m_Cpu->pc = m_Cpu->ReadRegisterPair(RegisterPairs8080::HL);
}

//jump if p = 1
void i8080Emulator::JPE() {
	JUMP(m_Cpu->ConditionBits.p);
}

//exchange HL and DE
void i8080Emulator::XCHG() {
	uint16_t oldDE = m_Cpu->ReadRegisterPair(RegisterPairs8080::DE);
	m_Cpu->SetRegisterPair(RegisterPairs8080::DE, m_Cpu->l, m_Cpu->h);//uint16_t(m_Cpu->h << 8) | m_Cpu->l);
	m_Cpu->SetRegisterPair(RegisterPairs8080::HL, oldDE);
	m_Cpu->pc += OPCODES[0xEB].sizeBytes;
}

//call if p = 1
void i8080Emulator::CPE() {
	CALLif(m_Cpu->ConditionBits.p);
}

//XOR A with a byte
void i8080Emulator::XRI() {
	uint16_t sum = m_Cpu->a ^ m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (sum & 0xFF);
	m_Cpu->ConditionBits.c = sum > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += OPCODES[0xEE].sizeBytes;
}

//return if positive
void i8080Emulator::RP() {
	RETURN(m_Cpu->ConditionBits.s == false);
}

//jump if positive
void i8080Emulator::JP() {
	JUMP(m_Cpu->ConditionBits.s == false);
}

//disable Interrupts
void i8080Emulator::DI() {
	m_Cpu->interruptsEnabled = false;
	m_Cpu->pc += 1;
}

//call if positive
void i8080Emulator::CP() {
	CALLif(m_Cpu->ConditionBits.s == false);
}

//biwise OR A with a byte
void i8080Emulator::ORI() {
	const uint16_t result = m_Cpu->a | m_Memory[m_Cpu->pc + 1];
	m_Cpu->a = (result & 0xFF);
	m_Cpu->ConditionBits.c = result > 0xFF00;
	m_Cpu->UpdateFlags(m_Cpu->a);
	m_Cpu->pc += 2;
}

//return if minus
void i8080Emulator::RM() {
	RETURN(m_Cpu->ConditionBits.s == true);
}

//sets SP to HL
void i8080Emulator::SPHL() {
	m_Cpu->sp = m_Cpu->ReadRegisterPair(RegisterPairs8080::HL);
	m_Cpu->pc += OPCODES[0xF9].sizeBytes;
}

//jump if minus
void i8080Emulator::JM() {
	JUMP(m_Cpu->ConditionBits.s == true);
}

//enable Interrrupts
void i8080Emulator::EI() {
	m_Cpu->interruptsEnabled = true;
	m_Cpu->pc += 1;
}

//call if minus
void i8080Emulator::CM() {
	CALLif(m_Cpu->ConditionBits.s == true);
}

//ComPare Immediate with Accumulator
//See page 29 8080-Programmers-Manual
void i8080Emulator::CPI() {
	const uint8_t result = m_Cpu->a - m_Memory[m_Cpu->pc + 1];
	m_Cpu->ConditionBits.c = m_Cpu->a < m_Memory[m_Cpu->pc + 1];
	m_Cpu->UpdateFlags(result);
	m_Cpu->pc += OPCODES[0xFE].sizeBytes;
}

#pragma endregion OpcodeFunctions

