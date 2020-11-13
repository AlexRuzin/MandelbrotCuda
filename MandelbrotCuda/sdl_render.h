#pragma once

#include <SDL2/SDL.h>

#include <stdint.h>
#include <assert.h>
#include <vector>
#include <mutex>

#include "main.h"
#include "frame.h"

#define COLOR_WHITE frame::rgbPixel{ 255, 255, 255 }

namespace render
{
	class sdlBase {
	private:
		const uint32_t windowHeight, windowWidth;
		const std::string windowTitle;

		SDL_Window *window;
		SDL_Renderer *renderer;

		// Frame buffer
		std::vector<frame::rgbPixel> frameBuffer;
		std::mutex frameBufferLock;

	public:
		error_t init_window(void)
		{
			window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				windowWidth, windowHeight, 0);
			if (window == nullptr) {
				return -1;
			}

			renderer = SDL_CreateRenderer(window, -1, 0);
			if (renderer == nullptr) {
				return -1;
			}

			return 0;
		}

		error_t create_render_loop(void)
		{

			return 0;
		}

		// Input operator
		void operator<<(const std::vector<frame::rgbPixel>& d)
		{
			frameBufferLock.lock();
			frameBuffer = d;
			frameBufferLock.unlock();
		}

	public:
		sdlBase(uint32_t height, uint32_t width, std::string windowTitle) :
			windowHeight(height), windowWidth(width), windowTitle(windowTitle),
			window(nullptr), renderer(nullptr)
		{
			assert(SDL_Init(SDL_INIT_VIDEO) == 0);

			// Initialize frame buffer with WHITE color
			frameBuffer.resize(height * width);
			for (uint32_t i = 0; i < (height * width); ++i) {
				frameBuffer.push_back(COLOR_WHITE);
			}
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