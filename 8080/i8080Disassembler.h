#pragma once
#include <iostream>

//TODO: maybe turn this in to a cpu class
//holds the state of the CPU
struct State8080
{
	//Dedicated Shift Hardware //Source: https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#dedicated-shift-hardware
	//The 8080 instruction set does not include opcodes for shifting.
	//An 8-bit pixel image must be shifted into a 16-bit word for the desired bit-position on the screen.
	//Space Invaders adds a hardware shift register to help with the math.
	uint16_t regShift;
	uint8_t shiftOffset; //(3 bits)

	//I/O
	uint8_t inPort[4];
	uint8_t outPort[7];

	// Status
	uint8_t interruptsEnabled;

	//primary accumulator
	uint8_t a;
	
	//three 16-bit register pairs (BC, DE, and HL)
	//that can function as six individual 8-bit registers
	//the reversed order of the two registers in each register pair is intentional due to the little-endianness
	//technically this is not very important as I'll only be accessing one byte at a time
	uint8_t c;
	uint8_t b;

	uint8_t e;
	uint8_t d;

	uint8_t l; //holds the least significant 8 bits
	uint8_t h; //holds most significant 8 bits
	//

	//stack pointer
	uint16_t sp;
	//program counter
	uint16_t pc;

	//file://Resources/8080-Programmers-Manual.pdf ConditionBits p.11 in pdf or p.5 in the book
	struct
	{
		bool c : 1;  //carry, set if the last addition operation resulted in a carry 
							// or last sub required borrow
		bool : 1;
		bool p : 1;  //parity bit, set if the number of true bits in the result is even
		bool : 1;
		bool ac : 1; //auxiliary carry bit for binary coded decimal arithmetic 
		bool : 1;
		bool z : 1;  //zero bit, set if the result is zero 
		bool s : 1;  //sign bit, set if the result is negative
	} ConditionBits;
};

//based 8080-Programmers-Manual page 25
enum class RegisterPairs8080
{
	BC = 0b00,
	DE = 0b01,
	HL = 0b10,
	SP = 0b11 //technically not part of the three register pairs but is used as one in operations like LXI SP
};

//based 8080-Programmers-Manual page 16-17
enum class Registers8080
{
	B = 0b000,
	C = 0b001,
	D = 0b010,
	E = 0b011,
	H = 0b100,
	L = 0b101,
	MEM = 0b110, //memory reference M, the addressed location is specified by the HL reg
	A = 0b111,
};

class i8080Emulator
{
	void defaultOpcode();
	struct Opcode
	{
		void (i8080Emulator::* opcode)() { &i8080Emulator::defaultOpcode};
		const char* mnemonic;
		unsigned char sizeBytes;
	};

public:
	i8080Emulator(const char* path);
	~i8080Emulator();

	i8080Emulator(const i8080Emulator& other) = delete;
	i8080Emulator(i8080Emulator&& other) noexcept = delete;
	i8080Emulator& operator=(const i8080Emulator& other) = delete;
	i8080Emulator& operator=(i8080Emulator&& other) noexcept = delete;

	bool CycleCpu();

	//Debug
	void PrintRegister() const;
	void PrintDisassembledRom() const;

private:

	State8080* m_Cpu;
	uint8_t* m_Memory;
	int64_t m_CurrRomSize;

	uint8_t m_CurrentOpcode;

	//http://www.computerarcheology.com/Arcade/SpaceInvaders/RAMUse.html
	static constexpr int MEMORY_SIZE = 0x10000;//0x4000;
	static constexpr int ROM_SIZE = 0x2000;
	static constexpr int STACK_START = 0x2400;
	static constexpr int PROGRAM_START = 0x0000;

	void UpdateFlags(const uint8_t& value) const;

	void MemWrite(uint16_t address, uint8_t data);

	uint8_t ReadFlags() const;
	void SetFlags(uint8_t);

	uint16_t ReadRegisterPair(RegisterPairs8080 pair) const;
	void SetRegisterPair(RegisterPairs8080 pair, uint16_t value);
	void SetRegisterPair(RegisterPairs8080 pair, uint8_t LSByte, uint8_t MSByte);

