#pragma once
#include <chrono>
#include <iostream>

class Keyboard;
class Display;
class CPU;

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
	i8080Emulator();
	i8080Emulator(const char* path, bool consoleProgram);
	~i8080Emulator();

	i8080Emulator(const i8080Emulator& other) = delete;
	i8080Emulator(i8080Emulator&& other) noexcept = delete;
	i8080Emulator& operator=(const i8080Emulator& other) = delete;
	i8080Emulator& operator=(i8080Emulator&& other) noexcept = delete;

	bool LoadRom(bool consoleProgram, const char* path);

	void Update();
	void Stop();

	void Interrupt(uint8_t ID);

	void MemWrite(uint16_t address, uint8_t data);
	uint8_t ReadMem(uint16_t address);

	Display* GetDisplay() const {return m_pDisplay;}
	Keyboard* GetKeyboard() const {return m_pKeyboard;}

	void SetClockSpeed(uint64_t clocksPerMs) { m_ClocksPerMs = clocksPerMs; }
	uint64_t GetClockSpeed() const { return m_ClocksPerMs; }

	//Debug
	void PrintDisassembledRom() const;

private:
	void ThrottleCPU(uint64_t currentTime);
	void CycleCpu();
	void Syscall(uint16_t ID);

	bool m_ConsoleProg;

	CPU* m_pCpu;
	uint8_t* m_Memory;
	int64_t m_CurrRomSize;
	uint16_t m_ProgramStart = 0x0000;

	uint8_t m_CurrentOpcode;


	uint64_t m_ClocksPerMs;
	std::chrono::time_point<std::chrono::steady_clock> m_StartTime{};
	uint64_t m_LastThrottle{};

	Display* m_pDisplay;
	Keyboard* m_pKeyboard;

	//http://www.computerarcheology.com/Arcade/SpaceInvaders/RAMUse.html
	static constexpr int memory_size = 0x10000;
	static constexpr int rom_size = 0x2000;
	static constexpr int stack_start = 0x2400;


#pragma region OpcodeFunctions

