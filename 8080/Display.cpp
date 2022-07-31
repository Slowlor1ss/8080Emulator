#include "Display.h"
#include "i8080Emulator.h"

Display::Display(const char* title, uint16_t width, uint16_t height, uint16_t pixelSize)
	: m_LastDraw(0)
	, m_FirstHalf(true)
{

	m_Width = width;
	m_Height = height;

	m_Pixels = new uint16_t[m_Width * m_Height];

	m_pMainWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Width * pixelSize, m_Height * pixelSize, SDL_WINDOW_SHOWN);
	m_pMainRenderer = SDL_CreateRenderer(m_pMainWindow, -1, SDL_RENDERER_ACCELERATED);
	m_pMainTexture = SDL_CreateTexture(m_pMainRenderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STREAMING, m_Width, m_Height);

	SDL_SetRenderDrawColor(m_pMainRenderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(m_pMainRenderer);
}

Display::~Display() {
	SDL_DestroyWindow(m_pMainWindow);

	delete[] m_Pixels;
	m_Pixels = nullptr;
}

void Display::Draw(uint8_t* VRAM) const {

	for (uint16_t x = 0; x < m_Width; x++) {
		for (uint16_t y = 0; y < m_Height; y += 8) { //the pixels are saved in 8 bit integers

			uint8_t VRAMByte = VRAM[x * (m_Height >> 3) + (y >> 3)];

			for (uint8_t bit = 0; bit < 8; bit++) {

				uint16_t colorToDraw = black;

				//if the bit is 1 theres a pixel we need to color
				if (((VRAMByte >> bit) & 1)) {

					//(just a small detail =D ) these are some checks to give certain pixels on the screen a color
					//this is to recreate the display color overlays that were often used in old arcade machines
					//https://youtu.be/QyjyWUrHsFc?t=95
					if (y <= 60) {
						colorToDraw = green;

						if ((x <= 16 || x >= m_Width - 122) && y <= 15) //white text at the bottom for the credits and lives
							colorToDraw = white;
					}
					else if (y >= m_Height - 64 && y <= m_Height - 33)
						colorToDraw = red;
					else
						colorToDraw = white;
				}

				const uint16_t coordX = x;
				const uint16_t coordY = static_cast<uint16_t>(m_Height - 1 - (y + bit));
				m_Pixels[coordY * m_Width + coordX] = colorToDraw;
			}
		}
	}

	SDL_UpdateTexture(m_pMainTexture, nullptr, m_Pixels, 2 * m_Width);
	SDL_RenderCopy(m_pMainRenderer, m_pMainTexture, nullptr, nullptr);
	SDL_RenderPresent(m_pMainRenderer);
}

void Display::Update(uint64_t currTime, uint8_t* VRAM, i8080Emulator* i8080) {

	if (currTime - m_LastDraw > draw_frequency) {
		m_LastDraw = currTime;

		if (m_FirstHalf) {
			Draw(VRAM);
			i8080->Interrupt(FirstHalf);
		}
		else
			i8080->Interrupt(SecondHalf);

		m_FirstHalf = !m_FirstHalf;
	}

}
