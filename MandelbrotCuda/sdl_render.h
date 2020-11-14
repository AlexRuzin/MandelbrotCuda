#pragma once

#include <SDL2/SDL.h>

#include <stdint.h>
#include <assert.h>
#include <vector>
#include <mutex>
#include <thread>
#include <assert.h>

#include "main.h"
#include "frame.h"

/*
 * Configuration for SDL2 rendering engine
 */
#define RENDER_ENABLE_FPS_CAP 
#if defined(RENDER_ENABLE_FPS_CAP) 
#define RENDER_FPS_CAP						60
#define RENDER_SCREEN_TICKS_PER_FRAME		1000 / RENDER_FPS_CAP
#endif //RENDER_ENABLE_FPS_CAP

#define COLOR_WHITE frame::rgbPixel{ 255, 255, 255 }

namespace render
{
	class sdlTimer;
	class sdlBase;

	//https://lazyfoo.net/tutorials/SDL/23_advanced_timers/index.php
	class sdlTimer {
	private:
		uint32_t mStartTicks, mPausedTicks;
		bool mPaused, mStarted;

	public:
		sdlTimer(void) :
			mStartTicks(0), mPausedTicks(0),
			mPaused(false), mStarted(false)
		{

		}

		~sdlTimer(void)
		{

		}

		void start(void)
		{
			mStarted = true;
			mPaused = false;

			mStartTicks = SDL_GetTicks();
			mPausedTicks = 0;
		}

		void pause(void)
		{
			if (mStarted && !mPaused)
			{
				mPaused = true;

				mPausedTicks = SDL_GetTicks() - mStartTicks;
				mStartTicks = 0;
			}
		}

		void unpause(void)
		{
			if (mStarted && mPaused)
			{
				mPaused = false;
				mStartTicks = SDL_GetTicks() - mPausedTicks;
				mPausedTicks = 0;
			}
		}

		uint32_t getTicks(void) const
		{
			Uint32 time = 0;

			if (mStarted)
			{
				if (mPaused)
				{
					time = mPausedTicks;
				}
				else
				{
					time = SDL_GetTicks() - mStartTicks;
				}
			}

			return time;
		}

		bool isStarted(void) const { return mStarted; }
		bool isPaused(void) const { return mPaused; }
	};

	class sdlBase {
	private:
		const uint32_t windowHeight, windowWidth;
		const std::string windowTitle;

		SDL_Window *window;
		SDL_Renderer *renderer;

		// Frame buffer
		std::vector<frame::rgbPixel> frameBuffer;
		std::mutex frameBufferLock;

		// Render loop flag
		bool doRender;
		std::thread *renderThread;

		// Frame counter
	private:
		sdlTimer fpsTimer;
		sdlTimer capTimer;
		uint64_t frameCount;

	private:
		static friend void render_loop(sdlBase* b)
		{
			assert(b->window != nullptr && b->renderer != nullptr);

			SDL_Event e;
			SDL_Color textColor = { 0, 0, 0, 255 };

			b->fpsTimer.start();

			while (b->doRender) {
				
			}

			return;
		}

	public:
		error_t init_window(void)
		{
			window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				windowWidth, windowHeight, 0);
			if (window == nullptr) {
				return -1;
			}

			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == nullptr) {
				return -1;
			}

			return 0;
		}

		error_t create_render_loop(void)
		{
			assert(!doRender && renderThread == nullptr);

			doRender = true;
			this->renderThread = new std::thread(this);

			return 0;
		}

		void kill_render_loop(void)
		{
			doRender = false;
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
			window(nullptr), renderer(nullptr),
			doRender(false), renderThread(nullptr), frameCount(0)
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