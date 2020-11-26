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
		// x,y position of the fractal image
		const double origOffsetX, origOffsetY;
		double offsetX, offsetY;

		// Total size of the pixelBuffer (in bytes)
		const size_t pixelBufferRawSize;
		const size_t pixelLength, pixelHeight;

		// CUDA frame buffer object
		rgbaPixel* pixelBuffer;

		const double scale;

	public:
		error_t generate_mandelbrot_ppm(void);

	private:
		template<class T, typename... A>
		error_t launch_kernel(T& kernel, dim3 work, A&&... args);

	public:
		// Constructor for PPM image generator
		cudaKernel(double offsetX, double offsetY, size_t pixelLength, size_t pixelHeight) :
			origOffsetX(offsetX), origOffsetY(offsetY),
			offsetX(offsetX), offsetY(offsetY),
			pixelBuffer(nullptr),
			pixelLength(pixelLength), pixelHeight(pixelHeight),
			pixelBufferRawSize(pixelLength * pixelHeight * sizeof(rgbaPixel)), 
			scale(IMAGE_SCALEA / (pixelLength / IMAGE_SCALEB))
		{

		}
	};	
}