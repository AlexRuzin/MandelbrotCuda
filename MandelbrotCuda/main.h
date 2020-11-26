#pragma once

//#include "stdafx.h"
#include <stdint.h>
#include <string>
#include <iostream>

typedef int32_t error_t;

// Definition for the rgba frame buffer 
//  SDL_PIXELFORMAT_ARGB8888 (SDL2)
typedef uint8_t BYTE, * PBYTE;
typedef struct rgbaPixel {
	BYTE red;
	BYTE green;
	BYTE blue;
	BYTE alpha;
};
typedef rgbaPixel* rgbaArray;

class controller {
private:
	// Inputs 
	const double origOffsetX, origOffsetY;
	
	// Total size of the pixelBuffer (in bytes)
	const size_t pixelBufferRawSize;
	const size_t pixelLength, pixelHeight;

	// Pointer from CUDA kernel to the rgba frame buffer
	rgbaPixel* frameBuffer;


public:
	controller(
		double offsetX, double offsetY,
		size_t length, size_t height) :

		origOffsetX(offsetX), origOffsetY(offsetY),
		pixelLength(length), pixelHeight(height), pixelBufferRawSize(length * height * sizeof(rgbaPixel))
	{

	}

};

//EOF