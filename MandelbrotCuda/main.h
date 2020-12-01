#pragma once

//#include "stdafx.h"
#include <stdint.h>
#include <string>
#include <iostream>

// Tests the mandelbrot_cpu.h structures using PPM (obsolete)
#undef TEST_MANDELBROT_CPU_PPM

// Tests the frame:: class to write data into a file
#undef TEST_MANDELBROT_CPU_FRAME

// Tests the mandelbrot GPU renderer
#undef TEST_MANDELBROT_GPU

// Tests the controller path
#define TEST_CONTROLLER_PATH

#define PPM_OUTPUT_FILE				"output.ppm"

// Defines
#define FRAME_BUFFER_HEIGHT			2048 //10000
#define FRAME_BUFFER_LENGTH			2048 //10000
#define WINDOW_NAME					"v0.1"

// Defines the size of the render window
#define RENDER_WINDOW_HEIGHT		1300
#define RENDER_WINDOW_LENGTH		2500

// Max CUDA iteratations of mandelbrot
#define CUDA_MANDELBROT_INTERATIONS 512

#define FRACTAL_OFFSET_X			-1.4312  // -0.6
#define FRACTAL_OFFSET_Y			0.0   // 0.0
	
#define DEBUG_LOG_FILE				"debug.log"

// CUDA scale parameter
#define IMAGE_SCALEA				1.1  //1.0
#define IMAGE_SCALEB				4.0  //4.0

// Scale and position delta
#define DELTA_SCALEA				0.001
#define DELTA_SCALEB				0.000
#define DELTA_OFFSETX				-0.0
#define DELTA_OFFSETY				-0.0

//EOF