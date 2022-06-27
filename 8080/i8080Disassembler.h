#pragma once
#include <iostream>

//holds the state of the CPU
struct State8080
{
	//primary accumulator
	uint8_t a;
	
	//three 16-bit register pairs (BC, DE, and HL)
	//that can function as six individual 8-bit registers
	//the reversed order of the two registers in each register pair is intentional due to the little-endianness
	//technically this is not very important as I'll only be reading one byte at a time
	//but I wanted to keep the little-endianness as in the original 8080
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
	struct ConditionBits
	{
		uint8_t c : 1;  //carry, set if the last addition operation resulted in a carry 
							// or last sub required borrow
		uint8_t : 1;
		uint8_t p : 1;  //parity bit, set if the number of true bits in the result is even
		uint8_t : 1;
		uint8_t ac : 1; //auxiliary carry bit for binary coded decimal arithmetic 
		uint8_t : 1;
		uint8_t z : 1;  //zero bit, set if the result is zero 
		uint8_t s : 1;  //sign bit, set if the result is negative
	};
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

	void CycleCpu();

	//Debug
	void PrintRegister() const;
	void PrintDisassembledRom() const;

private:

	State8080* m_Cpu;
	uint8_t* m_Memory;
	int64_t m_Fsize;

	uint8_t m_CurrentOpcode;

	//http://www.computerarcheology.com/Arcade/SpaceInvaders/RAMUse.html
	static constexpr int MEMORY_SIZE = 0x4000;
	static constexpr int ROM_SIZE = 0x2000;
	static constexpr int STACK_START = 0x2400;
	static constexpr int PROGRAM_START = 0x0000;

#pragma region OpcodeFunctions

#pragma region GenericOpcodeFunctions
	//Generic functions
	void RETURN(bool);
	void RESET(uint16_t);
	void CALLif(bool);
	void JUMP(bool);
	uint16_t POP();
	void DCX(char);
	void DAD(char);
	void DCR(uint8_t&);
	void MVI(uint8_t&);
	void PUSH(uint16_t);
	void LXI(char);
	void INX(char);
	void INR(byte_ref<uint16_t, Offset> reg);
#pragma endregion GenericOpcodeFunctions

