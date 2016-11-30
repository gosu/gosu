// Used for custom TTF files
// Adapted from customFont class by José Tomás Tocino García (TheOm3ga)

#include "TextSDLTTF.hpp"

namespace Gosu
{
	SDLTTFRenderer::SDLTTFRenderer(const std::wstring& fontName, unsigned fontHeight)
	{
		static int initResult = TTF_Init();
		if (initResult < 0)
			throw std::runtime_error("Could not initialize SDL_TTF");

		// Try to open the font at the given path
		font = TTF_OpenFont(Gosu::wstringToUTF8(fontName).c_str(), fontHeight);
		if (!font)
			throw std::runtime_error("Could not open TTF file " + Gosu::wstringToUTF8(fontName));

		// Re-open with scaled height so that ascenders/descenders fit
		int tooLargeHeight = TTF_FontHeight(font);
		int realHeight = fontHeight * fontHeight / tooLargeHeight;
		TTF_CloseFont(font);
		font = TTF_OpenFont(Gosu::wstringToUTF8(fontName).c_str(), realHeight);
		if (!font)
			throw std::runtime_error("Could not open TTF file " + Gosu::wstringToUTF8(fontName));
	}

	SDLTTFRenderer::~SDLTTFRenderer()
	{
		TTF_CloseFont(font);
	}

	unsigned SDLTTFRenderer::textWidth(const std::wstring& text) {
		return SDLSurface(font, text, 0xffffff).width();
	}

	void SDLTTFRenderer::drawText(Bitmap& bmp, const std::wstring& text, int x, int y, Gosu::Color c) {
		SDLSurface surf(font, text, c);
		Gosu::Bitmap temp;
		temp.resize(surf.width(), surf.height());
		std::memcpy(temp.data(), surf.data(), temp.width() * temp.height() * 4);
		bmp.insert(temp, x, y);
	}
}