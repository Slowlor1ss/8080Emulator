#include "i8080Emulator.h"

//Standard includes
#include <thread>
#include <chrono>
#include <fstream>
#include <bitset>
#include <fstream>
#include <cassert>
#include <format>
#include <iomanip>

//Project includes
#include "CPU.h"
#include "Display.h"
#include "Keyboard.h"

using namespace std::chrono;

uint64_t GetDeltaTime(const time_point<high_resolution_clock>* startTime) {

	const auto timeDifference = high_resolution_clock::now() - *startTime;
	return duration_cast<microseconds>(timeDifference).count(); //get microseconds
}

i8080Emulator::i8080Emulator()
	: m_ConsoleProg(false)
	, m_pCpu(new CPU{ this }) //2 MHz
	, m_Memory(nullptr)
	, m_CurrRomSize(0)
	, m_CurrentOpcode(0x00)
	, m_ClocksPerMs(2'000'000)
	, m_pDisplay(new Display("Intel 8080", 224, 256, 2))
	, m_pKeyboard(new Keyboard(m_pCpu))
{
	m_pCpu->halt = true;
}

i8080Emulator::i8080Emulator(const char* path, bool consoleProgram)
	: i8080Emulator()
{
	LoadRom(consoleProgram, path);
}

i8080Emulator::~i8080Emulator() {

	delete m_pCpu;
	m_pCpu = nullptr;

	delete[] m_Memory;
	m_Memory = nullptr;

	delete m_pDisplay;
	m_pDisplay = nullptr;

	delete m_pKeyboard;
	m_pKeyboard = nullptr;
}

bool i8080Emulator::LoadRom(bool consoleProgram, const char* path)
{
	m_pCpu->Reset();
	m_ConsoleProg = consoleProgram;

	std::ifstream file(path, std::ios::in | std::ios::binary);

	if (!file)
	{
		std::cerr << "Couldn't open file" << '\n';
		m_pCpu->halt = true;
		return false;
	}

	//get the file size and read it into a memory buffer    
	file.seekg(0, std::ios::end);
	m_CurrRomSize = file.tellg();
	file.seekg(0, std::ios::beg);

	//delete old memory load new
	delete m_Memory;
	m_Memory = new uint8_t[memory_size];
	std::fill_n(m_Memory, memory_size, 0);
	uint8_t temp[memory_size]{};

	file.read(reinterpret_cast<char*>(temp), m_CurrRomSize);

	//console programs start at 256
	m_ProgramStart = m_ConsoleProg ? 0x100 : 0x0;

	for (int64_t i{ 0 }; i < m_CurrRomSize; ++i) {
		m_Memory[m_ProgramStart + i] = temp[i];
	}

	file.close();


	//initialize CPU
	m_pCpu->pc = m_ProgramStart;
	m_pCpu->sp = stack_start; //TODO: TEST

	m_StartTime = high_resolution_clock::now();
	m_pCpu->halt = false;

	return true;
}

void i8080Emulator::ThrottleCPU(uint64_t currentTime) {

	if (currentTime - m_LastThrottle <= 4000) { // Check every 4 us

		if (m_pCpu->clockCount >= (m_ClocksPerMs << 2)) { //throttle CPU
			const uint64_t usToSleep = 4000 - (currentTime - m_LastThrottle); //sleep for the rest of the 4 us
			std::this_thread::sleep_for(microseconds(usToSleep));
			m_LastThrottle = GetDeltaTime(&m_StartTime);
			m_pCpu->clockCount = 0;
		}
	}
	else { // Host CPU is slower or equal to i8080
		m_LastThrottle = currentTime;
		m_pCpu->clockCount = 0;
	}
}

void i8080Emulator::Update() {

	if (!m_pCpu->halt)
	{
		if (!m_ConsoleProg) {
			const uint64_t currentTime = GetDeltaTime(&m_StartTime);

			ThrottleCPU(currentTime);

			m_pDisplay->Update(currentTime, m_Memory + stack_start, this);

			m_pKeyboard->Update(currentTime);
		}

		CycleCpu();
	}
}