#pragma region GenericOpcodeFunctions
	//Generic functions
	void RETURN(bool);
	void RESTART(uint16_t);
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
	void STAXB();
	void RLC();
	void LDAXB();
	void RRC();
	void STAXD();
	void RAL();
	void LDAXD();
	void RAR();
	void SHLD();
	void DAA();
	void LHLD();
	void CMA();
	void STA();
	void STC();
	void LDA();
	void CMC();
	void MOV();
	void HLT();
	void ADD();
	void ADC();
	void SUB();
	void SBB();
	void ANA();
	void XRA();
	void ORA();
	void CMP();
	void RNZ();
	void JNZ();
	void JMP();
	void CNZ();
	void ADI();
	void RST();
	void RZ();
	void RET();
	void JZ();
	void CZ();
	void CALL();
	void ACI();
	void RNC();
	void JNC();
	void OUT();
	void CNC();
	void SUI();
	void RC();
	void JC();
	void IN();
	void CC();
	void SBI();
	void RPO();
	void JPO();
	void XTHL();
	void CPO();
	void ANI();
	void RPE();
	void PCHL();
	void JPE();
	void XCHG();
	void CPE();
	void XRI();
	void RP();
	void JP();
	void DI();
	void CP();
	void ORI();
	void RM();
	void SPHL();
	void JM();
	void EI();
	void CM();
	void CPI();

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
	{ &i8080Emulator::RLC,"RLC", 1 },
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
	{ &i8080Emulator::RAL,"RAL", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::DAD,"DAD D", 1 },
	{ &i8080Emulator::LDAXD,"LDAX D", 1 },
	{ &i8080Emulator::DCX,"DCX D", 1 },
	{ &i8080Emulator::INR,"INR E", 1 },
	{ &i8080Emulator::DCR,"DCR E", 1 },
	{ &i8080Emulator::MVI,"MVI E", 2 },
	{ &i8080Emulator::RAR,"RAR", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::LXI,"LXI H", 3 },
	{ &i8080Emulator::SHLD,"SHLD", 3 },
	{ &i8080Emulator::INX,"INX H", 1 },
	{ &i8080Emulator::INR,"INR H", 1 },
	{ &i8080Emulator::DCR,"DCR H", 1 },
	{ &i8080Emulator::MVI,"MVI H", 2 },
	{ &i8080Emulator::DAA,"DAA", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::DAD,"DAD H", 1 },
	{ &i8080Emulator::LHLD,"LHLD", 3 },
	{ &i8080Emulator::DCX,"DCX H", 1 },
	{ &i8080Emulator::INR,"INR L", 1 },
	{ &i8080Emulator::DCR,"DCR L", 1 },
	{ &i8080Emulator::MVI,"MVI L", 2 },
	{ &i8080Emulator::CMA,"CMA", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::LXI,"LXI SP", 3 },
	{ &i8080Emulator::STA,"STA", 3 },
	{ &i8080Emulator::INX,"INX SP", 1 },
	{ &i8080Emulator::INR,"INR M", 1 },
	{ &i8080Emulator::DCR,"DCR M", 1 },
	{ &i8080Emulator::MVI,"MVI M", 2 },
	{ &i8080Emulator::STC,"STC", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::DAD,"DAD SP", 1 },
	{ &i8080Emulator::LDA,"LDA", 3 },
	{ &i8080Emulator::DCX,"DCX SP", 1 },
	{ &i8080Emulator::INR,"INR A", 1 },
	{ &i8080Emulator::DCR,"DCR A", 1 },
	{ &i8080Emulator::MVI,"MVI A", 2 },
	{ &i8080Emulator::CMC,"CMC", 1 },
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
	{ &i8080Emulator::HLT,"HLT", 1 },
	{ &i8080Emulator::MOV,"MOV M,A", 1 },
	{ &i8080Emulator::MOV,"MOV A,B", 1 },
	{ &i8080Emulator::MOV,"MOV A,C", 1 },
	{ &i8080Emulator::MOV,"MOV A,D", 1 },
	{ &i8080Emulator::MOV,"MOV A,E", 1 },
	{ &i8080Emulator::MOV,"MOV A,H", 1 },
	{ &i8080Emulator::MOV,"MOV A,L", 1 },
	{ &i8080Emulator::MOV,"MOV A,M", 1 },
	{ &i8080Emulator::MOV,"MOV A,A", 1 },
	{ &i8080Emulator::ADD,"ADD B", 1 },
	{ &i8080Emulator::ADD,"ADD C", 1 },
	{ &i8080Emulator::ADD,"ADD D", 1 },
	{ &i8080Emulator::ADD,"ADD E", 1 },
	{ &i8080Emulator::ADD,"ADD H", 1 },
	{ &i8080Emulator::ADD,"ADD L", 1 },
	{ &i8080Emulator::ADD,"ADD M", 1 },
	{ &i8080Emulator::ADD,"ADD A", 1 },
	{ &i8080Emulator::ADC,"ADC B", 1 },
	{ &i8080Emulator::ADC,"ADC C", 1 },
	{ &i8080Emulator::ADC,"ADC D", 1 },
	{ &i8080Emulator::ADC,"ADC E", 1 },
	{ &i8080Emulator::ADC,"ADC H", 1 },
	{ &i8080Emulator::ADC,"ADC L", 1 },
	{ &i8080Emulator::ADC,"ADC M", 1 },
	{ &i8080Emulator::ADC,"ADC A", 1 },
	{ &i8080Emulator::SUB,"SUB B", 1 },
	{ &i8080Emulator::SUB,"SUB C", 1 },
	{ &i8080Emulator::SUB,"SUB D", 1 },
	{ &i8080Emulator::SUB,"SUB E", 1 },
	{ &i8080Emulator::SUB,"SUB H", 1 },
	{ &i8080Emulator::SUB,"SUB L", 1 },
	{ &i8080Emulator::SUB,"SUB M", 1 },
	{ &i8080Emulator::SUB,"SUB A", 1 },
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
	{ &i8080Emulator::CNZ,"CNZ", 3 },
	{ &i8080Emulator::PUSH,"PUSH B", 1 },
	{ &i8080Emulator::ADI,"ADI", 2 },
	{ &i8080Emulator::RST,"RST 0", 1 },
	{ &i8080Emulator::RZ,"RZ", 1 },
	{ &i8080Emulator::RET,"RET", 1 },
	{ &i8080Emulator::JZ,"JZ", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::CZ,"CZ", 3 },
	{ &i8080Emulator::CALL,"CALL", 3 },
	{ &i8080Emulator::ACI,"ACI", 2 },
	{ &i8080Emulator::RST,"RST1", 1 },
	{ &i8080Emulator::RNC,"RNC", 1 },
	{ &i8080Emulator::POP,"POP D", 1 },
	{ &i8080Emulator::JNC,"JNC", 3 },
	{ &i8080Emulator::OUT,"OUT", 2 },
	{ &i8080Emulator::CNC,"CNC", 3 },
	{ &i8080Emulator::PUSH,"PUSH D", 1 },
	{ &i8080Emulator::SUI,"SUI", 2 },
	{ &i8080Emulator::RST,"RST 2", 1 },
	{ &i8080Emulator::RC,"RC", 1 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::JC,"JC", 3 },
	{ &i8080Emulator::IN,"IN D8", 2 },
	{ &i8080Emulator::CC,"CC", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::SBI,"SBI", 2 },
	{ &i8080Emulator::RST,"RST 3", 1 },
	{ &i8080Emulator::RPO,"RPO", 1 },
	{ &i8080Emulator::POP,"POP H", 1 },
	{ &i8080Emulator::JPO,"JPO", 3 },
	{ &i8080Emulator::XTHL,"XTHL", 1 },
	{ &i8080Emulator::CPO,"CPO", 3 },
	{ &i8080Emulator::PUSH,"PUSH H", 1 },
	{ &i8080Emulator::ANI,"ANI", 2 },
	{ &i8080Emulator::RST,"RST 4", 1 },
	{ &i8080Emulator::RPE,"RPE", 1 },
	{ &i8080Emulator::PCHL,"PCHL", 1 },
	{ &i8080Emulator::JPE,"JPE", 3 },
	{ &i8080Emulator::XCHG,"XCHG", 1 },
	{ &i8080Emulator::CPE,"CPE", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::XRI,"XRI", 2 },
	{ &i8080Emulator::RST,"RST 5", 1 },
	{ &i8080Emulator::RP,"RP", 1 },
	{ &i8080Emulator::POP,"POP PSW", 1 },
	{ &i8080Emulator::JP,"JP", 3 },
	{ &i8080Emulator::DI,"DI", 1 },
	{ &i8080Emulator::CP,"CP", 3 },
	{ &i8080Emulator::PUSH,"PUSH PSW", 1 },
	{ &i8080Emulator::ORI,"ORI", 2 },
	{ &i8080Emulator::RST,"RST 6", 1 },
	{ &i8080Emulator::RM,"RM", 1 },
	{ &i8080Emulator::SPHL,"SPHL", 1 },
	{ &i8080Emulator::JM,"JM", 3 },
	{ &i8080Emulator::EI,"EI", 1 },
	{ &i8080Emulator::CM,"CM", 3 },
	{ &i8080Emulator::defaultOpcode,"", 1 },
	{ &i8080Emulator::CPI,"CPI", 2 },
	{ &i8080Emulator::RST,"RST 7", 1 }
	};

	//from https://github.com/superzazu/8080
	inline static constexpr uint8_t InstructionCycles[]{
		//  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
			4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 7,  5,  5,  5,  7,  4,  // 0
			4,  10, 7,  5,  5,  5,  7,  4,  4,  10, 7,  5,  5,  5,  7,  4,  // 1
			4,  10, 16, 5,  5,  5,  7,  4,  4,  10, 16, 5,  5,  5,  7,  4,  // 2
			4,  10, 13, 5,  10, 10, 10, 4,  4,  10, 13, 5,  5,  5,  7,  4,  // 3
			5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  // 4
			5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  // 5
			5,  5,  5,  5,  5,  5,  7,  5,  5,  5,  5,  5,  5,  5,  7,  5,  // 6
			7,  7,  7,  7,  7,  7,  7,  7,  5,  5,  5,  5,  5,  5,  7,  5,  // 7
			4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  // 8
			4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  // 9
			4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  // A
			4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,  // B
			5,  10, 10, 10, 11, 11, 7,  11, 5,  10, 10, 10, 11, 11, 7,  11, // C
			5,  10, 10, 10, 11, 11, 7,  11, 5,  10, 10, 10, 11, 11, 7,  11, // D
			5,  10, 10, 18, 11, 11, 7,  11, 5,  5,  10, 5,  11, 11, 7,  11, // E
			5,  10, 10, 4,  11, 11, 7,  11, 5,  5,  10, 4,  11, 11, 7,  11  // F
	};
};

