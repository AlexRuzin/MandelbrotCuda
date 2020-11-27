#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <mutex>
#include <assert.h>

#include "types.h"

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

		void start(void);
		void pause(void);
		void unpause(void);
		uint32_t getTicks(void) const;
		void reset(void);
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
		LTexture(__inout SDL_Renderer* renderEngine);		

		~LTexture()
		{
			free();

			if (font != nullptr) {
				TTF_CloseFont(font);
				font = nullptr;
			}
		}

	private:
		error_t setFontType(const std::string& name, int size);

	public:
		error_t loadFromFile(std::string path);

		error_t loadFromRenderedText(std::string textureText, SDL_Color textColor);

		void free(void);

		void render(int x, int y);

		int getWidth() const { return mWidth; }
		int getHeight() const { return mHeight; }
	};


	class sdlBase {
	private:
		const size_t windowHeight, windowWidth;
		const std::string windowTitle;

		SDL_Window *window;
		SDL_Renderer *renderer;

		// Frame buffer sync 
		std::mutex *frameBufferLock;
		
		// Frame buffer (SDL2 texture array)
		//   Each pixel contains r, g, b, alpha, 8 bits each
		size_t framePixelLength, framePixelHeight, frameTotalPixels;
		rgbaPixel *frameBuffer;
		bool refreshBuffer;

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
		error_t render_loop(sdlBase* b);

	public:
		// Function for SDL2 raw frame buffer, uses format:
		//  4 bytes per pixel: r, g, b, alpha
		void write_static_frame(__in rgbaPixel* frame, size_t length, size_t height);

		error_t enter_render_loop(void)
		{
			this->doRender = true;
			return render_loop(this);
		}

		error_t init_window(void);

		void kill_render_loop(void)
		{
			doRender = false;
		}

	public:
		sdlBase(size_t height, size_t width, std::string windowTitle) :
			windowHeight(height), windowWidth(width), windowTitle(windowTitle),
			window(nullptr), renderer(nullptr),
			doRender(false), renderThread(nullptr), 		
			frameBuffer(nullptr), framePixelHeight(0), framePixelLength(0), refreshBuffer(false),
			frameBufferLock(new std::mutex()),
#if defined(RENDER_ENABLE_FPS_CAP)
			frameCount(0)
#endif //RENDER_ENABLE_FPS_CAP
		{
			assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0);
			assert(TTF_Init() == 0);
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