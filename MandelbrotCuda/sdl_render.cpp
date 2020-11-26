
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>
#include <stdint.h>
#include <assert.h>
#include <vector>
#include <mutex>
#include <thread>
#include <assert.h>

#include "sdl_render.h"
#include "debug.h"

using namespace render;

/*
 * sdlTimer 
 */
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

#if defined(RENDER_ENABLE_FPS_CAP)
	SDL_Color textColor = { 0, 0, 0, 255 };
	b->fpsTimer.start();
	uint32_t countedFrames = 0;
	std::stringstream timeText;
	LTexture fpsTexture(b->renderer);
#endif //RENDER_ENABLE_FPS_CAP

	SDL_Texture* frameTexture = SDL_CreateTexture
	(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		(uint32_t)framePixelLength, (uint32_t)framePixelHeight
	);

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

		if (fpsTexture.loadFromRenderedText(timeText.str().c_str(), textColor))
		{
			DERROR("render_loop: Failed to load rendered text");
			break;
		}
#endif //RENDER_ENABLE_FPS_CAP

		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent) != 0) {
			switch (sdlEvent.type) {
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

		// Draw raw frame buffer
		frameBufferLock.lock();
		if (refreshBuffer) {
			unsigned char* lockedPixels = nullptr;
			int pitch = 0;
			SDL_LockTexture
			(
				frameTexture,
				NULL,
				reinterpret_cast<void**>(&lockedPixels),
				&pitch
			);
			std::memcpy(lockedPixels, frameBuffer, frameTotalPixels * sizeof(rgbaPixel));
			SDL_UnlockTexture(frameTexture);
		}
		else {
			SDL_UpdateTexture
			(
				frameTexture,
				NULL,
				frameBuffer,
				frameTotalPixels * sizeof(rgbaPixel)
			);
		}
		frameBufferLock.unlock();

		// Copy frame image into renderer
		SDL_RenderCopy(renderer, frameTexture, NULL, NULL);

#if defined(RENDER_ENABLE_FPS_CAP)
#if defined(RENDER_SHOW_FPS_STRING)
		//texture.render((b->windowWidth - texture.getWidth()) / 2, (b->windowHeight - texture.getHeight()) / 2);
		fpsTexture.render(0, 0);
		countedFrames++;
#endif //RENDER_SHOW_FPS_STRING
#endif //RENDER_ENABLE_FPS_CAP

		SDL_RenderPresent(b->renderer);

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
	frameBufferLock.lock();

	if (this->frameBuffer != nullptr) {
		std::free(frameBuffer);
	}

	framePixelLength = length;
	framePixelHeight = height;
	frameTotalPixels = framePixelHeight * framePixelLength * sizeof(rgbaPixel);
	frameBuffer = frame;

	frameBufferLock.unlock();
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

	return 0;
}

//EOF