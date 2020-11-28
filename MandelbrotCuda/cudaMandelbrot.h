#pragma once

#include <stdint.h>
#include <assert.h>

#include "types.h"

#include "cuda_occupancy.h"
#include "cuda_runtime.h"
#include "cuda_profiler_api.h"

#define USE_CUDA_PERFORMANCE_METRICS

// Debug output for CUDA grid sizes
#define CUDA_DEBUG_OUT

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
		rgbaPixel *pixelBuffer;

		// Scales
		double scaleA, scaleB;
		double scale;

	public:
		error_t generate_mandelbrot(void);

		rgbaPixel *get_pixel_buffer(void) const
		{
			assert(pixelBuffer != nullptr);
			return pixelBuffer;
		}

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
			scale(1.0 / (pixelLength / 4.0))
		{

		}

		// Constructor for CUDA
		cudaKernel(double offsetX, double offsetY, size_t pixelLength, size_t pixelHeight, double scaleA, double scaleB) :
			origOffsetX(offsetX), origOffsetY(offsetY),
			offsetX(offsetX), offsetY(offsetY),
			pixelBuffer(nullptr),
			pixelLength(pixelLength), pixelHeight(pixelHeight),
			pixelBufferRawSize(pixelLength *pixelHeight * sizeof(rgbaPixel)),
			scaleA(scaleA), scaleB(scaleB),
			scale(scaleA / (pixelLength / scaleB))
		{

		}

		~cudaKernel(void)
		{

		}

		double getOffsetX(void) const { return offsetX; }
		double getOffsetY(void) const { return offsetY; }
		double getScaleA(void) const { return scaleA; }
		double getScaleB(void) const { return scaleB; }

		void setOffsetX(double val) { offsetX = val; }
		void setOffsetY(double val) { offsetY = val; }
		void setScaleA(double val) { scaleA = val; }
		void setScaleB(double val) { scaleB = val; }
	};	
}

//EOF