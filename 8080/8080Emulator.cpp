//External includes
#ifdef _DEBUG
#include <vld.h>
#endif
#pragma warning(push)
#pragma warning(disable: 26812) //disable warnings
#include "SDL.h"
#include "SDL_surface.h"
#pragma warning(pop)
#undef main

//Standard includes
#include <thread>
#include <chrono>
#include <fstream>

//Project includes
#include "CPU.h"
#include "i8080Disassembler.h"
#include "Display.h"


using namespace std::chrono;

uint64_t GetCurrentTime(time_point<high_resolution_clock>* StartTime) {
	auto TimeDifference = high_resolution_clock::now() - *StartTime;
	return duration_cast<microseconds>(TimeDifference).count(); // Get Microseconds
}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	i8080Emulator* i8080;
    if (argc == 2)
        i8080 = new i8080Emulator{ argv[1] };
    else
        i8080 = new i8080Emulator{ "../Roms/invadersfull.rom" };

    //i8080->PrintDisassembledRom();
	//bool failed{};
	//do
	//{
	//	failed = !i8080->CycleCpu();
	//}
	//while (!failed);

	SDL_Event ev;
	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);
	uint64_t LastInput = 0;


	Display* Disp = new Display("Intel 8080", 224, 256, 2);

	uint64_t CurrentTime = 0;
	uint64_t LastThrottle = 0;
	uint64_t LastDraw = 0;
	//uint64_t LastDebug = 0;
	bool DrawFull = false;
	uint32_t ClocksPerMS = 3000; // 3 MHz Sweet Spot
	uint64_t ClockCompensation = 0;
	auto StartTime = high_resolution_clock::now();

	while (!i8080->m_Cpu->halt)
	{
			CurrentTime = GetCurrentTime(&StartTime);

			if (CurrentTime - LastThrottle <= 4000) { // Check every 4 ms
				if (i8080->m_Cpu->clockCount >= (ClocksPerMS << 2) + ClockCompensation) { // Throttle CPU
					uint64_t usToSleep = 4000 - (CurrentTime - LastThrottle); // Sleep for the rest of the 4 ms
					std::this_thread::sleep_for(microseconds(usToSleep));
					LastThrottle = GetCurrentTime(&StartTime);
					i8080->m_Cpu->clockCount = 0;

					ClockCompensation = (ClocksPerMS * ((LastThrottle - CurrentTime) - usToSleep)) / 1000;
				}
			}
			else { // Host CPU is slower or equal to i8080
				LastThrottle = CurrentTime;
				i8080->m_Cpu->clockCount = 0;
			}

			if (/*!i8080->m_Cpu->printDebug && */CurrentTime - LastDraw > 1000000 / 120 || LastDraw > CurrentTime) { // 120 Hz - Manage Screen (Half screen in a cycle, then end screen in another)
				LastDraw = CurrentTime;

				if (DrawFull) {
					Disp->Update(i8080->m_Memory + i8080Emulator::STACK_START);
					i8080->Interrupt(1);
				}
				else
					i8080->Interrupt(0);

				DrawFull = !DrawFull;
			}

			if (CurrentTime - LastInput > 1000000 / 30 || LastInput > CurrentTime) { // 30 Hz - Manage Events
				LastInput = CurrentTime;
				while (SDL_PollEvent(&ev)) {
					if (ev.type == SDL_QUIT) {
						i8080->m_Cpu->halt = true;
					}
				}

				// Cleanup before input
				i8080->m_Cpu->inPort[0] &= 0b10001111;
				i8080->m_Cpu->inPort[1] &= 0b10001000;
				i8080->m_Cpu->inPort[2] &= 0b10001011;

				if (Keyboard[SDL_SCANCODE_SPACE]) { // Fire
					i8080->m_Cpu->inPort[0] |= 1 << 4;
					i8080->m_Cpu->inPort[1] |= 1 << 4;
					i8080->m_Cpu->inPort[2] |= 1 << 4; // P2
				}

				if (Keyboard[SDL_SCANCODE_A] || Keyboard[SDL_SCANCODE_LEFT]) { // Left
					i8080->m_Cpu->inPort[0] |= 1 << 5;
					i8080->m_Cpu->inPort[1] |= 1 << 5;
					i8080->m_Cpu->inPort[2] |= 1 << 5; // P2
				}

				if (Keyboard[SDL_SCANCODE_D] || Keyboard[SDL_SCANCODE_RIGHT]) { // Right
					i8080->m_Cpu->inPort[0] |= 1 << 6;
					i8080->m_Cpu->inPort[1] |= 1 << 6;
					i8080->m_Cpu->inPort[2] |= 1 << 6; // P2
				}

				if (Keyboard[SDL_SCANCODE_RETURN]) // Credit
					i8080->m_Cpu->inPort[1] |= 1 << 0;

				if (Keyboard[SDL_SCANCODE_1]) // 1P Start
					i8080->m_Cpu->inPort[1] |= 1 << 2;

				if (Keyboard[SDL_SCANCODE_2]) // 2P Start
					i8080->m_Cpu->inPort[1] |= 1 << 1;

				if (Keyboard[SDL_SCANCODE_DELETE]) // Tilt
					i8080->m_Cpu->inPort[2] |= 1 << 2;
			}

			////Keyboard
			//if (CurrentTime - LastInput > 1000000 / 30 || LastInput > CurrentTime) { // 30 Hz - Manage Events
			//
			//}
			//
			////audio
			//if ((CurrentTime - LastSound > 1000000 / 60 || LastSound > CurrentTime)) { // 30 Hz Audio
			//}
			//
			//if (CurrentTime - LastDebugPrint > 5000000 || LastDebugPrint > CurrentTime) { // 5 Seconds - Manage occasional prints
			//	float Duration = (CurrentTime - LastDebugPrint) / 1000000;
			//	uint64_t ClocksPerSec = cpu.ClockCount / Duration;
			//	LastDebugPrint = CurrentTime;
			//	printf("[INFO] Running at @%f MHz (%lu Instructions Per Second)\n", (float)ClocksPerSec / 1000000, (uint64_t)(cpu.InstructionCount / Duration));
			//
			//	i8080->m_Cpu->clockCount = 0;
			//	cpu.InstructionCount = 0;
			//}

			//if (CurrentTime - LastDebug > 315000)
			//{
			//	LastDebug = CurrentTime;
			//	std::cout << "PC:  " << i8080->m_Cpu->pc;
			//}

		i8080->CycleCpu();
	}



	delete i8080;
	delete Disp;
	SDL_Quit();

    return 0;
}
