#include "CPU.h"
#include <bitset>
#include <cassert>
#include <iomanip>
#include <iostream>
#include "i8080Disassembler.h"

CPU::CPU(i8080Emulator* emulatorRef)
	: m_I8080(emulatorRef)
{
}

//Used for debugging
void CPU::PrintRegister() const
{
	std::cout << "Main registers\n";
	std::cout << "A: " << std::bitset<8>(a) << " | " << "Flags: " << std::bitset<8>((ConditionBits.s << 7) | (ConditionBits.z << 6) | (ConditionBits.ac << 4) | (ConditionBits.p << 2) | (ConditionBits.c << 0)) << '\n';
	std::cout << "B: " << std::bitset<8>(b) << " | " << "C: " << std::bitset<8>(c) << '\n';
	std::cout << "D: " << std::bitset<8>(d) << " | " << "E: " << std::bitset<8>(e) << '\n';
	std::cout << "H: " << std::bitset<8>(h) << " | " << "L: " << std::bitset<8>(l) << '\n';
	std::cout << '\n';
	std::cout << "Index registers (Stack Pointer)\n";
	std::cout << "SP: " << std::setw(4) << std::setfill('0') << std::hex << sp << '\n';
	std::cout << '\n';
	std::cout << "Program counter\n";
	std::cout << "PC: " << std::setw(4) << std::setfill('0') << std::hex << pc << '\n';
	std::cout << '\n';
}

void CPU::UpdateFlags(const uint8_t& value)
{
	ConditionBits.s = (value & 128); //set if negative
	ConditionBits.z = (value == 0); //set if zero

	//TODO: clean up
	//find if the parity bit for 8bits (amount of true bits even or odd)
	//https://www.geeksforgeeks.org/finding-the-parity-of-a-number-efficiently/
	//uint8_t y = value ^ (value >> 1);
	//y = y ^ (y >> 2);
	//y = y ^ (y >> 4);
	//ConditionBits.p = !(y&1); //true if number of true bits is even

	//find if the parity bit for 8bits (amount of true bits even or odd)
	uint8_t ones = 0;
	for (int shift = 0; shift < 8; ++shift) {
		if ((value >> shift) & 1)
			++ones;
	}
	ConditionBits.p = !(ones & 1);
}

uint8_t CPU::ReadFlags() const
{
	return static_cast<uint8_t>(
		(ConditionBits.s << 7)
		| (ConditionBits.z << 6)
		| (ConditionBits.ac << 4)
		| (ConditionBits.p << 2)
		| (ConditionBits.c << 0));
}

void CPU::SetFlags(uint8_t data)
{
	ConditionBits.s = data & 0b10000000;
	ConditionBits.z = data & 0b01000000;
	ConditionBits.ac = data & 0b00010000;
	ConditionBits.p = data & 0b0100;
	ConditionBits.c = data & 0b0001;
}

uint16_t CPU::ReadRegisterPair(RegisterPairs8080 pair) const
{
	uint8_t LSByte{}, MSByte{};
	switch (pair)
	{
	case RegisterPairs8080::BC:
		MSByte = b;
		LSByte = c;
		break;
	case RegisterPairs8080::DE:
		MSByte = d;
		LSByte = e;
		break;
	case RegisterPairs8080::HL:
		MSByte = h;
		LSByte = l;
		break;
	case RegisterPairs8080::SP:
		return sp;
	default:
		assert(!"should never get here!");
	}

	return (uint16_t(MSByte << 8) | LSByte);
}

void CPU::SetRegisterPair(RegisterPairs8080 pair, uint16_t value)
{
	const uint8_t MSByte = (value & 0xFF00) >> 8;
	const uint8_t LSByte = (value & 0x00FF);

	SetRegisterPair(pair, LSByte, MSByte);
}

void CPU::SetRegisterPair(RegisterPairs8080 pair, uint8_t LSByte, uint8_t MSByte)
{
	switch (pair)
	{
	case RegisterPairs8080::BC:
		b = MSByte;
		c = LSByte;
		break;
	case RegisterPairs8080::DE:
		d = MSByte;
		e = LSByte;
		break;
	case RegisterPairs8080::HL:
		h = MSByte;
		l = LSByte;
		break;
	case RegisterPairs8080::SP:
		sp = uint16_t(m_I8080->ReadMem(pc + 2) << 8) | m_I8080->ReadMem(pc + 1);
		break;
	default:
		assert(!"should never get here!");
	}
}

uint8_t CPU::ReadRegister(Registers8080 reg) const
{
	switch (reg)
	{
	case Registers8080::B:
		return b;
	case Registers8080::C:
		return c;
	case Registers8080::D:
		return d;
	case Registers8080::E:
		return e;
	case Registers8080::H:
		return h;
	case Registers8080::L:
		return l;
	case Registers8080::MEM:
		return (m_I8080->ReadMem(ReadRegisterPair(RegisterPairs8080::HL)));
	case Registers8080::A:
		return a;
	default:
		assert(!"should never get here!");
		return 0;
	}
}

void CPU::SetRegister(Registers8080 reg, uint8_t value)
{
	switch (reg)
	{
	case Registers8080::B:
		b = value;
		break;
	case Registers8080::C:
		c = value;
		break;
	case Registers8080::D:
		d = value;
		break;
	case Registers8080::E:
		e = value;
		break;
	case Registers8080::H:
		h = value;
		break;
	case Registers8080::L:
		l = value;
		break;
	case Registers8080::MEM:
		m_I8080->MemWrite(ReadRegisterPair(RegisterPairs8080::HL), value);
		break;
	case Registers8080::A:
		a = value;
		break;
	default:
		assert(!"should never get here!");
		break;
	}
}

RegisterPairs8080 CPU::GetRegisterPairFromOpcode(const uint8_t opcode) const
{
	//based 8080-Programmers-Manual page 25
	return static_cast<RegisterPairs8080>((opcode & 0b00110000) >> 4);
}

Registers8080 CPU::GetRegisterFromOpcode(uint8_t opcode, uint8_t shift) const
{
	//based 8080-Programmers-Manual page 16-17
	//shift can be specified as some functions like INR DCR MVI use the second 3 bits instead of the first 3
	return static_cast<Registers8080>((opcode & (0b00000111 << shift)) >> shift);
}

