#pragma once
#pragma warning(push)
#pragma warning(disable: 26812) //disable warnings
#include "SDL.h"
#include "SDL_surface.h"
#pragma warning(pop)

class i8080Emulator;

class Display
{
public:
	Display(const char* title, uint16_t width, uint16_t height, uint16_t pixelSize);
	~Display();

	void Update(uint64_t currTime, uint8_t* VRAM, i8080Emulator* i8080);

	enum ScreenHalfs { FirstHalf = 0, SecondHalf = 1 };

private:
	void Draw(uint8_t* VRAM) const;

	uint16_t m_Width;
	uint16_t m_Height;
	uint16_t* m_Pixels;
	SDL_Window* m_pMainWindow;
	SDL_Renderer* m_pMainRenderer;
	SDL_Texture* m_pMainTexture;

	uint64_t m_LastDraw;
	bool m_FirstHalf;

	static constexpr uint64_t draw_frequency{ 1'000'000 / 144 }; //144 Hz (Half screen in a cycle, then end screen in another)

	static constexpr uint16_t black = 0xf000;
	static constexpr uint16_t white = 0xffff;
	static constexpr uint16_t green = 0xf0f0;
	static constexpr uint16_t red	= 0xff00;
};