	void NOP();
	void LXIB();
	void STAXB();
	void INXB();
	void INRB();
	void DCRB();
	void MVIB();
	void RLC();
	/*NOP*/
	void DADB();
	void LDAXB();
	void DCXB();
	void INRC();
	void DCRC();
	void MVIC();
	void RRC();
	/*NOP*/
	void LXID();
	void STAXD();
	void INXD();
	void INRD();
	void DCRD();
	void MVID();
	void RAL();
	/*NOP*/
	void DADD();
	void LDAXD();
	void DCXD();
	void INRE();
	void DCRE();
	void MVIE();
	void RAR();
	/*NOP*/
	void LXIH();
	void SHLD();
	void INXH();
	void INRH();
	void DCRH();
	void MVIH();
	void DAA();
	/*NOP*/
	void DADH();
	void LHLD();
	void DCXH();
	void INRL();
	void DCRL();
	void MVIL();
	void CMA();
	/*NOP*/
	void LXISP();
	void STA();
	void INXSP();
	void INRM();
	void DCRM();
	void MVIM();
	void STC();
	/*NOP*/
	void DADSP();
	void LDA();
	void DCXSP();
	void INRA();
	void DCRA();
	void MVIA();
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
	void POPB();
	void JNZ();
	void JMP();
	void CNZ();
	void PUSHB();
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
	void POPD();
	void JNC();
	void OUT();
	void CNC();
	void PUSHD();
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
	void POPH();
	void JPO();
	void XTHL();
	void CPO();
	void PUSHH();
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
	void POPPSW();
	void JP();
	void DI();
	void CP();
	void PUSHPSW();
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
	{ &i8080Emulator::defaultOpcode,"LXI B", 3 },
	{ &i8080Emulator::defaultOpcode,"STAX B", 1 },
	{ &i8080Emulator::defaultOpcode,"INX B", 1 },
	{ &i8080Emulator::defaultOpcode,"INR B", 1 },
	{ &i8080Emulator::defaultOpcode,"DCR B", 1 },
	{ &i8080Emulator::defaultOpcode,"MVI B", 2 },
	{ &i8080Emulator::defaultOpcode,"RLC", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"DAD B", 1 },
	{ &i8080Emulator::defaultOpcode,"LDAX B", 1 },
	{ &i8080Emulator::defaultOpcode,"DCX B", 1 },
	{ &i8080Emulator::defaultOpcode,"INR C", 1 },
	{ &i8080Emulator::defaultOpcode,"DCR C", 1 },
	{ &i8080Emulator::defaultOpcode,"MVI C", 2 },
	{ &i8080Emulator::defaultOpcode,"RRC", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"LXI D", 3 },
	{ &i8080Emulator::defaultOpcode,"STAX D", 1 },
	{ &i8080Emulator::defaultOpcode,"INX D", 1 },
	{ &i8080Emulator::defaultOpcode,"INR D D", 1 },
	{ &i8080Emulator::defaultOpcode,"DCR D", 1 },
	{ &i8080Emulator::defaultOpcode,"MVI D", 2 },
	{ &i8080Emulator::defaultOpcode,"RAL", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"DAD D", 1 },
	{ &i8080Emulator::defaultOpcode,"LDAX D", 1 },
	{ &i8080Emulator::defaultOpcode,"DCX D", 1 },
	{ &i8080Emulator::defaultOpcode,"INR E", 1 },
	{ &i8080Emulator::defaultOpcode,"DCR E", 1 },
	{ &i8080Emulator::defaultOpcode,"MVI E", 2 },
	{ &i8080Emulator::defaultOpcode,"RAR", 1 },
	{ &i8080Emulator::defaultOpcode,"RIM", 1 },
	{ &i8080Emulator::defaultOpcode,"LXI H", 3 },
	{ &i8080Emulator::defaultOpcode,"SHLD", 3 },
	{ &i8080Emulator::defaultOpcode,"INX H", 1 },
	{ &i8080Emulator::defaultOpcode,"INR H", 1 },
	{ &i8080Emulator::defaultOpcode,"DCR H", 1 },
	{ &i8080Emulator::defaultOpcode,"MVI H", 2 },
	{ &i8080Emulator::defaultOpcode,"DAA", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"DAD H", 1 },
	{ &i8080Emulator::defaultOpcode,"LHLD", 3 },
	{ &i8080Emulator::defaultOpcode,"DCX H", 1 },
	{ &i8080Emulator::defaultOpcode,"INR L", 1 },
	{ &i8080Emulator::defaultOpcode,"DCR L", 1 },
	{ &i8080Emulator::defaultOpcode,"MVI L", 2 },
	{ &i8080Emulator::defaultOpcode,"CMA", 1 },
	{ &i8080Emulator::defaultOpcode,"SIM", 1 },
	{ &i8080Emulator::defaultOpcode,"LXI SP", 3 },
	{ &i8080Emulator::defaultOpcode,"STA", 3 },
	{ &i8080Emulator::defaultOpcode,"INX SP", 1 },
	{ &i8080Emulator::defaultOpcode,"INR M", 1 },
	{ &i8080Emulator::defaultOpcode,"DCR M", 1 },
	{ &i8080Emulator::defaultOpcode,"MVI M", 2 },
	{ &i8080Emulator::defaultOpcode,"STC", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"DAD SP", 1 },
	{ &i8080Emulator::defaultOpcode,"LDA", 3 },
	{ &i8080Emulator::defaultOpcode,"DCX SP", 1 },
	{ &i8080Emulator::defaultOpcode,"INR A", 1 },
	{ &i8080Emulator::defaultOpcode,"DCR A", 1 },
	{ &i8080Emulator::defaultOpcode,"MVI A", 2 },
	{ &i8080Emulator::defaultOpcode,"CMC", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV B,B", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV B,C", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV B,D", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV B,E", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV B,H", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV B,L", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV B,M", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV B,A", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV C,B", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV C,C", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV C,D", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV C,E", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV C,H", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV C,L", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV C,M", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV C,A", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV D,B", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV D,C", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV D,D", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV D,E", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV D,H", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV D,L", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV D,M", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV D,A", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV E,B", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV E,C", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV E,D", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV E,E", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV E,H", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV E,L", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV E,M", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV E,A", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV H,B", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV H,C", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV H,D", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV H,E", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV H,H", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV H,L", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV H,M", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV H,A", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV L,B", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV L,C", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV L,D", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV L,E", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV L,H", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV L,L", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV L,M", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV L,A", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV M,B", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV M,C", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV M,D", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV M,E", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV M,H", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV M,L", 1 },
	{ &i8080Emulator::defaultOpcode,"HLT", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV M,A", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV A,B", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV A,C", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV A,D", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV A,E", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV A,H", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV A,L", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV A,M", 1 },
	{ &i8080Emulator::defaultOpcode,"MOV A,A", 1 },
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
	{ &i8080Emulator::defaultOpcode,"SBB B", 1 },
	{ &i8080Emulator::defaultOpcode,"SBB C", 1 },
	{ &i8080Emulator::defaultOpcode,"SBB D", 1 },
	{ &i8080Emulator::defaultOpcode,"SBB E", 1 },
	{ &i8080Emulator::defaultOpcode,"SBB H", 1 },
	{ &i8080Emulator::defaultOpcode,"SBB L", 1 },
	{ &i8080Emulator::defaultOpcode,"SBB M", 1 },
	{ &i8080Emulator::defaultOpcode,"SBB A", 1 },
	{ &i8080Emulator::defaultOpcode,"ANA B", 1 },
	{ &i8080Emulator::defaultOpcode,"ANA C", 1 },
	{ &i8080Emulator::defaultOpcode,"ANA D", 1 },
	{ &i8080Emulator::defaultOpcode,"ANA E", 1 },
	{ &i8080Emulator::defaultOpcode,"ANA H", 1 },
	{ &i8080Emulator::defaultOpcode,"ANA L", 1 },
	{ &i8080Emulator::defaultOpcode,"ANA M", 1 },
	{ &i8080Emulator::defaultOpcode,"ANA A", 1 },
	{ &i8080Emulator::defaultOpcode,"XRA B", 1 },
	{ &i8080Emulator::defaultOpcode,"XRA C", 1 },
	{ &i8080Emulator::defaultOpcode,"XRA D", 1 },
	{ &i8080Emulator::defaultOpcode,"XRA E", 1 },
	{ &i8080Emulator::defaultOpcode,"XRA H", 1 },
	{ &i8080Emulator::defaultOpcode,"XRA L", 1 },
	{ &i8080Emulator::defaultOpcode,"XRA M", 1 },
	{ &i8080Emulator::defaultOpcode,"XRA A", 1 },
	{ &i8080Emulator::defaultOpcode,"ORA B", 1 },
	{ &i8080Emulator::defaultOpcode,"ORA C", 1 },
	{ &i8080Emulator::defaultOpcode,"ORA D", 1 },
	{ &i8080Emulator::defaultOpcode,"ORA E", 1 },
	{ &i8080Emulator::defaultOpcode,"ORA H", 1 },
	{ &i8080Emulator::defaultOpcode,"ORA L", 1 },
	{ &i8080Emulator::defaultOpcode,"ORA M", 1 },
	{ &i8080Emulator::defaultOpcode,"ORA A", 1 },
	{ &i8080Emulator::defaultOpcode,"CMP B", 1 },
	{ &i8080Emulator::defaultOpcode,"CMP C", 1 },
	{ &i8080Emulator::defaultOpcode,"CMP D", 1 },
	{ &i8080Emulator::defaultOpcode,"CMP E", 1 },
	{ &i8080Emulator::defaultOpcode,"CMP H", 1 },
	{ &i8080Emulator::defaultOpcode,"CMP L", 1 },
	{ &i8080Emulator::defaultOpcode,"CMP M", 1 },
	{ &i8080Emulator::defaultOpcode,"CMP A", 1 },
	{ &i8080Emulator::defaultOpcode,"RNZ", 1 },
	{ &i8080Emulator::defaultOpcode,"POP B", 1 },
	{ &i8080Emulator::defaultOpcode,"JNZ", 3 },
	{ &i8080Emulator::defaultOpcode,"JMP", 3 },
	{ &i8080Emulator::defaultOpcode,"CNZ", 3 },
	{ &i8080Emulator::defaultOpcode,"PUSH B", 1 },
	{ &i8080Emulator::defaultOpcode,"ADI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 0", 1 },
	{ &i8080Emulator::defaultOpcode,"RZ", 1 },
	{ &i8080Emulator::defaultOpcode,"RET", 1 },
	{ &i8080Emulator::defaultOpcode,"JZ", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"CZ", 3 },
	{ &i8080Emulator::defaultOpcode,"CALL", 3 },
	{ &i8080Emulator::defaultOpcode,"ACI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST1", 1 },
	{ &i8080Emulator::defaultOpcode,"RNC", 1 },
	{ &i8080Emulator::defaultOpcode,"POP D", 1 },
	{ &i8080Emulator::defaultOpcode,"JNC", 3 },
	{ &i8080Emulator::defaultOpcode,"OUT", 2 },
	{ &i8080Emulator::defaultOpcode,"CNC", 3 },
	{ &i8080Emulator::defaultOpcode,"PUSH D", 1 },
	{ &i8080Emulator::defaultOpcode,"SUI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 2", 1 },
	{ &i8080Emulator::defaultOpcode,"RC", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"JC", 3 },
	{ &i8080Emulator::defaultOpcode,"IN D8", 2 },
	{ &i8080Emulator::defaultOpcode,"CC", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"SBI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 3", 1 },
	{ &i8080Emulator::defaultOpcode,"RPO", 1 },
	{ &i8080Emulator::defaultOpcode,"POP H", 1 },
	{ &i8080Emulator::defaultOpcode,"JPO", 3 },
	{ &i8080Emulator::defaultOpcode,"XTHL", 1 },
	{ &i8080Emulator::defaultOpcode,"CPO", 3 },
	{ &i8080Emulator::defaultOpcode,"PUSH H", 1 },
	{ &i8080Emulator::defaultOpcode,"ANI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 4", 1 },
	{ &i8080Emulator::defaultOpcode,"RPE", 1 },
	{ &i8080Emulator::defaultOpcode,"PCHL", 1 },
	{ &i8080Emulator::defaultOpcode,"JPE", 3 },
	{ &i8080Emulator::defaultOpcode,"XCHG", 1 },
	{ &i8080Emulator::defaultOpcode,"CPE", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"XRI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 5", 1 },
	{ &i8080Emulator::defaultOpcode,"RP", 1 },
	{ &i8080Emulator::defaultOpcode,"POP PSW", 1 },
	{ &i8080Emulator::defaultOpcode,"JP", 3 },
	{ &i8080Emulator::defaultOpcode,"DI", 1 },
	{ &i8080Emulator::defaultOpcode,"CP", 3 },
	{ &i8080Emulator::defaultOpcode,"PUSH PSW", 1 },
	{ &i8080Emulator::defaultOpcode,"ORI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 6", 1 },
	{ &i8080Emulator::defaultOpcode,"RM", 1 },
	{ &i8080Emulator::defaultOpcode,"SPHL", 1 },
	{ &i8080Emulator::defaultOpcode,"JM", 3 },
	{ &i8080Emulator::defaultOpcode,"EI", 1 },
	{ &i8080Emulator::defaultOpcode,"CM", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::defaultOpcode,"CPI", 2 },
	{ &i8080Emulator::defaultOpcode,"RST 7", 1 }
	};
};

