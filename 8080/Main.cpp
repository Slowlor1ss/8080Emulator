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
#include "i8080Emulator.h"


using namespace std::chrono;

//uint64_t GetCurrentTime(time_point<high_resolution_clock>* StartTime) {
//	auto TimeDifference = high_resolution_clock::now() - *StartTime;
//	return duration_cast<microseconds>(TimeDifference).count(); // Get Microseconds
//}

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO);

	i8080Emulator* i8080;
	if (argc == 2)
		i8080 = new i8080Emulator{ argv[1], false };
	else
		i8080 = new i8080Emulator{ "../Roms/invaders.rom", false };

	i8080->Start();

	//i8080->PrintDisassembledRom();
	//bool failed{};
	//do
	//{
	//	failed = !i8080->CycleCpu();
	//}
	//while (!failed);
	//
	//SDL_Event ev;
	//const uint8_t* Keyboard = SDL_GetKeyboardState(nullptr);
	//uint64_t LastInput = 0;
	//
	//
	//Keyboard* keyboard = new Keyboard();
	//
	//auto* Disp = new Display("Intel 8080", 224, 256, 2);
	//
	//uint64_t CurrentTime = 0;
	//uint64_t LastThrottle = 0;
	////uint64_t LastDraw = 0;
	////uint64_t LastDebug = 0;
	////bool DrawFull = false;
	//uint64_t ClocksPerMS = 2'000'000; //2 MHz
	//uint64_t ClockCompensation = 0;
	//auto StartTime = high_resolution_clock::now();
	//
	//while (!i8080->m_Cpu->halt)
	//{
	//
	//if(!i8080->m_ConsoleProg){
	//
	//	CurrentTime = GetCurrentTime(&StartTime);
	//
	//	if (CurrentTime - LastThrottle <= 4000) { // Check every 4 ms
	//		if (i8080->m_Cpu->clockCount >= (ClocksPerMS << 2) + ClockCompensation) { // Throttle CPU
	//			uint64_t usToSleep = 4000 - (CurrentTime - LastThrottle); // Sleep for the rest of the 4 ms
	//			std::this_thread::sleep_for(microseconds(usToSleep));
	//			LastThrottle = GetCurrentTime(&StartTime);
	//			i8080->m_Cpu->clockCount = 0;
	//
	//			ClockCompensation = (ClocksPerMS * ((LastThrottle - CurrentTime) - usToSleep)) / 1000;
	//		}
	//	}
	//	else { // Host CPU is slower or equal to i8080
	//		LastThrottle = CurrentTime;
	//		i8080->m_Cpu->clockCount = 0;
	//	}
	//
	//	Disp->Update(CurrentTime, i8080->m_Memory + i8080Emulator::STACK_START, i8080);
	//
	//	//if (/*!i8080->m_Cpu->printDebug && */CurrentTime - LastDraw > 1'000'000 / 120 /*|| LastDraw > CurrentTime*/) { // 120 Hz - Manage Screen (Half screen in a cycle, then end screen in another)
	//	//	LastDraw = CurrentTime;
	//	//
	//	//	if (DrawFull) {
	//	//		Disp->Update(CurrentTime, i8080->m_Memory + i8080Emulator::STACK_START, i8080);
	//	//		i8080->Interrupt(1);
	//	//	}
	//	//	else
	//	//		i8080->Interrupt(0);
	//	//
	//	//	DrawFull = !DrawFull;
	//	//}
	//
	//	keyboard->Update(CurrentTime, i8080);
	//
	//	//if (CurrentTime - LastInput > 1'000'000 / 30 /*|| LastInput > CurrentTime*/) { // 30 Hz - Manage Events
	//	//	LastInput = CurrentTime;
	//	//	while (SDL_PollEvent(&ev)) {
	//	//		if (ev.type == SDL_QUIT) {
	//	//			i8080->m_Cpu->halt = true;
	//	//		}
	//	//	}
	//	//
	//	//	//for more info on what bit of which port does what see:
	//	//	//https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#inputs
	//	//
	//	//	// Cleanup before input
	//	//	i8080->m_Cpu->inPort[0] &= 0b1000'1111;
	//	//	i8080->m_Cpu->inPort[1] &= 0b1000'1000;
	//	//	i8080->m_Cpu->inPort[2] &= 0b1000'1011;
	//	//
	//	//	if (Keyboard[SDL_SCANCODE_SPACE]) { // Fire
	//	//		i8080->m_Cpu->inPort[0] |= 1 << 4;
	//	//		i8080->m_Cpu->inPort[1] |= 1 << 4;
	//	//		i8080->m_Cpu->inPort[2] |= 1 << 4; // P2
	//	//	}
	//	//
	//	//	if (Keyboard[SDL_SCANCODE_A] || Keyboard[SDL_SCANCODE_LEFT]) { // Left
	//	//		i8080->m_Cpu->inPort[0] |= 1 << 5;
	//	//		i8080->m_Cpu->inPort[1] |= 1 << 5;
	//	//		i8080->m_Cpu->inPort[2] |= 1 << 5; // P2
	//	//	}
	//	//
	//	//	if (Keyboard[SDL_SCANCODE_D] || Keyboard[SDL_SCANCODE_RIGHT]) { // Right
	//	//		i8080->m_Cpu->inPort[0] |= 1 << 6;
	//	//		i8080->m_Cpu->inPort[1] |= 1 << 6;
	//	//		i8080->m_Cpu->inPort[2] |= 1 << 6; // P2
	//	//	}
	//	//
	//	//	if (Keyboard[SDL_SCANCODE_RETURN]) // Credit
	//	//		i8080->m_Cpu->inPort[1] |= 1 << 0;
	//	//
	//	//	if (Keyboard[SDL_SCANCODE_1]) // 1P Start
	//	//		i8080->m_Cpu->inPort[1] |= 1 << 2;
	//	//
	//	//	if (Keyboard[SDL_SCANCODE_2]) // 2P Start
	//	//		i8080->m_Cpu->inPort[1] |= 1 << 1;
	//	//
	//	//	if (Keyboard[SDL_SCANCODE_DELETE]) // Tilt
	//	//		i8080->m_Cpu->inPort[2] |= 1 << 2;
	//	//}
	//
	//	//https://computerarcheology.com/Arcade/SpaceInvaders/Hardware.html#output
	//		////audio
	//		//if ((CurrentTime - LastSound > 1000000 / 60 || LastSound > CurrentTime)) { // 30 Hz Audio
	//		//}
	//		//
	//
	//		//if (CurrentTime - LastDebug > 315000)
	//		//{
	//		//	LastDebug = CurrentTime;
	//		//	std::cout << "PC:  " << i8080->m_Cpu->pc;
	//		//}
	//}
	//
	//	i8080->CycleCpu();
	//}
	//
	//
	//delete Disp;
	//delete keyboard;

	delete i8080;


	std::cout << "\n\n";

	SDL_Quit();

    return 0;
}
