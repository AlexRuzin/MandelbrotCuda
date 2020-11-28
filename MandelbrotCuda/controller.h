#pragma once

#include <Windows.h>

#include <random>
#include <thread>
#include <string>
#include <chrono>
#include <assert.h>


#include "types.h"
#include "cudaMandelbrot.h"
#include "sdl_render.h"
#include "debug.h"

#define DEFAULT_WINDOW_NAME "sdl_window"

// Time for the CUDA rendering thread to wait
#define CUDA_THREAD_SLEEP_TIME 10 //ms

// Measures CUDA execution time
#define MEASURE_CUDA_EXECUTION_TIME 


// prng
// https://stackoverflow.com/questions/25298585/efficiently-generating-random-bytes-of-data-in-c11-14
using random_bytes_engine = std::independent_bits_engine<std::default_random_engine, CHAR_BIT, unsigned char>;

namespace controller {
	class loopTimer {
	private:
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

		// SDL2 render loop
		render::sdlBase *renderer;

		// CUDA renderer
		cuda::cudaKernel *cudaKernel;
		std::thread *cudaThread;
		bool runCudaThread;

		// Test renderer (debug only)
		std::thread *testFrameThread;
		bool runTestFrameThread;

	private:
		rgbaPixel *generate_blank_frame(size_t pixelCount) const
		{
			rgbaPixel *out = (rgbaPixel *)std::malloc(pixelCount * sizeof(rgbaPixel));
			std::memset((void*)out, 0x0, (size_t)(pixelCount * sizeof(rgbaPixel)));
			for (uint32_t i = 0; i < pixelCount; i++) {
				out[i].red = std::rand() % 256;
				out[i].blue = std::rand() % 256;
				out[i].green = std::rand() % 256;
			}

			return out;
		}

		/*
		 * CUDA rendering thread (primary)
		 */
		friend static void cuda_render_thread(loopTimer *controller)
		{
			while(controller->runCudaThread) {
				Sleep(10);

#if defined(MEASURE_CUDA_EXECUTION_TIME)
				auto t1 = std::chrono::high_resolution_clock::now();
#endif //MEASURE_CUDA_EXECUTION_TIME
				error_t err = controller->cudaKernel->generate_mandelbrot();
				if (err != 0) {
					DERROR("Error in generating CUDA kernel: " + std::to_string(err));
				}				

				rgbaPixel *pixelBuffer = controller->cudaKernel->get_pixel_buffer();
				assert(pixelBuffer != nullptr);

				controller->renderer->write_static_frame(pixelBuffer, controller->pixelLength, controller->pixelHeight);

#if defined(MEASURE_CUDA_EXECUTION_TIME)
				auto t2 = std::chrono::high_resolution_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
				const double cudaExecTime = std::chrono::duration<double>(duration).count();
				controller->renderer->update_cuda_rendering_stats(render::cudaRenderingStats{ cudaExecTime });
#endif //MEASURE_CUDA_EXECUTION_TIME
			}
		}

		/*
		 * Static frame rendering thread (test)
		 */
#define STATIC_FRAME_RENDERING_WAIT_TIME 1 //ms
		friend static void frame_render_thread(loopTimer *controller)
		{
			while (controller->testFrameThread) {
				rgbaPixel *out = controller->generate_blank_frame(controller->pixelBufferRawSize);

				controller->renderer->write_static_frame(out, controller->pixelLength, controller->pixelHeight);

				std::this_thread::sleep_for(std::chrono::milliseconds(STATIC_FRAME_RENDERING_WAIT_TIME));
			}
		}

	public:
		loopTimer(
			double offsetX, double offsetY,
			size_t length, size_t height,
			double scaleA, double scaleB) :
			renderer(nullptr),
			origOffsetX(offsetX), origOffsetY(offsetY),
			pixelLength(length), pixelHeight(height), pixelBufferRawSize(length* height * sizeof(rgbaPixel)),
			cudaKernel(nullptr), cudaThread(nullptr),
			runCudaThread(false),
			origScaleA(scaleA), origScaleB(scaleB)
		{

		}

		/*
		 * Creates the CUDA thread (default)
		 */
		error_t create_cuda_thread(void)
		{
			assert(cudaKernel == nullptr);
			this->cudaKernel = new cuda::cudaKernel(
				origOffsetX, origOffsetY, 
				pixelLength, pixelHeight, 
				origScaleA, origScaleB);

			runCudaThread = true;
			this->cudaThread = new std::thread(&cuda_render_thread, this);
			DINFO("Created CUDA rendering thread");

			return 0;
		}

		/*
		 * Creates the test thread
		 */
		error_t create_test_thread(void)
		{
			runTestFrameThread = true;
			this->testFrameThread = new std::thread(frame_render_thread, this);
			DINFO("Created TEST rendering thread");

			return 0;
		}

		void stop_cuda_thread(void)
		{
			assert(runCudaThread == true);
			runCudaThread = false;
		}

		error_t init_sdl2_renderer(const std::string windowName)
		{
			windowName != "" ? sdlWindowTitle = windowName : sdlWindowTitle = DEFAULT_WINDOW_NAME;
			this->renderer = new render::sdlBase(pixelHeight, pixelLength, sdlWindowTitle);
			error_t err = renderer->init_window();
			if (err != 0) {
				DERROR("Failed to initialize SDL2 window");
				return err;
			}

			rgbaPixel *blankFrame = generate_blank_frame(pixelBufferRawSize);
			renderer->write_static_frame(blankFrame, pixelLength, pixelHeight);

			err = renderer->enter_render_loop();
			if (err != 0) {
				DERROR("Failed to enter SDL2 loop");
				return err;
			}

			return 0;
		}
	};
}

//EOF