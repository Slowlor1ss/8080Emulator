#pragma once
#include <cstdint>

enum class Registers8080;
enum class RegisterPairs8080;
class i8080Emulator;

class CPU
{
public:
	CPU(i8080Emulator* emulatorRef);

	//Debug
	void PrintRegister() const;

private:
	void UpdateFlags(const uint8_t& value);

	uint8_t ReadFlags() const;
	void SetFlags(uint8_t);

	uint16_t ReadRegisterPair(RegisterPairs8080 pair) const;
	void SetRegisterPair(RegisterPairs8080 pair, uint16_t value);
	void SetRegisterPair(RegisterPairs8080 pair, uint8_t LSByte, uint8_t MSByte);

	uint8_t ReadRegister(Registers8080 reg) const;
	void SetRegister(Registers8080 pair, uint8_t value);

	RegisterPairs8080 GetRegisterPairFromOpcode(uint8_t opcode) const;
	Registers8080 GetRegisterFromOpcode(uint8_t opcode, uint8_t shift = 0) const;

private:
	//temp
	bool printDebug{ false };
	friend int main(int argc, char** argv);
	friend class i8080Emulator;

	//no ownership
	i8080Emulator* m_I8080;

	//Dedicated Shift Hardware //Source: https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#dedicated-shift-hardware
	//The 8080 instruction set does not include opcodes for shifting.
	//An 8-bit pixel image must be shifted into a 16-bit word for the desired bit-position on the screen.
	//Space Invaders adds a hardware shift register to help with the math.
	uint16_t regShift{};
	uint8_t shiftOffset{}; //(3 bits)

	//I/O
	uint8_t inPort[4]{};
	uint8_t outPort[7]{};

	// Status
	uint8_t interruptsEnabled{};
	bool halt{};
	uint64_t clockCount{};

	//primary accumulator
	uint8_t a{};

	//three 16-bit register pairs (BC, DE, and HL)
	//that can function as six individual 8-bit registers
	//the reversed order of the two registers in each register pair is intentional due to the little-endianness
	//technically this is not very important as I'll only be accessing one byte at a time
	uint8_t c{};
	uint8_t b{};

	uint8_t e{};
	uint8_t d{};

	uint8_t l{}; //holds the least significant 8 bits
	uint8_t h{}; //holds most significant 8 bits
	//

	//stack pointer
	uint16_t sp{};
	//program counter
	uint16_t pc{};

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
	} ConditionBits{};

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
