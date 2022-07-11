#pragma once
#pragma warning(push)
#pragma warning(disable: 26812) //disable warnings
#include "SDL.h"
#include "SDL_surface.h"
#pragma warning(pop)

class Display
{
public:
	Display(const char* Title, uint16_t _Width, uint16_t _Height, uint16_t _PixelSize);
	~Display();
	void Update(uint8_t* VRAMMap);
private:
	uint16_t m_PixelSize;
	uint16_t m_Width;
	uint16_t m_Height;
	uint16_t m_Pixels[224 * 256];
	SDL_Window* m_pMainWindow;
	SDL_Renderer* m_pMainRenderer;
	SDL_Texture* m_pMainTexture;
};

