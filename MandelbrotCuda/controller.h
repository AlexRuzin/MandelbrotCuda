#pragma once

#include <Windows.h>

#include <random>
#include <thread>
#include <string>
#include <chrono>
#include <assert.h>

#include "types.h"
#include "cudaMandelbrot.h"

#define DEFAULT_WINDOW_NAME "sdl_window"

// Time for the CUDA rendering thread to wait
#define CUDA_THREAD_SLEEP_TIME 10 //ms

// Measures CUDA execution time
#define MEASURE_CUDA_EXECUTION_TIME 


// prng
// https://stackoverflow.com/questions/25298585/efficiently-generating-random-bytes-of-data-in-c11-14
using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;

namespace controller {

	// Sets the zoom to start, stop, or reverse
	typedef enum {
		SET_ZOOM_PAUSE,
		SET_ZOOM_REVERSE,
		SET_ZOOM_RESUME
	} USER_IO_STATE;

	class loopTimer {
	private:
		// States for the rendering thread
		typedef enum {
			THREAD_STATE_TERMINATED,
			THREAD_STATE_RUNNING,
			THREAD_STATE_PAUSED,
		} thread_state;

		// Inputs 
		const double origOffsetX, origOffsetY;
		const double origScaleA, origScaleB;

		// Total size of the pixelBuffer (in bytes)
		const size_t pixelBufferRawSize;
		const size_t pixelLength, pixelHeight;

		// Window name
		std::string sdlWindowTitle;

		// Pointer from CUDA kernel to the rgba frame buffer
		rgbaPixel* frameBuffer;	

		// CUDA renderer
		cuda::cudaKernel *cudaKernel;
		std::thread *cudaThread;
		thread_state threadStateCuda;

		// Test renderer (debug only)
		std::thread *testFrameThread;
		bool runTestFrameThread;

		// Controls the zoom on/off
		USER_IO_STATE user_io_state = SET_ZOOM_RESUME;

	private:
		rgbaPixel *generate_blank_frame(size_t pixelCount) const;

		/*
		 * CUDA rendering thread (primary)
		 */
		static void cuda_render_thread(loopTimer *controller);

		/*
		 * Process scale function
		 *  f(x) = Ae^(-k(x+c))
		 */
		static double process_scale(double in, double delta_scale)
		{
			double ex = std::exp(-0.6 * (in + 3.8));
			return ex;
		}

		/*
		 * Static frame rendering thread (test)
		 */
#define STATIC_FRAME_RENDERING_WAIT_TIME 1 //ms
		static void frame_render_thread(loopTimer *controller);

	public:
		loopTimer(
			double offsetX, double offsetY,
			size_t length, size_t height,
			double scaleA, double scaleB) :

			origOffsetX(offsetX), origOffsetY(offsetY),
			pixelLength(length), pixelHeight(height), pixelBufferRawSize(length* height * sizeof(rgbaPixel)),
			cudaKernel(nullptr), cudaThread(nullptr),
			threadStateCuda(THREAD_STATE_TERMINATED),
			origScaleA(scaleA), origScaleB(scaleB),
			user_io_state(SET_ZOOM_RESUME)
		{

		}

		/*
		 * Creates the CUDA thread (default)
		 */
		error_t create_cuda_thread(void);

		/*
		 * Pauses the CUDA rendering thread
		 *  The last rendered frame buffer will be stored
		 */
		error_t pause_cuda_thread(void);

		/*
		 * Creates the test thread
		 */
		error_t create_test_thread(void);

		/*
		 * Stop CUDA rendering thread
		 */
		void stop_cuda_thread(void);

		/*
		 * Initializes SDL2 renderer
		 */
		error_t init_sdl2_renderer(const std::string windowName);


		/*
		 * User I/O
		 *  This will pause automatic zoom of the fractal object through input by SDL2
		 */
		void set_user_io_state(USER_IO_STATE state);
	};
}

//EOF