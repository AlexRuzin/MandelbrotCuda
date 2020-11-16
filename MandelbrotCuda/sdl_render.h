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

#define FPS_COUNTER_FONT_TYPE		"C:\\Windows\\Fonts\\Arial.ttf"
#define FPS_COUNTER_FONT_SIZE		28

/*
 * Configuration for SDL2 rendering engine
 */
#define RENDER_ENABLE_FPS_CAP 
#if defined(RENDER_ENABLE_FPS_CAP) 
#define RENDER_FPS_CAP						60
#define RENDER_SCREEN_TICKS_PER_FRAME		1000 / RENDER_FPS_CAP
#define RENDER_SHOW_FPS_STRING
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
			mainRenderer(renderEngine), font(nullptr)
		{
			// Load default font type
			if (setFontType(FPS_COUNTER_FONT_TYPE, FPS_COUNTER_FONT_SIZE) != 0) {
				DERROR("Failed to load texture");
			}
		}

		~LTexture()
		{
			free();

			if (font != nullptr) {
				TTF_CloseFont(font);
				font = nullptr;
			}
		}

	private:
		error_t setFontType(const std::string& name, int size)
		{
			if (font != nullptr)
			{
				TTF_CloseFont(font);
				font = nullptr;
			}

			std::ifstream f(name.c_str());
			if (!f.good()) {
				DWARNING("Font file does not exist: " + name);
			}

			font = TTF_OpenFont(name.c_str(), size);
			if (font == nullptr) {
				DERROR("Failed to load TTF: " + name + " SDL ERROR: " + TTF_GetError());
				return -1;
			}

			if (font == nullptr)
			{
				DERROR("Failed to load font! SDL_ttf Error");
			}

			return 0;
		}

	public:
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

		void render(int x, int y)
		{
			SDL_Rect renderQuad = { x, y, mWidth, mHeight };
			SDL_RenderCopy(mainRenderer, mTexture, NULL, &renderQuad);
		}

		int getWidth() const { return mWidth; }
		int getHeight() const { return mHeight; }
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

		// Framer renderer lock mutex
		std::mutex *waitForRendererEnd;

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
			DINFO("Starting SDL2 renderer loop thread");

			SDL_Event sdlEvent = {0};

#if defined(RENDER_ENABLE_FPS_CAP)
			SDL_Color textColor = { 0, 0, 0, 255 };
			b->fpsTimer.start();
			uint32_t countedFrames = 0;
			std::stringstream timeText;
			LTexture texture(b->renderer);
#endif //RENDER_ENABLE_FPS_CAP

			b->waitForRendererEnd->lock();

			/*
			 * Primary rendering loop
			 */
			while (b->doRender) {

#if defined(RENDER_ENABLE_FPS_CAP)
				float avgFPS = countedFrames / (b->fpsTimer.getTicks() / 1000.f);
				if (avgFPS > 2000000)
				{
					avgFPS = 0;
				}

				timeText.str("");
				timeText << "FPS Limit: " << RENDER_FPS_CAP << " Average: " << avgFPS;

				if (texture.loadFromRenderedText(timeText.str().c_str(), textColor))
				{
					DERROR("render_loop: Failed to load rendered text");
					break;
				}
#endif //RENDER_ENABLE_FPS_CAP

				while (SDL_PollEvent(&sdlEvent) != 0) {
					switch (sdlEvent.type) {
						DINFO("Event triggered");
					case SDL_QUIT:
						DINFO("Renderer has received SDL_QUIT signal");
						b->doRender = false;
						break;
					case SDL_KEYDOWN:
						DINFO("Keystroke detected");
						switch (sdlEvent.key.keysym.sym) {
						case SDLK_ESCAPE:
							DINFO("Renderer has received user input quit signal");
							b->doRender = false;
							break;
						}
						break;
					case SDL_WINDOWEVENT:
						switch (sdlEvent.window.event) {
						case SDL_WINDOWEVENT_CLOSE:
							DINFO("Renderer exiting");
							b->doRender = false;
							break;
						}
						break;
					}
				}

				SDL_SetRenderDrawColor(b->renderer, 0xff, 0xff, 0xff, 0xff);
				SDL_RenderClear(b->renderer);

#if defined(RENDER_ENABLE_FPS_CAP)
#if defined(RENDER_SHOW_FPS_STRING)
				//texture.render((b->windowWidth - texture.getWidth()) / 2, (b->windowHeight - texture.getHeight()) / 2);
				texture.render(0, 0); 
				countedFrames++;
#endif //RENDER_SHOW_FPS_STRING
#endif //RENDER_ENABLE_FPS_CAP

				SDL_RenderPresent(b->renderer);

#if defined(RENDER_ENABLE_FPS_CAP)
				uint32_t frameTicks = b->capTimer.getTicks();
				if (frameTicks < RENDER_SCREEN_TICKS_PER_FRAME) {
					SDL_Delay(RENDER_SCREEN_TICKS_PER_FRAME - frameTicks);
				}
#endif //RENDER_ENABLE_FPS_CAP
			}

#if defined(RENDER_ENABLE_FPS_CAP)
			b->fpsTimer.pause();
#endif
			b->waitForRendererEnd->unlock();
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
			debug::sleep(100);

			DINFO("Created rendering loop");

			return 0;
		}

		void wait_for_render_exit(void) 
		{
			waitForRendererEnd->lock();
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
			doRender(false), renderThread(nullptr), 			
#if defined(RENDER_ENABLE_FPS_CAP)
			frameCount(0),
#endif //RENDER_ENABLE_FPS_CAP
			waitForRendererEnd(new std::mutex())
		{
			assert(SDL_Init(SDL_INIT_EVERYTHING) == 0);
			assert(TTF_Init() == 0);

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

//EOF