void i8080Emulator::Stop()
{
	m_pCpu->halt = true;
}

void i8080Emulator::CycleCpu() {

	if (m_pCpu->pc >= memory_size) {
		std::cout << "Program counter overflow\n";
		m_pCpu->PrintRegister();
		m_pCpu->halt = true;
	}

	m_CurrentOpcode = m_Memory[m_pCpu->pc];

	(this->*OPCODES[m_CurrentOpcode].opcode)();

	m_pCpu->clockCount += InstructionCycles[m_CurrentOpcode];

	if (m_ConsoleProg) {
		//exit if 0, print if 5
		if(m_pCpu->pc == 0 || m_pCpu->pc == 5) {
			Syscall(m_pCpu->pc);
		}
	}
}

void i8080Emulator::Syscall(uint16_t ID) {
	switch (ID) {
	case 0: 
		m_pCpu->halt = true;
		break;
	case 5:
		if (m_pCpu->c == 2) {
			std::cout << static_cast<char>(m_pCpu->e);
		}
		else if (m_pCpu->c == 9) {
			for (int i = m_pCpu->ReadRegisterPair(RegisterPairs8080::DE); m_Memory[i] != 0x24; i++) {
				std::cout << static_cast<char>(m_Memory[i]);
			}
		}
		fflush(stdout);
		RET();
		break;
	default: ;
	}
}

void i8080Emulator::Interrupt(uint8_t ID)
{
	if (!m_pCpu->interruptsEnabled)
		return;

	m_pCpu->interruptsEnabled = false; //Disable Interrupts

	switch (ID)
	{
	case 0: 
		m_CurrentOpcode = 0xCF; //RST 1
		break;
	case 1: 
		m_CurrentOpcode = 0xD7; //RST 2
		break;
	default:
		assert(!"should never get here!");
		break;
	}

	//RST assumes current operation call was RST but as this is an interrupt thats not the case
	//and thus current operation still has to be called
	//so by decrementing the pc the correct operation will be saved to sp
	m_pCpu->pc -= 1;

	//call correct RST according to opcode set above
	RST();
}

