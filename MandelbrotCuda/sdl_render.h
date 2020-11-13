#pragma once

#include <stdint.h>

namespace render
{
	class sdlBase {
	private:
		uint32_t windowHeight, windowLength;

	public:
		sdlBase(uint32_t height, uint32_t length) :
			windowHeight(height), windowLength(length)
		{

		}

		~sdlBase()
		{

		}
	};
}