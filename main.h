#pragma once

#include <stdint.h>
#include <complex>
#include <vector>
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
	std::vector<std::vector<uint32_t>> *compute_image()
	{
		std::vector<std::vector<uint32_t>> *o = new std::vector<std::vector<uint32_t>>;
		for (uint32_t y = 0; y < yWindowLength; y++) {
			std::vector<uint32_t> line;
			for (uint32_t x = 0; x < xWindowLength; x++) {
				line.push_back(compute_point(x, y));
			}
			o->push_back(line);
		}

		return o;
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

class writePPMFile {
private: 
	std::string filename;
	uint32_t length, height;

	std::vector<std::vector<uint32_t>> data;

public:
	writePPMFile(std::string filename, uint32_t length, uint32_t height) :
		filename(filename),
		length(length), height(height)
	{

	}

	~writePPMFile()
	{

	}
};