//used for debugging
void i8080Emulator::PrintDisassembledRom() const
{
	if (m_Memory == nullptr)
		return;

	//not using m_Cpu->pc because we just want to print the rom not actually move the pc
	auto pc = m_ProgramStart;

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
	if (address >= memory_size) {
		address = address % rom_size + rom_size;
	}
	if (!m_ConsoleProg && address < m_CurrRomSize) {
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
	m_pCpu->PrintRegister();
	//if you reach this at this point
	//make sure you're not running space invaders as a console program
	//or the other way around (see checkbox next to load rom)
	__debugbreak(); 
	NOP();
}

//opcode implementations
#pragma region OpcodeFunctions

#pragma region GenericOpcodeFunctions
//fetches a PC from the stack
void i8080Emulator::RETURN(bool condition)
{
	if (condition) {
		m_pCpu->pc = uint16_t(m_Memory[m_pCpu->sp + 1] << 8) | m_Memory[m_pCpu->sp];
		m_pCpu->sp += 2;
	}
	else {
		m_pCpu->pc += 1;
	}
}

//the contents of the program counter
//are pushed onto the stack, providing a return address for
//later use by a RETURN instruction
void i8080Emulator::RESTART(uint16_t callAddress)
{
	MemWrite((m_pCpu->sp - 1), ((m_pCpu->pc + 1) >> 8));
	MemWrite((m_pCpu->sp - 2), uint8_t(m_pCpu->pc + 1));
	m_pCpu->sp -= 2;
	m_pCpu->pc = callAddress;
}

//pushes PC, then sets PC
void i8080Emulator::CALLif(bool condition)
{
	if (condition) {
		MemWrite((m_pCpu->sp - 1), ((m_pCpu->pc + 3) & 0xFF00) >> 8);
		MemWrite((m_pCpu->sp - 2), ((m_pCpu->pc + 3) & 0x00FF));
		m_pCpu->sp -= 2;

		m_pCpu->pc = uint16_t(m_Memory[m_pCpu->pc + 2] << 8) | m_Memory[m_pCpu->pc + 1];
	}
	else {
		m_pCpu->pc += 3;
	}
}

void i8080Emulator::JUMP(bool condition)
{
	if (condition) {
		const uint16_t address = uint16_t(m_Memory[m_pCpu->pc + 2] << 8) | m_Memory[m_pCpu->pc + 1];
		m_pCpu->pc = address;
	}
	else {
		m_pCpu->pc += 3;
	}
}

void i8080Emulator::POP()
{
	const RegisterPairs8080 pair = m_pCpu->GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint16_t value = uint16_t(m_Memory[m_pCpu->sp + 1] << 8) | m_Memory[m_pCpu->sp];

	if (pair == RegisterPairs8080::SP){ //special case in the pop operation sp is replaced by a and has special calculations see page 23 8080-Programmers-Manual
		m_pCpu->a = (value >> 8);
		m_pCpu->SetFlags(value & 0x00FF);
	}
	else
		m_pCpu->SetRegisterPair(pair, value);

	m_pCpu->sp += 2;
	m_pCpu->pc += 1;
}

//decrement register pair
void i8080Emulator::DCX()
{
	const RegisterPairs8080 pair = m_pCpu->GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint16_t result = m_pCpu->ReadRegisterPair(pair) - 1;
	m_pCpu->SetRegisterPair(pair, result);
	m_pCpu->pc += 1;
}

//add register pair to HL
void i8080Emulator::DAD()
{
	const RegisterPairs8080 pair = m_pCpu->GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint32_t result = m_pCpu->ReadRegisterPair(RegisterPairs8080::HL) + m_pCpu->ReadRegisterPair(pair);
	m_pCpu->SetRegisterPair(RegisterPairs8080::HL, uint16_t(result));
	m_pCpu->ConditionBits.c = (result > 0xFFFF);
	m_pCpu->pc += 1;
}

//decrements register
void i8080Emulator::DCR()
{
	const Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode, 3);
	const uint8_t result = m_pCpu->ReadRegister(reg) - 1;
	m_pCpu->SetRegister(reg, result);
	m_pCpu->UpdateFlags(result);
	m_pCpu->pc += 1;
}

//sets a register to the byte after PC
void i8080Emulator::MVI()
{
	const Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode, 3);
	m_pCpu->SetRegister(reg, m_Memory[m_pCpu->pc + 1]);
	m_pCpu->pc += 2;
}

//see page 3 8080-Programmers-Manual [STACK PUSH OPERATION]
void i8080Emulator::PUSH()
{
	const RegisterPairs8080 pair = m_pCpu->GetRegisterPairFromOpcode(m_CurrentOpcode);

	uint16_t value{};
	if (pair == RegisterPairs8080::SP) //special case in the push operation sp is replaced by a and has special calculations see page 22-23 8080-Programmers-Manual
		value = uint16_t(m_pCpu->a << 8) | m_pCpu->ReadFlags();
	else
		value = m_pCpu->ReadRegisterPair(pair);
		

	MemWrite((m_pCpu->sp - 1), (value & 0xFF00) >> 8);
	MemWrite((m_pCpu->sp - 2), (value & 0x00FF));
	m_pCpu->sp -= 2;
	m_pCpu->pc += 1;
}

//loads immediate into register pair
void i8080Emulator::LXI()
{
	const RegisterPairs8080 pair = m_pCpu->GetRegisterPairFromOpcode(m_CurrentOpcode);
	m_pCpu->SetRegisterPair(pair,m_Memory[m_pCpu->pc + 1], m_Memory[m_pCpu->pc + 2]);

	m_pCpu->pc += 3;
}

