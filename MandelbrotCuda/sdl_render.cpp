
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <iomanip>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>
#include <stdint.h>
#include <assert.h>
#include <vector>
#include <mutex>
#include <thread>
#include <assert.h>

#include "main.h"
#include "sdl_render.h"
#include "controller.h"
#include "debug.h"

using namespace render;

static controller::loopTimer *controllerPtr = nullptr;

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return out.str();
}

/*
 * sdlTimer 
 */
#if defined(RENDER_ENABLE_FPS_CAP)
void sdlTimer::start(void)
{
	mStarted = true;
	mPaused = false;

	mStartTicks = SDL_GetTicks();
	mPausedTicks = 0;
}

void sdlTimer::pause(void)
{
	if (mStarted && !mPaused)
	{
		mPaused = true;

		mPausedTicks = SDL_GetTicks() - mStartTicks;
		mStartTicks = 0;
	}
}

void sdlTimer::unpause(void)
{
	if (mStarted && mPaused)
	{
		mPaused = false;
		mStartTicks = SDL_GetTicks() - mPausedTicks;
		mPausedTicks = 0;
	}
}

uint32_t sdlTimer::getTicks(void) const
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

void sdlTimer::reset(void)
{
	mStarted = mPaused = false;
	mStarted = mPausedTicks = 0;
}
#endif //RENDER_ENABLE_FPS_CAP

/*
 * LTexture
 */
LTexture::LTexture(__inout SDL_Renderer* renderEngine)
{
	mTexture = nullptr;
	font = nullptr;
	mWidth = mHeight = 0;
	mainRenderer = renderEngine;

	// Load default font type
	if (setFontType(FPS_COUNTER_FONT_TYPE, FPS_COUNTER_FONT_SIZE) != 0) {
		DERROR("Failed to load texture");
		std::exit(0);
	}
}

error_t LTexture::setFontType(const std::string& name, int size)
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

error_t LTexture::loadFromFile(std::string path)
{
	assert(mainRenderer != nullptr);

	free();

	SDL_Texture* newTexture = nullptr;
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
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

error_t LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor, uint32_t pixLength)
{
	free();

	SDL_Surface *textSurface = TTF_RenderText_Blended_Wrapped(font, textureText.c_str(), textColor, pixLength);
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

void LTexture::free(void)
{
	if (mTexture != nullptr) {
		SDL_DestroyTexture(mTexture);
		mTexture = nullptr;
		mHeight = mWidth = 0;
	}
}

void LTexture::render(int x, int y)
{
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };
	SDL_RenderCopy(mainRenderer, mTexture, NULL, &renderQuad);
}


// SDLBase
error_t sdlBase::render_loop(sdlBase* b)
{
	assert(b->window != nullptr && b->renderer != nullptr);
	DINFO("Starting SDL2 renderer loop thread");

	SDL_Color textColor = { 255, 255, 255, 255 };

#if defined(RENDER_ENABLE_FPS_CAP)
	b->fpsTimer.start();
	uint32_t countedFrames = 0;
#endif //RENDER_ENABLE_FPS_CAP

	// Primary frame texture
	SDL_Texture* frameTexture = SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		(uint32_t)framePixelLength, (uint32_t)framePixelHeight);

#if !defined(DISABLE_FPS_COUNTERS)
	render::renderLines screenStats("Mandelbrot Fractal v0.2", b->renderer, textColor);
#endif //DISABLE_FPS_COUNTERS

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
		SCREEN_STATS("FPS Limit: " + std::to_string(RENDER_FPS_CAP) + " Average: " + std::to_string(avgFPS));
#endif //RENDER_ENABLE_FPS_CAP

#if defined(RENDER_CUDA_STATS)
		SCREEN_STATS("Last CUDA rendering time: " + std::to_string(b->cudaStats.frameRenderElapsedms) + " ms");
#endif //RENDER_CUDA_STATS

