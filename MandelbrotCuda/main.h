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

//EOF