//increments register pair
void i8080Emulator::INX()
{
	const RegisterPairs8080 pair = m_pCpu->GetRegisterPairFromOpcode(m_CurrentOpcode);
	const uint16_t result = m_pCpu->ReadRegisterPair(pair) + 1;
	m_pCpu->SetRegisterPair(pair, result);
	m_pCpu->pc += 1;
}

//increments given register by one
void i8080Emulator::INR()
{
	const Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode, 3);
	const uint8_t result = m_pCpu->ReadRegister(reg) + 1;
	m_pCpu->SetRegister(reg, result);
	m_pCpu->UpdateFlags(result);
	m_pCpu->pc += 1;
}
#pragma endregion GenericOpcodeFunctions

////////////////////////////////////////////////////
////////////////////////////////////////////////////

void i8080Emulator::NOP() {
	m_pCpu->pc += OPCODES[0x00].sizeBytes;
}

//write A to mem at address BC 
void i8080Emulator::STAXB() {
	MemWrite(m_pCpu->ReadRegisterPair(RegisterPairs8080::BC), m_pCpu->a);
	m_pCpu->pc += OPCODES[0x02].sizeBytes;
}

//bit shift A left, bit 0 & Cy = prev bit 7
void i8080Emulator::RLC() {
	uint8_t oldBit7 = (m_pCpu->a & 128) >> 7;
	m_pCpu->a <<= 1;
	m_pCpu->a = m_pCpu->a | oldBit7;
	m_pCpu->ConditionBits.c = (1 == oldBit7);
	m_pCpu->pc += 1;
}

//set register A to the contents or memory pointed by BC
void i8080Emulator::LDAXB() {
	m_pCpu->a = m_Memory[m_pCpu->ReadRegisterPair(RegisterPairs8080::BC)];
	m_pCpu->pc += 1;
}

//rotates A right 1, bit 7 & CY = prev bit 0
void i8080Emulator::RRC() {
	uint8_t oldBit0 = (m_pCpu->a & 1);
	m_pCpu->a >>= 1;
	m_pCpu->a = m_pCpu->a | (oldBit0 << 7);
	m_pCpu->ConditionBits.c = (1 == oldBit0);
	m_pCpu->pc += OPCODES[0x0F].sizeBytes;
}

//stores A into the address pointed to by the D reg pair
void i8080Emulator::STAXD() {
	MemWrite(m_pCpu->ReadRegisterPair(RegisterPairs8080::DE), m_pCpu->a);
	m_pCpu->pc += OPCODES[0x12].sizeBytes;
}

//the contents of the accumulator are rotated one bit position to the left
void i8080Emulator::RAL() {
	uint8_t oldA = m_pCpu->a;
	m_pCpu->a <<= 1;
	m_pCpu->a = m_pCpu->a | uint8_t(m_pCpu->ConditionBits.c);
	m_pCpu->ConditionBits.c = (oldA >= 128);
	m_pCpu->pc += OPCODES[0x17].sizeBytes;
}

//store the value at the memory referenced by DE in A
void i8080Emulator::LDAXD() {
	m_pCpu->a = m_Memory[m_pCpu->ReadRegisterPair(RegisterPairs8080::DE)];
	m_pCpu->pc += OPCODES[0x1A].sizeBytes;
}

//rotate A right one, bit 7 = prev CY, CY = prevbit0
void i8080Emulator::RAR() {
	uint8_t oldA = m_pCpu->a;
	m_pCpu->a >>= 1;
	m_pCpu->a = m_pCpu->a | (m_pCpu->ConditionBits.c ? 0x80 : 0x00);
	m_pCpu->ConditionBits.c = (0x01 == (oldA & 0x01));
	m_pCpu->pc += 1;
}

//stores HL into address listed after m_Cpu->pc
//adr <- L , adr+1 <- H
void i8080Emulator::SHLD() {
	uint16_t address = (m_Memory[m_pCpu->pc + 2] << 8) | m_Memory[m_pCpu->pc + 1];
	MemWrite(address, m_pCpu->l);
	MemWrite(address + 1, m_pCpu->h);
	m_pCpu->pc += 3;
}

