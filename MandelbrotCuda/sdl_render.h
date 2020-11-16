#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>
#include <stdint.h>
#include <assert.h>
#include <vector>
#include <mutex>
#include <thread>
#include <assert.h>

#include "main.h"
#include "frame.h"
#include "debug.h"

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

#if defined(RENDER_ENABLE_FPS_CAP)
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
			if (mStarted || mPaused) {
				this->pause();
			}
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

		void reset(void)
		{
			mStarted = mPaused = false;
			mStarted = mPausedTicks = 0;
		}

		bool isStarted(void) const { return mStarted; }
		bool isPaused(void) const { return mPaused; }
	};
#endif //RENDER_ENABLE_FPS_CAP

	class LTexture {
	private:
		SDL_Texture *mTexture;
		SDL_Renderer *mainRenderer;
		TTF_Font *font;

		int mWidth;
		int mHeight;

	public:
		LTexture(__inout SDL_Renderer *renderEngine) :
			mTexture(nullptr), mWidth(0), mHeight(0),
			mainRenderer(renderEngine)
		{

		}

		~LTexture()
		{
			free();
		}

		error_t loadFromFile(std::string path)
		{
			assert(mainRenderer != nullptr);

			free();

			SDL_Texture *newTexture = nullptr;
			SDL_Surface *loadedSurface = IMG_Load(path.c_str());
			if (loadedSurface == nullptr) {
				return -1;
			}

			SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xff, 0xff));
			newTexture = SDL_CreateTextureFromSurface(mainRenderer, loadedSurface);
			if (newTexture == nullptr) {
				return -1;
			}

			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;

			SDL_FreeSurface(loadedSurface);

			mTexture = newTexture;
			return 0;
		}

		error_t LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor)
		{
			free();

			SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
			if (textSurface == nullptr) {
				return -1;
			}

			mTexture = SDL_CreateTextureFromSurface(mainRenderer, textSurface);
			if (mTexture == NULL)
			{
				return -1;
			}

			mWidth = textSurface->w;
			mHeight = textSurface->h;

			SDL_FreeSurface(textSurface);

			return 0;
		}

		void free()
		{
			if (mTexture != nullptr) {
				SDL_DestroyTexture(mTexture);
				mTexture = nullptr;
				mHeight = mWidth = 0;
			}
		}

		void render(int x, int y);

		int getWidth();
		int getHeight();
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
#if defined(RENDER_ENABLE_FPS_CAP)
		sdlTimer fpsTimer;
		sdlTimer capTimer;
		uint64_t frameCount;
#endif //RENDER_ENABLE_FPS_CAP

	private:
		static friend void render_loop(sdlBase* b)
		{
			assert(b->window != nullptr && b->renderer != nullptr);

			SDL_Event e;
			SDL_Color textColor = { 0, 0, 0, 255 };

#if defined(RENDER_ENABLE_FPS_CAP)
			b->fpsTimer.start();
			uint32_t countedFrames = 0;
			std::stringstream timeText;
			LTexture texture(b->renderer);
#endif //RENDER_ENABLE_FPS_CAP

			while (b->doRender) {
				float avgFPS = countedFrames / (b->fpsTimer.getTicks() / 1000.f);
				if (avgFPS > 2000000)
				{
					avgFPS = 0;
				}

				timeText.str("");
				timeText << "Average Frames Per Second (With Cap) " << avgFPS;

				if (!texture.loadFromRenderedText(timeText.str().c_str(), textColor))
				{
					DERROR("render_loop: Failed to load rendered text");
					break;
				}
			}

#if defined(RENDER_ENABLE_FPS_CAP)
			b->fpsTimer.pause();
#endif

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
			this->renderThread = new std::thread(render_loop, this);

			DINFO("Created rendering loop");

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