#pragma once

#include <stdint.h>

#include "frame.h"

#include "cuda_occupancy.h"
#include "cuda_runtime.h"
#include "cuda_profiler_api.h"

#define IMAGE_SCALEA 1.0
#define IMAGE_SCALEB 4.0

#define USE_CUDA_PERFORMANCE_METRICS

namespace cuda {
	class cudaKernel {
	private:
		double cx, cy;
		const uint32_t image_size;
		const uint32_t image_width, image_height;
		const double scale;
		frame::image *image;

		frame::rgbPixel *pixelData;

	public:
		error_t generate_mandelbrot(void);

	private:
		template<class T, typename... A>
		error_t launch_kernel(T& kernel, dim3 work, A&&... args);

	public:
		cudaKernel(frame::image* image, double cx, double cy) :
			cx(cx), cy(cy),
			image_size(image->get_height() * image->get_width() * sizeof(frame::rgbPixel)),
			scale(IMAGE_SCALEA / (image->get_width() / IMAGE_SCALEB)),
			image_width(image->get_width()), image_height(image->get_height()),
			image(image)
		{

		}
	};	
}