#if defined(DISPLAY_KERNEL_PARAMETERS)
		SCREEN_STATS("SCALE Alpha: " + to_string_with_precision(b->cudaStats.scaleA, 32));
		SCREEN_STATS("SCALE Delta: " +
			to_string_with_precision(b->cudaStats.scaleA / ((double)b->framePixelLength / b->cudaStats.scaleB), 32));
		SCREEN_STATS("(fractal offset) C.x: " + to_string_with_precision(b->cudaStats.offsetX, 32));
		SCREEN_STATS("(fractal offset) C.y: " + to_string_with_precision(b->cudaStats.offsetY, 32));
#endif //DISPLAY_KERNEL_PARAMETERS

		SDL_GetMouseState((int *)&b->mouseX, (int *)&b->mouseY);
#if defined(DISPLAY_MOUSE_LOCATION)
		SCREEN_STATS("MouseXY: (" + std::to_string(b->mouseX) + "," + std::to_string(b->mouseY) + ") => " + 
			"(" + to_string_with_precision((double)b->mouseX / (double)RENDER_WINDOW_LENGTH) + "," + 
			to_string_with_precision((double)b->mouseY / (double)RENDER_WINDOW_HEIGHT) + ") => (" + 
			to_string_with_precision((double)controllerPtr->inMouseX * (2.0 / (double)RENDER_WINDOW_LENGTH) - 1.0, 32) + "," +
			to_string_with_precision((double)controllerPtr->inMouseY * (2.0 / (double)RENDER_WINDOW_HEIGHT) - 1.0, 32) + ")");
