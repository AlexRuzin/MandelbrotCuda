#pragma once

#include <random>

#include "types.h"
#include "sdl_render.h"

#define DEFAULT_WINDOW_NAME "sdl_window"

// prng
// https://stackoverflow.com/questions/25298585/efficiently-generating-random-bytes-of-data-in-c11-14
using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;

namespace controller {
	class loopTimer {
	private:
		// Inputs 
		const double origOffsetX, origOffsetY;

		// Total size of the pixelBuffer (in bytes)
		const size_t pixelBufferRawSize;
		const size_t pixelLength, pixelHeight;

		// Window name
		std::string sdlWindowTitle;

		// Pointer from CUDA kernel to the rgba frame buffer
		rgbaPixel* frameBuffer;

		// SDL2 render loop
		render::sdlBase *renderer;

	private:
		rgbaPixel *generate_blank_frame(size_t pixelCount) const
		{
			rgbaPixel *out = (rgbaPixel *)std::malloc(pixelCount * sizeof(rgbaPixel));
			std::memset((void*)out, 0x0, (size_t)(pixelCount * sizeof(rgbaPixel)));
			for (uint32_t i = 0; i < pixelCount; i++) {
				out[i].red = std::rand() % 256;
				out[i].blue = std::rand() % 256;
				out[i].green = std::rand() % 256;
			}

			return out;
		}

	public:
		loopTimer(
			double offsetX, double offsetY,
			size_t length, size_t height) :

			renderer(nullptr),
			origOffsetX(offsetX), origOffsetY(offsetY),
			pixelLength(length), pixelHeight(height), pixelBufferRawSize(length* height * sizeof(rgbaPixel))
		{

		}



		error_t init_sdl2_renderer(const std::string windowName)
		{
			windowName != "" ? sdlWindowTitle = windowName : sdlWindowTitle = DEFAULT_WINDOW_NAME;
			this->renderer = new render::sdlBase(pixelHeight, pixelLength, sdlWindowTitle);
			error_t err = renderer->init_window();
			if (err != 0) {
				DERROR("Failed to initialize SDL2 window");
				return err;
			}

			rgbaPixel *blankFrame = generate_blank_frame(pixelBufferRawSize);
			renderer->write_static_frame(blankFrame, pixelLength, pixelHeight);

			err = renderer->enter_render_loop();
			if (err != 0) {
				DERROR("Failed to enter SDL2 loop");
				return err;
			}

			return 0;
		}
	};
}

//EOF