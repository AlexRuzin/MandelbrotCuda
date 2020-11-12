#pragma once

#include <vector>
#include <cstdint>
#include <sstream>
#include <algorithm>
#include <functional>

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

		std::vector<rgbPixel> data;

	public:
		image(int32_t width, int32_t height) :
			width(width), height(height)
		{
			data.resize(width * height);
		}

		void insert_pixel(uint8_t r, uint8_t g, uint8_t b)
		{
			data.push_back({ r, g, b });
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
		}
	};
}