//The eight-bit hexadecimal number in the
//accumulator is adjusted to form two four-bit binary coded decimals
void i8080Emulator::DAA() {

	if (((m_pCpu->a & 0x0F) > 0x09) || m_pCpu->ConditionBits.ac) {
		m_pCpu->a += 0x06;
	}

	if (((m_pCpu->a & 0xF0) > 0x90) || m_pCpu->ConditionBits.c) {
		m_pCpu->ConditionBits.c = true;
		m_pCpu->a += 0x60;
	}

	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += 1;
}

//read memory from 2bytes after m_Cpu->pc into HL
// L <- adr, H <- adr+1
void i8080Emulator::LHLD() {
	uint16_t address = ((m_Memory[m_pCpu->pc + 2] << 8) | m_Memory[m_pCpu->pc + 1]);
	m_pCpu->l = m_Memory[address];
	m_pCpu->h = m_Memory[address + 1];
	m_pCpu->pc += 3;
}

//invert A
void i8080Emulator::CMA() {
	m_pCpu->a = ~m_pCpu->a;
	m_pCpu->pc += 1;
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
	const uint16_t address = uint16_t(m_Memory[m_pCpu->pc + 2] << 8) | m_Memory[m_pCpu->pc + 1];
	MemWrite(address, m_pCpu->a);

	m_pCpu->pc += OPCODES[0x32].sizeBytes;
}

//set carry flag to 1
void i8080Emulator::STC() {
	m_pCpu->ConditionBits.c = true;
	m_pCpu->pc += 1;
}

//set reg A to the value pointed by bytes after m_Cpu->pc
void i8080Emulator::LDA() {
	m_pCpu->a = m_Memory[((m_Memory[m_pCpu->pc + 2] << 8) | m_Memory[m_pCpu->pc + 1])];
	m_pCpu->pc += OPCODES[0x3A].sizeBytes;
}

//invert carry flag
void i8080Emulator::CMC() {
	m_pCpu->ConditionBits.c = !m_pCpu->ConditionBits.c;
	m_pCpu->pc += 1;
}

void i8080Emulator::MOV() {

	const auto src = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode);
	const auto dest = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode, 3);

	m_pCpu->SetRegister(dest, m_pCpu->ReadRegister(src));
	m_pCpu->pc += 1;
}

//halt
void i8080Emulator::HLT() {
	m_pCpu->halt = true;
	m_pCpu->pc += 1;
}

//add a register/mem value onto A, and update all flags
void i8080Emulator::ADD() {
	Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode);

	uint16_t sum = m_pCpu->a + m_pCpu->ReadRegister(reg);
	m_pCpu->ConditionBits.ac = (sum ^ m_pCpu->a ^ m_pCpu->ReadRegister(reg)) & (1 << 4);
	m_pCpu->ConditionBits.c = sum > 0xFF;
	m_pCpu->a = (sum & 0xFF);
	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += 1;
}

//add a register/mem value + carry bit onto A, update registers
void i8080Emulator::ADC() {
	Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode);

	uint16_t sum = m_pCpu->a + m_pCpu->ReadRegister(reg) + (uint8_t)m_pCpu->ConditionBits.c;
	m_pCpu->ConditionBits.ac = (sum ^ m_pCpu->a ^ m_pCpu->ReadRegister(reg)) & (1 << 4);
	m_pCpu->ConditionBits.c = sum > 0xFF;
	m_pCpu->a = (sum & 0xFF);
	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += 1;
}

//sub a register/mem value from A, and update all flags
void i8080Emulator::SUB() {
	const Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode);

	uint16_t sum = m_pCpu->a + m_pCpu->ReadRegister(reg) + 1;//(1-(uint8_t)m_Cpu->ConditionBits.c);
	m_pCpu->ConditionBits.ac = (sum ^ m_pCpu->a ^ m_pCpu->ReadRegister(reg)) & (1 << 4);
	m_pCpu->ConditionBits.c = sum > 0xFF;

	m_pCpu->a -= m_pCpu->ReadRegister(reg);
	m_pCpu->pc += 1;
}

