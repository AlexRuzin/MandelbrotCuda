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

// Disables all on-screen counters
#undef DISABLE_FPS_COUNTERS

// Defines
#define FRAME_BUFFER_HEIGHT			2048 //10000
#define FRAME_BUFFER_LENGTH			2048 //10000
#define WINDOW_NAME					"v0.2"

// Defines the size of the render window
#define RENDER_WINDOW_HEIGHT		900 //1100
#define RENDER_WINDOW_LENGTH		1200 //1400

// Max CUDA iteratations of mandelbrot
#define CUDA_MANDELBROT_INTERATIONS 255

// Initial fractal offset (in complex plane)
#define FRACTAL_OFFSET_X			-1.41645612  // -0.6
#define FRACTAL_OFFSET_Y			0.0   // 0.0

#define DEBUG_LOG_FILE				"debug.log"

// CUDA scale parameter
#define IMAGE_SCALEA				1.1  //1.0
#define IMAGE_SCALEB				4.0  //4.0

// Zoom scale rate (i.e. how fast zoom in/out)
#define ZOOM_ALPHA					0.4  //0.5
#define ZOOM_BETA					2.5 //2.5

// Scale and position delta
#define DELTA_SCALEA				0.001
#define DELTA_SCALEB				0.000

// Loop delay for cuda_render_thread (controller auto-render loop) -- in milliseconds
#define CONTROLLER_LOOP_WAIT		5

// Cross hair color code (rgba)
#define CROSSHAIR_COLOR				{ 162, 243, 11 }

// Do not exceed this Delta Scale A
#define MAX_DELTA_SCALE_A			0.1

//EOF