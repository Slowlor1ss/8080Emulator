#include "Keyboard.h"
#include "CPU.h"
#include "i8080Emulator.h"

Keyboard::Keyboard(CPU* CPUref)
	: m_LastInput(0)
	, m_pCPUref(CPUref)
{
}

void Keyboard::KeyUp(int key)
{
	m_KeyboardState[key] = false;
}

void Keyboard::KeyDown(int key)
{
	m_KeyboardState[key] = true;
}

void Keyboard::Update(uint64_t currTime)
{
	if (currTime - m_LastInput > update_frequency) {
		m_LastInput = currTime;

		//for more info on what bit of which port does what see:
		//https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#inputs

		// Cleanup before input
		m_pCPUref->inPort[0] &= 0b1000'1111;
		m_pCPUref->inPort[1] &= 0b1000'1000;
		m_pCPUref->inPort[2] &= 0b1000'1011;


		if (m_KeyboardState[Key_Space]) { // Fire
			m_pCPUref->inPort[0] |= 1 << 4;
			m_pCPUref->inPort[1] |= 1 << 4;
			m_pCPUref->inPort[2] |= 1 << 4; // P2
		}

		if (m_KeyboardState[Key_A]) { // Left
			m_pCPUref->inPort[0] |= 1 << 5;
			m_pCPUref->inPort[1] |= 1 << 5;
			m_pCPUref->inPort[2] |= 1 << 5; // P2
		}

		if (m_KeyboardState[Key_D]) { // Right
			m_pCPUref->inPort[0] |= 1 << 6;
			m_pCPUref->inPort[1] |= 1 << 6;
			m_pCPUref->inPort[2] |= 1 << 6; // P2
		}

		if (m_KeyboardState[Key_Shift]) // Credit
			m_pCPUref->inPort[1] |= 1 << 0;

		if (m_KeyboardState[Key_1]) // 1P Start
			m_pCPUref->inPort[1] |= 1 << 2;

		if (m_KeyboardState[Key_2]) // 2P Start
			m_pCPUref->inPort[1] |= 1 << 1;

		if (m_KeyboardState[Key_Delete]) // Tilt
			m_pCPUref->inPort[2] |= 1 << 2;
	}
}
