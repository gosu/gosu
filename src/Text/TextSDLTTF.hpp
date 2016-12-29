#pragma once

// Used for custom TTF files
// Adapted from customFont class by José Tomás Tocino García (TheOm3ga)

#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Utility.hpp>
#include <SDL_ttf.h>

namespace Gosu
{
	class SDLTTFRenderer
	{
		SDLTTFRenderer(const SDLTTFRenderer&);
		SDLTTFRenderer& operator=(const SDLTTFRenderer&);

		TTF_Font* font;

		class SDLSurface
		{
			SDLSurface(const SDLSurface&);
			SDLSurface& operator=(const SDLSurface&);

			SDL_Surface* surface;

		public:
			SDLSurface(TTF_Font* font, const std::wstring& text, Gosu::Color c)
			{
				// This is intentionally re-ordered to BGR. This way, the surface pixels do not
				// have to be converted from RGB to BGR later in the process.
				SDL_Color color = { c.blue(), c.green(), c.red() };
				surface = TTF_RenderUTF8_Blended(font, Gosu::wstringToUTF8(text).c_str(), color);
				if (!surface)
					throw std::runtime_error("Could not render text " + Gosu::wstringToUTF8(text));
			}

			~SDLSurface()
			{
				SDL_FreeSurface(surface);
			}

			unsigned height() const
			{
				return surface->h;
			}

			unsigned width() const
			{
				return surface->w;
			}

			const void* data() const
			{
				return surface->pixels;
			}
		};

	public:
		SDLTTFRenderer(const std::wstring& fontName, unsigned fontHeight);

		~SDLTTFRenderer();

		unsigned textWidth(const std::wstring& text);

		void drawText(Bitmap& bmp, const std::wstring& text, int x, int y, Gosu::Color c);
	};
}