#pragma once

#include <stdint.h>

#include "frame.h"

#include "cuda_occupancy.h"
#include "cuda_runtime.h"
#include "cuda_profiler_api.h"

#define IMAGE_SCALEA 1.0  //1.0
#define IMAGE_SCALEB 4.0 //4.0

#define USE_CUDA_PERFORMANCE_METRICS

namespace cuda {
	class cudaKernel {
	private:
		double cx, cy;
		const uint32_t image_size;
		const uint32_t image_width, image_height;
		const double scale;
		frame::image *image;

		double scaleA, scaleB;

		frame::rgbPixel *pixelData;
		unsigned char *rawFrameBuffer;

	public:
		error_t generate_mandelbrot(void);

	private:
		template<class T, typename... A>
		error_t launch_kernel(T& kernel, dim3 work, A&&... args);

	public:
		cudaKernel(frame::image* image, double cx, double cy, double scaleA, double scaleB) :
			cx(cx), cy(cy),
			image_size(image->get_height() * image->get_width() * sizeof(frame::rgbPixel)),
			scale(IMAGE_SCALEA / (image->get_width() / IMAGE_SCALEB)),
			image_width(image->get_width()), image_height(image->get_height()),
			image(image),
			scaleA(scaleA == 0 ? IMAGE_SCALEA : scaleA), scaleB(scaleB == 0 ? IMAGE_SCALEB : scaleB),
			rawFrameBuffer(nullptr)
		{

		}

		~cudaKernel(void)
		{

		}

		// Returns the raw frame buffer as r,g,b,a from the 3 byte rgbPixel structure
		const unsigned char* get_raw_frame(void)
		{
			if (rawFrameBuffer != nullptr) {
				std::free(rawFrameBuffer);
			}
			rawFrameBuffer = (unsigned char*)std::malloc(image_size * sizeof(uint8_t) * 4);
			std::memset(rawFrameBuffer, 0x0, image_size * sizeof(uint8_t) * 4);

			for (uint32_t i = 0; i <= image_size; i++) {
				rawFrameBuffer[i] = pixelData[i].red;
				rawFrameBuffer[i + 1] = pixelData[i].green;
				rawFrameBuffer[i + 2] = pixelData[i].blue;
			}
		}

		void setScaleA(double a)
		{
			scaleA = a;
		}
		void setScaleB(double b)
		{
			scaleB = b;
		}
		void setOffsetX(double x)
		{
			cx = x;
		}
		void setOffsetY(double y)
		{
			cy = y;
		}
	};	
}