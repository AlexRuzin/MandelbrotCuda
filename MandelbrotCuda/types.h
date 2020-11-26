#pragma once

//#include "stdafx.h"
#include <stdint.h>
#include <string>
#include <iostream>

typedef int32_t error_t;
typedef uint8_t BYTE, * PBYTE;

// Definition of the SDL_PIXELFORMAT_ARGB8888 format
typedef struct rgbaPixel {
	BYTE red;
	BYTE green;
	BYTE blue;
	BYTE alpha;
} RGBA_PIXEL, * PRGBA_PIXEL;
