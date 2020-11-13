#pragma once

#include "main.h"

#include <vector>
#include <cstdint>
#include <sstream>
#include <algorithm>
#include <functional>
#include <fstream>

namespace frame
{
	typedef struct rgbPixel {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
	} RGBPIXEL, *PRGBPIXEL;

	class image {
	private: 
		uint32_t width, height;

	public:
		std::vector<rgbPixel> data;

	public:
		image(int32_t width, int32_t height) :
			width(width), height(height)
		{
			data.resize(width * height);
		}

		uint32_t get_width() const
		{
			return this->width;
		}

		uint32_t get_height() const 
		{
			return this->height;
		}

		void insert_pixel(uint8_t r, uint8_t g, uint8_t b)
		{
			data.push_back({ r, g, b });
		}

		error_t write_to_file(std::string filename)
		{
			std::ofstream output(filename);
			if (!output.is_open()) {
				return -1;
			}

			output << "P6" << std::endl
				<< width << " "
				<< height << std::endl
				<< 255 << std::endl;

			std::size_t row_size = width * sizeof(rgbPixel);

			for (int i = height - 1; i >= 0; i--)
			{
				output.write((char*)&data[i * width], row_size);
			}

			return 0;
		}

		friend std::ostream& operator<<(std::ostream& output, const image& d)
		{
			output << "P6" << std::endl
				<< d.width << " "
				<< d.height << std::endl
				<< 255 << std::endl;

			std::size_t row_size = d.width * sizeof(rgbPixel);

			for (int i = d.height - 1; i >= 0; i--)
			{
				output.write((char*)&d.data[i * d.width], row_size);
			}

			return output;
		}
	};
}