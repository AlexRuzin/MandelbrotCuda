#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <mutex>
#include <assert.h>
#include <chrono>
#include <string>
#include <vector>

#include "main.h"
#include "types.h"

#define FPS_COUNTER_FONT_TYPE		"C:\\Windows\\Fonts\\Arial.ttf"
#define FPS_COUNTER_FONT_SIZE		20

/*
 * Configuration for SDL2 rendering engine
 */
#if !defined(DISABLE_FPS_COUNTERS)
#define RENDER_ENABLE_FPS_CAP 
#endif //DISABLE_FPS_COUNTERS
#if defined(RENDER_ENABLE_FPS_CAP) 
#define RENDER_FPS_CAP						60
#define RENDER_SCREEN_TICKS_PER_FRAME		1000 / RENDER_FPS_CAP
#define RENDER_SHOW_FPS_STRING
#endif //RENDER_ENABLE_FPS_CAP

/*
 * Renders the CUDA time elapsed stats
 */
#if !defined(DISABLE_FPS_COUNTERS)
#define RENDER_CUDA_STATS
#endif //DISABLE_FPS_COUNTERS

/*
 * Displays kernel parameters
 */
#if !defined(DISABLE_FPS_COUNTERS)
#define DISPLAY_KERNEL_PARAMETERS
#endif //DISABLE_FPS_COUNTERS

/*
 * Displays the location of the mouse on screen
 */
#if !defined(DISABLE_FPS_COUNTERS)
#define DISPLAY_MOUSE_LOCATION
#endif //DISABLE_FPS_COUNTERS


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

		// Single line
		error_t loadFromRenderedText(std::string textureText, SDL_Color textColor);

		// Multiline, and takes size (in pixels) of next line (if using \r\n) delimiter
		error_t loadFromRenderedText(std::string textureText, SDL_Color textColor, uint32_t pixLength);

		void free(void);

		void render(int x, int y);

		int getWidth() const { return mWidth; }
		int getHeight() const { return mHeight; }
	};

	typedef struct cudaRenderingStats {
		double frameRenderElapsedms; // milliseconds
		double offsetX, offsetY;
		double scaleA, scaleB;
	} CUDA_RENDERING_STATS, *PCUDA_RENDERING_STATS;

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

		// Counter for the CUDA rendering
		cudaRenderingStats cudaStats;

		// Draw cross hairs
		bool drawCrosshair;
		const uint8_t crosshairColor[3] = CROSSHAIR_COLOR;

		// Mouse cursor position, starting from (0,0) to (inf, inf)
		uint32_t mouseX, mouseY;

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

		void update_cuda_rendering_stats(cudaRenderingStats stats)
		{
			cudaStats = stats;
		}

	public:
		sdlBase(size_t height, size_t width, std::string windowTitle) :
			windowHeight(height), windowWidth(width), windowTitle(windowTitle),
			window(nullptr), renderer(nullptr),
			doRender(false), renderThread(nullptr),
			frameBuffer(nullptr), framePixelHeight(0), framePixelLength(0), refreshBuffer(false),
			frameBufferLock(new std::mutex()),
			cudaStats(cudaRenderingStats{ 56666666555 }),
#if defined(RENDER_ENABLE_FPS_CAP)
			frameCount(0),
#endif //RENDER_ENABLE_FPS_CAP
			drawCrosshair(false)
		{
			assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0);
			assert(TTF_Init() == 0);
		}

		// Sets the controller object as a void * to prevent cyclic header redundancy
		void set_controller_obj(__inout void *ptr) const;

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

	/*
	 * Stats class
	 */

	// Use the optimized version or suffer a memory leak from hell
#define USE_OPTIMIZED_RENDERLINES

#define SCREEN_STATS(x) screenStats.append((std::string)x)
	class renderLines {
	private:
		const uint32_t verticalOffset;

		SDL_Renderer *renderer;
		SDL_Color color;

		std::vector<std::string> *lineArray;

#if !defined(USE_OPTIMIZED_RENDERLINES)
		std::vector<LTexture *> *lineTextures;
#endif //USE_OPTIMIZED_RENDERLINES

	public:
		renderLines(std::string initString, __inout SDL_Renderer *renderEngine, SDL_Color color) :
			verticalOffset(1000),
			color(color),
			renderer(renderEngine),
#if !defined(USE_OPTIMIZED_RENDERLINES)
			lineTextures(new(std::vector<LTexture *>)),
#endif //USE_OPTIMIZED_RENDERLINES
			lineArray(new(std::vector<std::string>))
		{
			this->append(initString);
		}

		void append(std::string input)
		{
			lineArray->push_back(input);

#if !defined(USE_OPTIMIZED_RENDERLINES)
			lineTextures->push_back(new LTexture(renderer));
			lineTextures->back()->loadFromRenderedText(lineArray->back().c_str(), color);
#endif //USE_OPTIMIZED_RENDERLINES
		}

		void append(std::string input, SDL_Color colorOverride)
		{
			lineArray->push_back(input);

#if !defined(USE_OPTIMIZED_RENDERLINES)
			lineTextures->push_back(new LTexture(renderer));
			lineTextures->back()->loadFromRenderedText(lineArray->back().c_str(), colorOverride);
#else 

#endif //USE_OPTIMIZED_RENDERLINES
		}

		void change_color(SDL_Color newColor) { color = newColor; }

		/*
		std::ostream &operator<<(std::ostream &out, const std::string &towrite) {
		{
			this->lineArray.push_back(towrite);
			lineTextures.push_back(LTexture(renderer));
			lineTextures.back().loadFromRenderedText(lineArray.back().c_str(), color);
			return out;
		}
		*/

		void render(void)
		{
#if !defined(USE_OPTIMIZED_RENDERLINES)
			uint32_t offset = 0;
			for (std::vector<LTexture *>::iterator i = lineTextures->begin(); i != lineTextures->end(); ++i) {
				(*i)->render(0, offset);
				offset += verticalOffset;
			}
#else //USE_OPTIMIZED_RENDERLINES
			std::string outputStr;
			for (std::vector<std::string>::const_iterator i = lineArray->begin(); i != lineArray->end(); ++i) {
				outputStr += *i + "\n";
			}
			
			LTexture strTexture(renderer);
			if (verticalOffset != 0) {
				strTexture.loadFromRenderedText(outputStr.c_str(), color, verticalOffset);
			}
			else {
				strTexture.loadFromRenderedText(outputStr.c_str(), color);
			}
			strTexture.render(0, 0);
#endif //USE_OPTIMIZED_RENDERLINES
		}

		void clear(void) 
		{
			delete(lineArray);
			lineArray = new std::vector<std::string>();

#if !defined(USE_OPTIMIZED_RENDERLINES)
			delete(lineTextures);
#endif //USE_OPTIMIZED_RENDERLINES
		}
	};
}

//EOF