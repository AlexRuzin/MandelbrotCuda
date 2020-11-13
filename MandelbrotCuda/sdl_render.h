#pragma once

#include <SDL2/SDL.h>

#include <stdint.h>
#include <assert.h>

#include "main.h"

namespace render
{
	class sdlBase {
	private:
		const uint32_t windowHeight, windowLength;
		const std::string windowTitle;

		SDL_Window *window;
		SDL_Renderer *renderer;

	public:
		error_t init_window(void)
		{
			window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				windowLength, windowHeight, 0);
			if (window == nullptr) {
				return -1;
			}

			renderer = SDL_CreateRenderer(window, -1, 0);
			if (renderer == nullptr) {
				return -1;
			}

			return 0;
		}

	public:
		sdlBase(uint32_t height, uint32_t length, std::string windowTitle) :
			windowHeight(height), windowLength(length), windowTitle(windowTitle),
			window(nullptr), renderer(nullptr)
		{
			assert(SDL_Init(SDL_INIT_VIDEO) == 0);
		}

		~sdlBase()
		{
			if (renderer != nullptr) {
				SDL_DestroyRenderer(renderer);
			}
			if (window != nullptr) {
				SDL_DestroyWindow(window);
			}

			SDL_Quit();
		}
	};
}