#pragma once

//#include "main.h"

#include <SDL2/SDL.h>

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

		size_t get_checksum(void) const
		{
			size_t seed = data.size();
			for (auto& i : data) {
				seed ^= (i.blue + i.red + i.green) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}

		// Exports frame buffer in following format:
		//  Each pixel is 4 bytes: r, g, b, alpha
		std::vector<uint8_t> export_raw_frame_buffer(void) const
		{
			std::vector<uint8_t> out;
			out.resize(data.size() * (sizeof(uint8_t) * 4));
			uint32_t bufOffset = 0;
			for (std::vector<rgbPixel>::const_iterator i = data.begin(); i != data.end(); i++, bufOffset += 4) {
				out[bufOffset] = (i->red);
				out[bufOffset + 1] = (i->green);
				out[bufOffset + 2] = (i->blue);
				out[bufOffset + 3] = SDL_ALPHA_OPAQUE;
			}

			return out;
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

//EOF