	uint8_t ReadRegister(Registers8080 reg) const;
	void SetRegister(Registers8080 pair, uint8_t value);

	//uint8_t* DecodeOpcodeRegister(uint8_t opcode3bit);

	RegisterPairs8080 GetRegisterPairFromOpcode(uint8_t opcode) const;
	Registers8080 GetRegisterFromOpcode(uint8_t opcode, uint8_t shift = 0) const;

#pragma region OpcodeFunctions

#pragma region GenericOpcodeFunctions
	//Generic functions
	void RETURN(bool);
	void RESET(uint16_t);
	void CALLif(bool);
	void JUMP(bool);
	void POP();
	void DCX();
	void DAD();
	void DCR();
	void MVI();
	void PUSH();
	void LXI();
	void INX();
	void INR();
#pragma endregion GenericOpcodeFunctions

	void NOP();
	//void LXIB();
	void STAXB();
	//void INXB();
	//void INRB();
	//void DCRB();
	//void MVIB();
	void RLC();
	/*NOP*/
	//void DADB();
	void LDAXB();
	//void DCXB();
	//void INRC();
	//void DCRC();
	//void MVIC();
	void RRC();
	/*NOP*/
	//void LXID();
	void STAXD();
	//void INXD();
	//void INRD();
	//void DCRD();
	//void MVID();
	void RAL();
	/*NOP*/
	//void DADD();
	void LDAXD();
	//void DCXD();
	//void INRE();
	//void DCRE();
	//void MVIE();
	void RAR();
	/*NOP*/
	//void LXIH();
	void SHLD();
	//void INXH();
	//void INRH();
	//void DCRH();
	//void MVIH();
	void DAA();
	/*NOP*/
	//void DADH();
	void LHLD();
	//void DCXH();
	//void INRL();
	//void DCRL();
	//void MVIL();
	void CMA();
	/*NOP*/
	//void LXISP();
	void STA();
	//void INXSP();
	//void INRM();
	//void DCRM();
	//void MVIM();
	void STC();
	/*NOP*/
	//void DADSP();
	void LDA();
	//void DCXSP();
	//void INRA();
	//void DCRA();
	//void MVIA();
	void CMC();
	// (0x40 - 75, 0x77 - 0x7F)
	void MOV();
	// 0x76
	void HLT();
	// (0x80 - 0x87)
	void ADD();
	// (0x88 - 0x8f)
	void ADC();
	// (0x90 - 0x97)
	void SUB();
	// (0x98 - 0x9f)
	void SBB();
	// (0xA0 - 0xA7)
	void ANA();
	// (0xA8 - 0xAF)
	void XRA();
	// (0xB0 - 0xB7)
	void ORA();
	// (0xB8 - 0xBF)
	void CMP();
	void RNZ();
	//void POPB();
	void JNZ();
	void JMP();
	void CNZ();
	//void PUSHB();
	void ADI();
	void RST0();
	void RZ();
	void RET();
	void JZ();
	/*NOP*/
	void CZ();
	void CALL();
	void ACI();
	void RST1();
	void RNC();
	//void POPD();
	void JNC();
	void OUT();
	void CNC();
	//void PUSHD();
	void SUI();
	void RST2();
	void RC();
	/*NOP*/
	void JC();
	void IN();
	void CC();
	/*NOP*/
	void SBI();
	void RST3();
	void RPO();
	//void POPH();
	void JPO();
	void XTHL();
	void CPO();
	//void PUSHH();
	void ANI();
	void RST4();
	void RPE();
	void PCHL();
	void JPE();
	void XCHG();
	void CPE();
	/*NOP*/
	void XRI();
	void RST5();
	void RP();
	//void POPPSW();
	void JP();
	void DI();
	void CP();
	//void PUSHPSW();
	void ORI();
	void RST6();
	void RM();
	void SPHL();
	void JM();
	void EI();
	void CM();
	/*NOP*/
	void CPI();
	void RST7();

#pragma endregion OpcodeFunctions

