#include "Keyboard.h"
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include "CPU.h"
#include "i8080Emulator.h"

Keyboard::Keyboard()
	: m_LastInput(0)
	, m_CurrentKeyState(SDL_GetKeyboardState(nullptr))
{
}

void Keyboard::Update(uint64_t currTime, CPU* CPU)
{

	if (currTime - m_LastInput > update_frequency) {
		m_LastInput = currTime;

		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				CPU->halt = true;
			}
		}

		//for more info on what bit of which port does what see:
		//https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#inputs

		// Cleanup before input
		CPU->inPort[0] &= 0b1000'1111;
		CPU->inPort[1] &= 0b1000'1000;
		CPU->inPort[2] &= 0b1000'1011;

		if (m_CurrentKeyState[SDL_SCANCODE_SPACE]) { // Fire
			CPU->inPort[0] |= 1 << 4;
			CPU->inPort[1] |= 1 << 4;
			CPU->inPort[2] |= 1 << 4; // P2
		}

		if (m_CurrentKeyState[SDL_SCANCODE_A] || m_CurrentKeyState[SDL_SCANCODE_LEFT]) { // Left
			CPU->inPort[0] |= 1 << 5;
			CPU->inPort[1] |= 1 << 5;
			CPU->inPort[2] |= 1 << 5; // P2
		}

		if (m_CurrentKeyState[SDL_SCANCODE_D] || m_CurrentKeyState[SDL_SCANCODE_RIGHT]) { // Right
			CPU->inPort[0] |= 1 << 6;
			CPU->inPort[1] |= 1 << 6;
			CPU->inPort[2] |= 1 << 6; // P2
		}

		if (m_CurrentKeyState[SDL_SCANCODE_RETURN]) // Credit
			CPU->inPort[1] |= 1 << 0;

		if (m_CurrentKeyState[SDL_SCANCODE_1]) // 1P Start
			CPU->inPort[1] |= 1 << 2;

		if (m_CurrentKeyState[SDL_SCANCODE_2]) // 2P Start
			CPU->inPort[1] |= 1 << 1;

		if (m_CurrentKeyState[SDL_SCANCODE_DELETE]) // Tilt
			CPU->inPort[2] |= 1 << 2;
	}

}
