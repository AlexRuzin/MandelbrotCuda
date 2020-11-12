#pragma once

#include "main.h"
#include "frame.h"

#include <string>
#include <vector>
#include <fstream>

namespace ppm {
	typedef struct ppm_pixel {
		uint32_t x, y, z;
	} PPM_PIXEL, *PPPM_PIXEL;

	class writePPMFile {
	private:
		std::string filename;

		uint32_t width, height;

		std::vector<ppm_pixel> data;


	public:
		error_t write_file()
		{
			std::ofstream mImage(filename);
			if (!mImage.is_open()) {
				return -1;
			}

			std::vector<ppm_pixel>::iterator pixelI = data.begin();

			mImage << "P3\n" << width << " " << height << " 255\n";
			for (uint32_t y = 0; y < height; y++) {
				for (uint32_t x = 0; x < width; x++) {
					//mImage << (*pixelI).z << " " << 
				}
			}

			return 0;
		}

		writePPMFile(std::string filename, std::vector<ppm_pixel> data, uint32_t width, uint32_t height) :
			height(height), width(width),
			filename(filename),
			data(data)
		{

		}

		~writePPMFile()
		{

		}
	};
}

//EOF