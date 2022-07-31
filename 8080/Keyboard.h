#pragma once
#include <cstdint>
#include <SDL_stdinc.h>

class CPU;
class i8080Emulator;

class Keyboard
{
public:
	Keyboard();

	void Update(uint64_t currTime, CPU* CPU);

private:
	uint64_t m_LastInput;
	const Uint8* m_CurrentKeyState;

	static constexpr uint64_t update_frequency{ 1'000'000 / 30 }; //30hz
};