//SBB Subtract Register or Memory From Accumulator With Borrow
void i8080Emulator::SBB() {
	const Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode);

	uint16_t result = m_pCpu->a - (m_pCpu->ReadRegister(reg) + m_pCpu->ConditionBits.c);

	uint16_t sum = m_pCpu->a + m_pCpu->ReadRegister(reg) + (1 - (uint8_t)m_pCpu->ConditionBits.c);
	m_pCpu->ConditionBits.ac = (sum ^ m_pCpu->a ^ m_pCpu->ReadRegister(reg)) & (1 << 4);
	m_pCpu->ConditionBits.c = sum > 0xFF;

	m_pCpu->a = (result & 0xFF);

	m_pCpu->pc += 1;
}

//bitwise AND a register/mem value with A
void i8080Emulator::ANA() {
	const Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_pCpu->a & m_pCpu->ReadRegister(reg);
	m_pCpu->a = (result & 0xFF);

	m_pCpu->ConditionBits.c = false;
	m_pCpu->UpdateFlags(m_pCpu->a);

	m_pCpu->pc += 1;
}

//bitwise XOR a register/mem value with A, update registers
void i8080Emulator::XRA() {
	const Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_pCpu->a ^ m_pCpu->ReadRegister(reg);
	m_pCpu->a = (result & 0xFF);

	m_pCpu->ConditionBits.c = false;
	m_pCpu->UpdateFlags(m_pCpu->a);

	m_pCpu->pc += 1;
}

//bitwise OR a register/mem value with A, update registers
void i8080Emulator::ORA() {
	const Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_pCpu->a | m_pCpu->ReadRegister(reg);
	m_pCpu->a = (result & 0xFF);

	m_pCpu->ConditionBits.c = false;
	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += 1;
}

//compare a register/mem value with A, update registers
void i8080Emulator::CMP() {
	const Registers8080 reg = m_pCpu->GetRegisterFromOpcode(m_CurrentOpcode);

	const uint16_t result = m_pCpu->a - m_pCpu->ReadRegister(reg);
	m_pCpu->ConditionBits.ac = (result ^ m_pCpu->a ^ m_pCpu->ReadRegister(reg)) & (1 << 4);
	m_pCpu->ConditionBits.c = (result & 0xFF00) != 0;
	m_pCpu->UpdateFlags(uint8_t(result));

	m_pCpu->pc += 1;
}

//return on nonzero
void i8080Emulator::RNZ() {
	RETURN((m_pCpu->ConditionBits.z == false));
}

//jump to address from after m_Cpu->pc if non zero
void i8080Emulator::JNZ() {
	JUMP(m_pCpu->ConditionBits.z == false);
}

//normal jump
void i8080Emulator::JMP() {
	JUMP(true);
}

//call if not zero
void i8080Emulator::CNZ() {
	CALLif(m_pCpu->ConditionBits.z == false);
}

//adds a byte onto A, fetched after m_Cpu->pc
void i8080Emulator::ADI() {
	uint16_t sum = m_pCpu->a + m_Memory[m_pCpu->pc + 1];
	m_pCpu->a = (sum & 0xFF);
	m_pCpu->ConditionBits.c = sum > 0xFF;
	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += OPCODES[0xC6].sizeBytes;
}