	//wrote the opcode functions myself
	//copied name and size of opcodes from https://github.com/mlima/8080/blob/master/disassemble.cpp
	static constexpr Opcode OPCODES[256]
	{
	{ &i8080Emulator::NOP,"NOP", 1 },
	{ &i8080Emulator::LXI,"LXI B", 3 },
	{ &i8080Emulator::STAXB,"STAX B", 1 },
	{ &i8080Emulator::INX,"INX B", 1 },
	{ &i8080Emulator::INR,"INR B", 1 },
	{ &i8080Emulator::DCR,"DCR B", 1 },
	{ &i8080Emulator::MVI,"MVI B", 2 },
	{ &i8080Emulator::defaultOpcode,"RLC", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::DAD,"DAD B", 1 },
	{ &i8080Emulator::LDAXB,"LDAX B", 1 },
	{ &i8080Emulator::DCX,"DCX B", 1 },
	{ &i8080Emulator::INR,"INR C", 1 },
	{ &i8080Emulator::DCR,"DCR C", 1 },
	{ &i8080Emulator::MVI,"MVI C", 2 },
	{ &i8080Emulator::RRC,"RRC", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::LXI,"LXI D", 3 },
	{ &i8080Emulator::STAXD,"STAX D", 1 },
	{ &i8080Emulator::INX,"INX D", 1 },
	{ &i8080Emulator::INR,"INR D", 1 },
	{ &i8080Emulator::DCR,"DCR D", 1 },
	{ &i8080Emulator::MVI,"MVI D", 2 },
	{ &i8080Emulator::defaultOpcode,"RAL", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::DAD,"DAD D", 1 },
	{ &i8080Emulator::LDAXD,"LDAX D", 1 },
	{ &i8080Emulator::DCX,"DCX D", 1 },
	{ &i8080Emulator::INR,"INR E", 1 },
	{ &i8080Emulator::DCR,"DCR E", 1 },
	{ &i8080Emulator::MVI,"MVI E", 2 },
	{ &i8080Emulator::defaultOpcode,"RAR", 1 },
	{ &i8080Emulator::defaultOpcode,"RIM", 1 },
	{ &i8080Emulator::LXI,"LXI H", 3 },
	{ &i8080Emulator::defaultOpcode,"SHLD", 3 },
	{ &i8080Emulator::INX,"INX H", 1 },
	{ &i8080Emulator::INR,"INR H", 1 },
	{ &i8080Emulator::DCR,"DCR H", 1 },
	{ &i8080Emulator::MVI,"MVI H", 2 },
	{ &i8080Emulator::defaultOpcode,"DAA", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::DAD,"DAD H", 1 },
	{ &i8080Emulator::defaultOpcode,"LHLD", 3 },
	{ &i8080Emulator::DCX,"DCX H", 1 },
	{ &i8080Emulator::INR,"INR L", 1 },
	{ &i8080Emulator::DCR,"DCR L", 1 },
	{ &i8080Emulator::MVI,"MVI L", 2 },
	{ &i8080Emulator::defaultOpcode,"CMA", 1 },
	{ &i8080Emulator::defaultOpcode,"SIM", 1 },
	{ &i8080Emulator::LXI,"LXI SP", 3 },
	{ &i8080Emulator::STA,"STA", 3 },
	{ &i8080Emulator::INX,"INX SP", 1 },
	{ &i8080Emulator::INR,"INR M", 1 },
	{ &i8080Emulator::DCR,"DCR M", 1 },
	{ &i8080Emulator::MVI,"MVI M", 2 },
	{ &i8080Emulator::defaultOpcode,"STC", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::DAD,"DAD SP", 1 },
	{ &i8080Emulator::LDA,"LDA", 3 },
	{ &i8080Emulator::DCX,"DCX SP", 1 },
	{ &i8080Emulator::INR,"INR A", 1 },
	{ &i8080Emulator::DCR,"DCR A", 1 },
	{ &i8080Emulator::MVI,"MVI A", 2 },
	{ &i8080Emulator::defaultOpcode,"CMC", 1 },
	{ &i8080Emulator::MOV, "MOV B,B", 1 },
	{ &i8080Emulator::MOV, "MOV B,C", 1 },
	{ &i8080Emulator::MOV, "MOV B,D", 1 },
	{ &i8080Emulator::MOV, "MOV B,E", 1 },
	{ &i8080Emulator::MOV, "MOV B,H", 1 },
	{ &i8080Emulator::MOV, "MOV B,L", 1 },
	{ &i8080Emulator::MOV, "MOV B,M", 1 },
	{ &i8080Emulator::MOV, "MOV B,A", 1 },
	{ &i8080Emulator::MOV, "MOV C,B", 1 },
	{ &i8080Emulator::MOV, "MOV C,C", 1 },
	{ &i8080Emulator::MOV, "MOV C,D", 1 },
	{ &i8080Emulator::MOV, "MOV C,E", 1 },
	{ &i8080Emulator::MOV, "MOV C,H", 1 },
	{ &i8080Emulator::MOV, "MOV C,L", 1 },
	{ &i8080Emulator::MOV, "MOV C,M", 1 },
	{ &i8080Emulator::MOV, "MOV C,A", 1 },
	{ &i8080Emulator::MOV, "MOV D,B", 1 },
	{ &i8080Emulator::MOV, "MOV D,C", 1 },
	{ &i8080Emulator::MOV, "MOV D,D", 1 },
	{ &i8080Emulator::MOV, "MOV D,E", 1 },
	{ &i8080Emulator::MOV, "MOV D,H", 1 },
	{ &i8080Emulator::MOV, "MOV D,L", 1 },
	{ &i8080Emulator::MOV, "MOV D,M", 1 },
	{ &i8080Emulator::MOV, "MOV D,A", 1 },
	{ &i8080Emulator::MOV, "MOV E,B", 1 },
	{ &i8080Emulator::MOV, "MOV E,C", 1 },
	{ &i8080Emulator::MOV, "MOV E,D", 1 },
	{ &i8080Emulator::MOV, "MOV E,E", 1 },
	{ &i8080Emulator::MOV, "MOV E,H", 1 },
	{ &i8080Emulator::MOV, "MOV E,L", 1 },
	{ &i8080Emulator::MOV, "MOV E,M", 1 },
	{ &i8080Emulator::MOV, "MOV E,A", 1 },
	{ &i8080Emulator::MOV, "MOV H,B", 1 },
	{ &i8080Emulator::MOV, "MOV H,C", 1 },
	{ &i8080Emulator::MOV, "MOV H,D", 1 },
	{ &i8080Emulator::MOV, "MOV H,E", 1 },
	{ &i8080Emulator::MOV, "MOV H,H", 1 },
	{ &i8080Emulator::MOV, "MOV H,L", 1 },
	{ &i8080Emulator::MOV, "MOV H,M", 1 },
	{ &i8080Emulator::MOV, "MOV H,A", 1 },
	{ &i8080Emulator::MOV, "MOV L,B", 1 },
	{ &i8080Emulator::MOV, "MOV L,C", 1 },
	{ &i8080Emulator::MOV, "MOV L,D", 1 },
	{ &i8080Emulator::MOV, "MOV L,E", 1 },
	{ &i8080Emulator::MOV, "MOV L,H", 1 },
	{ &i8080Emulator::MOV, "MOV L,L", 1 },
	{ &i8080Emulator::MOV, "MOV L,M", 1 },
	{ &i8080Emulator::MOV, "MOV L,A", 1 },
	{ &i8080Emulator::MOV, "MOV M,B", 1 },
	{ &i8080Emulator::MOV, "MOV M,C", 1 },
	{ &i8080Emulator::MOV, "MOV M,D", 1 },
	{ &i8080Emulator::MOV, "MOV M,E", 1 },
	{ &i8080Emulator::MOV, "MOV M,H", 1 },
	{ &i8080Emulator::MOV, "MOV M,L", 1 },
	{ &i8080Emulator::defaultOpcode,"HLT", 1 },
	{ &i8080Emulator::MOV,"MOV M,A", 1 },
	{ &i8080Emulator::MOV,"MOV A,B", 1 },
	{ &i8080Emulator::MOV,"MOV A,C", 1 },
	{ &i8080Emulator::MOV,"MOV A,D", 1 },
	{ &i8080Emulator::MOV,"MOV A,E", 1 },
	{ &i8080Emulator::MOV,"MOV A,H", 1 },
	{ &i8080Emulator::MOV,"MOV A,L", 1 },
	{ &i8080Emulator::MOV,"MOV A,M", 1 },
	{ &i8080Emulator::MOV,"MOV A,A", 1 },
	{ &i8080Emulator::defaultOpcode,"ADD B", 1 },
	{ &i8080Emulator::defaultOpcode,"ADD C", 1 },
	{ &i8080Emulator::defaultOpcode,"ADD D", 1 },
	{ &i8080Emulator::defaultOpcode,"ADD E", 1 },
	{ &i8080Emulator::defaultOpcode,"ADD H", 1 },
	{ &i8080Emulator::defaultOpcode,"ADD L", 1 },
	{ &i8080Emulator::defaultOpcode,"ADD M", 1 },
	{ &i8080Emulator::defaultOpcode,"ADD A", 1 },
	{ &i8080Emulator::defaultOpcode,"ADC B", 1 },
	{ &i8080Emulator::defaultOpcode,"ADC C", 1 },
	{ &i8080Emulator::defaultOpcode,"ADC D", 1 },
	{ &i8080Emulator::defaultOpcode,"ADC E", 1 },
	{ &i8080Emulator::defaultOpcode,"ADC H", 1 },
	{ &i8080Emulator::defaultOpcode,"ADC L", 1 },
	{ &i8080Emulator::defaultOpcode,"ADC M", 1 },
	{ &i8080Emulator::defaultOpcode,"ADC A", 1 },
	{ &i8080Emulator::defaultOpcode,"SUB B", 1 },
	{ &i8080Emulator::defaultOpcode,"SUB C", 1 },
	{ &i8080Emulator::defaultOpcode,"SUB D", 1 },
	{ &i8080Emulator::defaultOpcode,"SUB E", 1 },
	{ &i8080Emulator::defaultOpcode,"SUB H", 1 },
	{ &i8080Emulator::defaultOpcode,"SUB L", 1 },
	{ &i8080Emulator::defaultOpcode,"SUB M", 1 },
	{ &i8080Emulator::defaultOpcode,"SUB A", 1 },
	{ &i8080Emulator::SBB,"SBB B", 1 },
	{ &i8080Emulator::SBB,"SBB C", 1 },
	{ &i8080Emulator::SBB,"SBB D", 1 },
	{ &i8080Emulator::SBB,"SBB E", 1 },
	{ &i8080Emulator::SBB,"SBB H", 1 },
	{ &i8080Emulator::SBB,"SBB L", 1 },
	{ &i8080Emulator::SBB,"SBB M", 1 },
	{ &i8080Emulator::SBB,"SBB A", 1 },
	{ &i8080Emulator::ANA,"ANA B", 1 },
	{ &i8080Emulator::ANA,"ANA C", 1 },
	{ &i8080Emulator::ANA,"ANA D", 1 },
	{ &i8080Emulator::ANA,"ANA E", 1 },
	{ &i8080Emulator::ANA,"ANA H", 1 },
	{ &i8080Emulator::ANA,"ANA L", 1 },
	{ &i8080Emulator::ANA,"ANA M", 1 },
	{ &i8080Emulator::ANA,"ANA A", 1 },
	{ &i8080Emulator::XRA,"XRA B", 1 },
	{ &i8080Emulator::XRA,"XRA C", 1 },
	{ &i8080Emulator::XRA,"XRA D", 1 },
	{ &i8080Emulator::XRA,"XRA E", 1 },
	{ &i8080Emulator::XRA,"XRA H", 1 },
	{ &i8080Emulator::XRA,"XRA L", 1 },
	{ &i8080Emulator::XRA,"XRA M", 1 },
	{ &i8080Emulator::XRA,"XRA A", 1 },
	{ &i8080Emulator::ORA,"ORA B", 1 },
	{ &i8080Emulator::ORA,"ORA C", 1 },
	{ &i8080Emulator::ORA,"ORA D", 1 },
	{ &i8080Emulator::ORA,"ORA E", 1 },
	{ &i8080Emulator::ORA,"ORA H", 1 },
	{ &i8080Emulator::ORA,"ORA L", 1 },
	{ &i8080Emulator::ORA,"ORA M", 1 },
	{ &i8080Emulator::ORA,"ORA A", 1 },
	{ &i8080Emulator::CMP,"CMP B", 1 },
	{ &i8080Emulator::CMP,"CMP C", 1 },
	{ &i8080Emulator::CMP,"CMP D", 1 },
	{ &i8080Emulator::CMP,"CMP E", 1 },
	{ &i8080Emulator::CMP,"CMP H", 1 },
	{ &i8080Emulator::CMP,"CMP L", 1 },
	{ &i8080Emulator::CMP,"CMP M", 1 },
	{ &i8080Emulator::CMP,"CMP A", 1 },
	{ &i8080Emulator::RNZ,"RNZ", 1 },
	{ &i8080Emulator::POP,"POP B", 1 },
	{ &i8080Emulator::JNZ,"JNZ", 3 },
	{ &i8080Emulator::JMP,"JMP", 3 },
	{ &i8080Emulator::defaultOpcode,"CNZ", 3 },
	{ &i8080Emulator::PUSH,"PUSH B", 1 },
	{ &i8080Emulator::ADI,"ADI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 0", 1 },
	{ &i8080Emulator::defaultOpcode,"RZ", 1 },
	{ &i8080Emulator::RET,"RET", 1 },
	{ &i8080Emulator::JZ,"JZ", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"CZ", 3 },
	{ &i8080Emulator::CALL,"CALL", 3 },
	{ &i8080Emulator::defaultOpcode,"ACI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST1", 1 },
	{ &i8080Emulator::defaultOpcode,"RNC", 1 },
	{ &i8080Emulator::POP,"POP D", 1 },
	{ &i8080Emulator::defaultOpcode,"JNC", 3 },
	{ &i8080Emulator::OUT,"OUT", 2 },
	{ &i8080Emulator::defaultOpcode,"CNC", 3 },
	{ &i8080Emulator::PUSH,"PUSH D", 1 },
	{ &i8080Emulator::defaultOpcode,"SUI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 2", 1 },
	{ &i8080Emulator::defaultOpcode,"RC", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"JC", 3 },
	{ &i8080Emulator::IN,"IN D8", 2 },
	{ &i8080Emulator::defaultOpcode,"CC", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"SBI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 3", 1 },
	{ &i8080Emulator::defaultOpcode,"RPO", 1 },
	{ &i8080Emulator::POP,"POP H", 1 },
	{ &i8080Emulator::defaultOpcode,"JPO", 3 },
	{ &i8080Emulator::defaultOpcode,"XTHL", 1 },
	{ &i8080Emulator::defaultOpcode,"CPO", 3 },
	{ &i8080Emulator::PUSH,"PUSH H", 1 },
	{ &i8080Emulator::ANI,"ANI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 4", 1 },
	{ &i8080Emulator::defaultOpcode,"RPE", 1 },
	{ &i8080Emulator::defaultOpcode,"PCHL", 1 },
	{ &i8080Emulator::defaultOpcode,"JPE", 3 },
	{ &i8080Emulator::XCHG,"XCHG", 1 },
	{ &i8080Emulator::defaultOpcode,"CPE", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"XRI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 5", 1 },
	{ &i8080Emulator::defaultOpcode,"RP", 1 },
	{ &i8080Emulator::POP,"POP PSW", 1 },
	{ &i8080Emulator::defaultOpcode,"JP", 3 },
	{ &i8080Emulator::DI,"DI", 1 },
	{ &i8080Emulator::defaultOpcode,"CP", 3 },
	{ &i8080Emulator::PUSH,"PUSH PSW", 1 },
	{ &i8080Emulator::defaultOpcode,"ORI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 6", 1 },
	{ &i8080Emulator::defaultOpcode,"RM", 1 },
	{ &i8080Emulator::defaultOpcode,"SPHL", 1 },
	{ &i8080Emulator::defaultOpcode,"JM", 3 },
	{ &i8080Emulator::EI,"EI", 1 },
	{ &i8080Emulator::defaultOpcode,"CM", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::CPI,"CPI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 7", 1 }
	};
};

