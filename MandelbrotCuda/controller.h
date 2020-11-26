#pragma once

#include "types.h"
#include "sdl_render.h"

namespace controller {
	class loopTimer {
	private:
		// Inputs 
		const double origOffsetX, origOffsetY;

		// Total size of the pixelBuffer (in bytes)
		const size_t pixelBufferRawSize;
		const size_t pixelLength, pixelHeight;

		// Pointer from CUDA kernel to the rgba frame buffer
		rgbaPixel* frameBuffer;

		// SDL2 render loop
		render::sdlBase* renderer;


	public:
		loopTimer(
			double offsetX, double offsetY,
			size_t length, size_t height) :

			origOffsetX(offsetX), origOffsetY(offsetY),
			pixelLength(length), pixelHeight(height), pixelBufferRawSize(length* height * sizeof(rgbaPixel))
		{

		}

		error_t init_sdl2_renderer(void)
		{

			return 0;
		}
	};
}



//EOF