void i8080Emulator::RST() {
	const uint8_t address = (m_CurrentOpcode & 0b0011'1000);
	RESTART(address);
}

//return if Z
void i8080Emulator::RZ() {
	RETURN(m_pCpu->ConditionBits.z);
}

//return
void i8080Emulator::RET() {
	RETURN(true);
}

//jump if zero
void i8080Emulator::JZ() {
	JUMP(m_pCpu->ConditionBits.z);
}

//call if Z flag set
void i8080Emulator::CZ() {
	CALLif(m_pCpu->ConditionBits.z);
}

//store m_Cpu->pc on stack, and jump to a new location
void i8080Emulator::CALL() {
	CALLif(true);
}

//add carry bit and Byte onto A
void i8080Emulator::ACI() {
	uint16_t sum = m_pCpu->a + m_Memory[m_pCpu->pc + 1] + (uint8_t)m_pCpu->ConditionBits.c;
	m_pCpu->a = (sum & 0xFF);
	m_pCpu->ConditionBits.c = sum > 0xFF;
	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += 2;
}

//return if Cy = 0
void i8080Emulator::RNC() {
	RETURN(m_pCpu->ConditionBits.c == false);
}

//jump when carry = 0
void i8080Emulator::JNC() {
	JUMP(m_pCpu->ConditionBits.c == false);
}

//https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#dedicated-shift-hardware
void i8080Emulator::OUT() {
	uint8_t port = m_Memory[m_pCpu->pc + 1];

	if (port == 2){
		m_pCpu->shiftOffset = m_pCpu->a & 7;
	}
	else if (port == 4){
		m_pCpu->regShift >>= 8;
		m_pCpu->regShift |= (m_pCpu->a << 8);
	}
	else{
		m_pCpu->outPort[port] = m_pCpu->a;
	}

	m_pCpu->pc += OPCODES[0xD3].sizeBytes;
}

//call on carry = false
void i8080Emulator::CNC() {
	CALLif((m_pCpu->ConditionBits.c == false));
}

//subtract a byte from A
void i8080Emulator::SUI() {
	uint16_t result = m_pCpu->a - m_Memory[m_pCpu->pc + 1];
	m_pCpu->a = (result & 0xFF);
	m_pCpu->ConditionBits.c = result > 0xFF00;
	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += 2;
}

//return if CY = 1
void i8080Emulator::RC() {
	RETURN(m_pCpu->ConditionBits.c);
}

//jump if cy = 1
void i8080Emulator::JC() {
	JUMP(m_pCpu->ConditionBits.c);
}

//https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#inputs
void i8080Emulator::IN() {
	uint8_t port = m_Memory[m_pCpu->pc + 1];

	if (port == 3){
		m_pCpu->a = uint8_t(m_pCpu->regShift >> (8 - m_pCpu->shiftOffset));
	}
	else{
		m_pCpu->a = m_pCpu->inPort[port];
	}

	m_pCpu->pc += OPCODES[0xDB].sizeBytes;
}

//call if cy =1
void i8080Emulator::CC() {
	CALLif(m_pCpu->ConditionBits.c);
}

//sub byte and cy from A
void i8080Emulator::SBI() {
	uint16_t sum = m_pCpu->a - m_Memory[m_pCpu->pc + 1] - (uint8_t)m_pCpu->ConditionBits.c;
	m_pCpu->a = (sum & 0xFF);
	m_pCpu->ConditionBits.c = sum > 0xFF00;
	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += 2;
}

//return if P = 0
void i8080Emulator::RPO() {
	RETURN(m_pCpu->ConditionBits.p == 0);
}

//jump if p = 0
void i8080Emulator::JPO() {
	JUMP(m_pCpu->ConditionBits.p == 0);
}

//exchange HL and SP data
//L <-> (SP) | H <-> (SP+1)
void i8080Emulator::XTHL() {
	const uint16_t stackContents = (m_Memory[m_pCpu->sp + 1] << 8) | m_Memory[m_pCpu->sp];
	MemWrite(m_pCpu->sp, m_pCpu->l);
	MemWrite(m_pCpu->sp + 1, m_pCpu->h);
	m_pCpu->SetRegisterPair(RegisterPairs8080::HL, stackContents);
	m_pCpu->pc += 1;
}

//call if p = 0
void i8080Emulator::CPO() {
	CALLif(m_pCpu->ConditionBits.p == 0);
}

//bitwise AND byte with A
void i8080Emulator::ANI() {
	uint16_t result = m_pCpu->a & m_Memory[m_pCpu->pc + 1];
	m_pCpu->a = (result & 0xFF);

	m_pCpu->ConditionBits.c = result > 0xFF00;
	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += OPCODES[0xE6].sizeBytes;
}

//return if p = 1
void i8080Emulator::RPE() {
	RETURN(m_pCpu->ConditionBits.p);
}

//set pc to HL
void i8080Emulator::PCHL() {
	m_pCpu->pc = m_pCpu->ReadRegisterPair(RegisterPairs8080::HL);
}

//jump if p = 1
void i8080Emulator::JPE() {
	JUMP(m_pCpu->ConditionBits.p);
}

//exchange HL and DE
void i8080Emulator::XCHG() {
	uint16_t oldDE = m_pCpu->ReadRegisterPair(RegisterPairs8080::DE);
	m_pCpu->SetRegisterPair(RegisterPairs8080::DE, m_pCpu->l, m_pCpu->h);//uint16_t(m_Cpu->h << 8) | m_Cpu->l);
	m_pCpu->SetRegisterPair(RegisterPairs8080::HL, oldDE);
	m_pCpu->pc += OPCODES[0xEB].sizeBytes;
}

//call if p = 1
void i8080Emulator::CPE() {
	CALLif(m_pCpu->ConditionBits.p);
}

//XOR A with a byte
void i8080Emulator::XRI() {
	uint16_t sum = m_pCpu->a ^ m_Memory[m_pCpu->pc + 1];
	m_pCpu->a = (sum & 0xFF);
	m_pCpu->ConditionBits.c = sum > 0xFF00;
	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += OPCODES[0xEE].sizeBytes;
}

//return if positive
void i8080Emulator::RP() {
	RETURN(m_pCpu->ConditionBits.s == false);
}

//jump if positive
void i8080Emulator::JP() {
	JUMP(m_pCpu->ConditionBits.s == false);
}

//disable Interrupts
void i8080Emulator::DI() {
	m_pCpu->interruptsEnabled = false;
	m_pCpu->pc += 1;
}

//call if positive
void i8080Emulator::CP() {
	CALLif(m_pCpu->ConditionBits.s == false);
}

//biwise OR A with a byte
void i8080Emulator::ORI() {
	const uint16_t result = m_pCpu->a | m_Memory[m_pCpu->pc + 1];
	m_pCpu->a = (result & 0xFF);
	m_pCpu->ConditionBits.c = result > 0xFF00;
	m_pCpu->UpdateFlags(m_pCpu->a);
	m_pCpu->pc += 2;
}

//return if minus
void i8080Emulator::RM() {
	RETURN(m_pCpu->ConditionBits.s == true);
}

//sets SP to HL
void i8080Emulator::SPHL() {
	m_pCpu->sp = m_pCpu->ReadRegisterPair(RegisterPairs8080::HL);
	m_pCpu->pc += OPCODES[0xF9].sizeBytes;
}

//jump if minus
void i8080Emulator::JM() {
	JUMP(m_pCpu->ConditionBits.s == true);
}

//enable Interrrupts
void i8080Emulator::EI() {
	m_pCpu->interruptsEnabled = true;
	m_pCpu->pc += 1;
}

//call if minus
void i8080Emulator::CM() {
	CALLif(m_pCpu->ConditionBits.s == true);
}

//ComPare Immediate with Accumulator
//See page 29 8080-Programmers-Manual
void i8080Emulator::CPI() {
	const uint8_t result = m_pCpu->a - m_Memory[m_pCpu->pc + 1];
	m_pCpu->ConditionBits.c = m_pCpu->a < m_Memory[m_pCpu->pc + 1];
	m_pCpu->UpdateFlags(result);
	m_pCpu->pc += OPCODES[0xFE].sizeBytes;
}

#pragma endregion OpcodeFunctions