#endif //DISPLAY_MOUSE_LOCATION

		SDL_Event sdlEvent;
		error_t err;
		while (SDL_PollEvent(&sdlEvent) != 0) {
			switch (sdlEvent.type) {
			case SDL_QUIT:
				DINFO("Renderer has received SDL_QUIT signal");
				b->doRender = false;
				break;
			case SDL_KEYDOWN:
				DINFO("Keystroke detected");
				switch (sdlEvent.key.keysym.sym) {

				/*
				 * Controls movement of the camera zoom, either forward, reverse, or pause
				 */
				case SDLK_1: // Run/resume
					controllerPtr->set_user_io_state(controller::SET_ZOOM_RESUME);
					break;
				case SDLK_2:
					controllerPtr->set_user_io_state(controller::SET_ZOOM_PAUSE);
					break;
				case SDLK_3:
					controllerPtr->set_user_io_state(controller::SET_ZOOM_REVERSE);
					break;

				// Sets the crosshair
				case SDLK_c:
					b->drawCrosshair = !b->drawCrosshair;
					break;

				// Dumps the current controller parameters into JSON
				case SDLK_d:
					err = controllerPtr->dump_parameters_json();
					if (err != 0) {
						DERROR("Failed to dump parameters to JSON: " + std::to_string(err));
						std::exit(err);
					}
					break;

				// Teriminate application
				case SDLK_ESCAPE:
					DINFO("Renderer has received user input quit signal");
					b->doRender = false;
					std::exit(0);
					break;
				}
				break;

			/* 
			 * Mouse event handling
			 */
			case SDL_MOUSEBUTTONDOWN:
				if (sdlEvent.button.button == SDL_BUTTON_LEFT) {
					controllerPtr->set_mouse_button_offset(this->mouseX, this->mouseY);
				}
				break;

			case SDL_WINDOWEVENT:
				switch (sdlEvent.window.event) {
				case SDL_WINDOWEVENT_CLOSE:
					DINFO("Renderer exiting");
					b->doRender = false;
					break;
				case SDL_WINDOWEVENT_MINIMIZED:
					DINFO("SDL_WINDOWEVENT_MAXIMIZED");
					break;
				case SDL_WINDOWEVENT_MAXIMIZED:
					DINFO("SDL_WINDOWEVENT_MAXIMIZED");
					break;
				case SDL_WINDOWEVENT_ENTER:
					DINFO("SDL_WINDOWEVENT_ENTER");
					break;
				default:
					DINFO("Unknown SDL_WINDOWEVENT");
				}

				break;
			default:
				//DWARNING("Unknown EVENT from SDL2 poll function");
				break;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

		// Draw raw frame buffer
		frameBufferLock->lock();
		if (refreshBuffer) {
			unsigned char* lockedPixels = nullptr;
			int pitch = 0;
			SDL_LockTexture(frameTexture,
				NULL,
				reinterpret_cast<void**>(&lockedPixels),
				&pitch);
			std::memcpy(lockedPixels, frameBuffer, frameTotalPixels * sizeof(rgbaPixel));
			SDL_UnlockTexture(frameTexture);
			
			refreshBuffer = false;
		} 
		
		/*
		else {
			SDL_UpdateTexture
			(
				frameTexture,
				NULL,
				frameBuffer,
				frameTotalPixels * sizeof(rgbaPixel)
			);
		}
		*/
		
		frameBufferLock->unlock();

		// Copy frame image into renderer
		SDL_RenderCopy(renderer, frameTexture, NULL, NULL);

		// Draw cross hair if configured
		if (b->drawCrosshair) {
			SDL_SetRenderDrawColor(b->renderer,
				b->crosshairColor[0], //r
				b->crosshairColor[1], //g
				b->crosshairColor[2], //b
				SDL_ALPHA_OPAQUE);
			SDL_Point points[4] = {
				{ 0, RENDER_WINDOW_HEIGHT / 2 },
				{ RENDER_WINDOW_LENGTH, RENDER_WINDOW_HEIGHT / 2 },
				{ RENDER_WINDOW_LENGTH / 2, 0 },
				{ RENDER_WINDOW_LENGTH / 2, RENDER_WINDOW_HEIGHT }
			};
			SDL_RenderDrawLines(b->renderer, points, 4);
		}

		// Render on-screen stats
#if !defined(DISABLE_FPS_COUNTERS)
		screenStats.render();
#endif //DISABLE_FPS_COUNTERS

#if defined(RENDER_ENABLE_FPS_CAP)
#if defined(RENDER_SHOW_FPS_STRING)
		//texture.render((b->windowWidth - texture.getWidth()) / 2, (b->windowHeight - texture.getHeight()) / 2);
		countedFrames++;
#endif //RENDER_SHOW_FPS_STRING
#endif //RENDER_ENABLE_FPS_CAP

		SDL_RenderPresent(b->renderer);

#if !defined(DISABLE_FPS_COUNTERS)
		screenStats.clear();
#endif //DISABLE_FPS_COUNTERS

#if defined(RENDER_ENABLE_FPS_CAP)
		uint32_t frameTicks = b->capTimer.getTicks();
		if (frameTicks < RENDER_SCREEN_TICKS_PER_FRAME) {
			SDL_Delay(RENDER_SCREEN_TICKS_PER_FRAME - frameTicks);
		}
#else
		SDL_Delay(10);
#endif //RENDER_ENABLE_FPS_CAP
	}

#if defined(RENDER_ENABLE_FPS_CAP)
	b->fpsTimer.pause();
#endif
	return 0;
}

void sdlBase::write_static_frame(__in rgbaPixel* frame, size_t length, size_t height)
{
	frameBufferLock->lock();

	if (this->frameBuffer != nullptr) {
		std::free(frameBuffer);
	}

	framePixelLength = length;
	framePixelHeight = height;
	frameTotalPixels = framePixelHeight * framePixelLength;
	frameBuffer = frame;
	refreshBuffer = true;

	frameBufferLock->unlock();
}

error_t sdlBase::init_window(void)
{
	window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		windowWidth, windowHeight,
		SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (window == nullptr) {
		return -1;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr) {
		return -1;
	}

	// Create mouse location thread


	return 0;
}

void sdlBase::set_controller_obj(__inout void *ptr) const
{
	controllerPtr = (controller::loopTimer *)ptr;
}

//EOF