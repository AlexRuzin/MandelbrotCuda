#pragma once

#include "main.h"
#include "ppm.h"
#include "frame.h"

#include <stdint.h>
#include <complex>
#include <vector>
//#include <ofstream>
//#include <unistd.h>

#define X_WINDOW_OFFSET 1.5
#define Y_WINDOW_OFFSET 0.5

#define COLOR_GRADIENT 255

class mandelbrotFractalCpu {
private:
	uint32_t iterations;
	float xWindowLength, yWindowLength;

private:
	uint32_t compute_point(uint32_t x, uint32_t y)
	{
		std::complex<float> point((float)x / xWindowLength - X_WINDOW_OFFSET,
			(float)y / yWindowLength - Y_WINDOW_OFFSET);

		std::complex<float> z(0, 0);
		uint32_t iter = 0;
		while (abs(z) < 2 && iter <= this->iterations) {
			z = z * z + point;
			iter++;
		}

		if (iter < iterations) {
			return COLOR_GRADIENT * iter / (iterations - 1);
		}
		else {
			return 0;
		}
	}

public:
	std::vector<ppm::ppm_pixel> *compute_image_ppm()
	{
		std::vector<ppm::ppm_pixel> *o = new std::vector<ppm::ppm_pixel>();
		for (uint32_t y = 0; y < yWindowLength; y++) {
			for (uint32_t x = 0; x < xWindowLength; x++) {
				o->push_back({x, y, compute_point(x, y)});
			}			
		}

		return o;
	}

	error_t compute_image_frame(__inout frame::image *frameBuffer)
	{
		for (uint32_t y = 0; y < yWindowLength; y++) {
			for (uint32_t x = 0; x < xWindowLength; x++) {
				frameBuffer->insert_pixel(compute_point(x, y), 0, 0);
			}
		}

		return 0;
	}

public:
	mandelbrotFractalCpu(uint32_t iterations, float xLength, float yLength) :
		iterations(iterations),
		xWindowLength(xLength), yWindowLength(yLength)
	{

	}

	~mandelbrotFractalCpu()
	{

	}
};

//EOF