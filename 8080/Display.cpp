#include "Display.h"

const uint16_t Black = 0xf000;
const uint16_t White = 0xffff;
const uint16_t Green = 0xf0f0;
const uint16_t Red = 0xff00;

Display::Display(const char* title, uint16_t width, uint16_t height, uint16_t pixelSize) {
	m_PixelSize = pixelSize;
	m_Width = width;
	m_Height = height;

	m_pMainWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Width * m_PixelSize, m_Height * m_PixelSize, SDL_WINDOW_SHOWN);
	m_pMainRenderer = SDL_CreateRenderer(m_pMainWindow, -1, SDL_RENDERER_ACCELERATED);
	m_pMainTexture = SDL_CreateTexture(m_pMainRenderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STREAMING, m_Width, m_Height);

	SDL_SetRenderDrawColor(m_pMainRenderer, 0x00, 0x00, 0x00, 0x00);
	SDL_RenderClear(m_pMainRenderer);
}

Display::~Display()
{
	SDL_DestroyWindow(m_pMainWindow);
}

void Display::Update(uint8_t* VRAM) {
	uint16_t ColorToDraw = White;

	for (uint16_t x = 0; x < m_Width; x++) {
		for (uint16_t y = 0; y < m_Height; y += 8) {
			uint8_t VRAMByte = VRAM[x * (m_Height >> 3) + (y >> 3)];

			for (uint8_t bit = 0; bit < 8; bit++) {
				ColorToDraw = Black;

				if (((VRAMByte >> bit) & 1)) {
					if (y <= 73) {
						ColorToDraw = Green;
						if ((x <= 16 || x >= m_Width - 122) && y <= 15) // Special Zone
							ColorToDraw = White;
					}
					else if (y >= m_Height - 64 && y <= m_Height - 33)
						ColorToDraw = Red;
					else
						ColorToDraw = White;
				}

				uint16_t CoordX = x;
				uint16_t CoordY = static_cast<uint16_t>(m_Height - 1 - (y + bit));
				m_Pixels[CoordY * m_Width + CoordX] = ColorToDraw;
			}
		}
	}

	SDL_UpdateTexture(m_pMainTexture, NULL, m_Pixels, 2 * m_Width);
	SDL_RenderCopy(m_pMainRenderer, m_pMainTexture, NULL, NULL);
	SDL_RenderPresent(m_pMainRenderer);
}
