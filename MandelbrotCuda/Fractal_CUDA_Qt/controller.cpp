#include <Windows.h>

#include <math.h>

#include "controller.h"

#include "main.h"
#include "types.h"
#include "cudaMandelbrot.h"
#include "sdl_render.h"
#include "debug.h"

using namespace controller;

static render::sdlBase *renderer = nullptr;

rgbaPixel *loopTimer::generate_blank_frame(size_t pixelCount) const
{
	rgbaPixel *out = (rgbaPixel *)std::malloc(pixelCount * sizeof(rgbaPixel));
	std::memset((void *)out, 0x0, (size_t)(pixelCount * sizeof(rgbaPixel)));
	for (uint32_t i = 0; i < pixelCount; i++) {
		out[i].red = std::rand() % 256;
		out[i].blue = std::rand() % 256;
		out[i].green = std::rand() % 256;
	}

	return out;
}

void loopTimer::cuda_render_thread(loopTimer *controller)
{
	cuda::cudaKernel *kernel = controller->cudaKernel;

	double lastSCALEA = 0.0000055;
	DINFO("Setting render thread to THREAD_STATE_RUNNING");
	while (controller->threadStateCuda == THREAD_STATE_RUNNING) {
		Sleep(CONTROLLER_LOOP_WAIT);

#if defined(MEASURE_CUDA_EXECUTION_TIME)
		auto t1 = std::chrono::high_resolution_clock::now();
#endif //MEASURE_CUDA_EXECUTION_TIME
		//kernel->setScaleA(kernel->getScaleA() + controller->process_scale(kernel->getScaleA(), 0.0));

		double newSCALEA = 0.0;
		switch (controller->user_io_state) {
		case SET_ZOOM_PAUSE:
			break; // Perform no zoom
		case SET_ZOOM_REVERSE:
			newSCALEA = ZOOM_ALPHA * kernel->getScaleA() * std::exp(-ZOOM_BETA * lastSCALEA);
			if (newSCALEA >= MAX_DELTA_SCALE_A) {
				newSCALEA = 0.0;
			}
			break;
		case SET_ZOOM_RESUME:
			newSCALEA = -ZOOM_ALPHA * kernel->getScaleA() * std::exp(-ZOOM_BETA * lastSCALEA);
		}

		// Sec scaling
		kernel->setScaleA(kernel->getScaleA() + newSCALEA);
		kernel->setScaleB(kernel->getScaleB());
		lastSCALEA -= newSCALEA;

		// Check for mouse override
		controller->mouseLock.lock();
		if (controller->setMouseState) {
			kernel->setOffsetX(kernel->getOffsetX() + (controller->mouseX * kernel->getScaleA()));
			kernel->setOffsetY(kernel->getOffsetY() + (controller->mouseY * kernel->getScaleA()));
			controller->setMouseState = false;
		}

		/*
		error_t err = kernel->generate_mandelbrot();
		if (err != 0) {
			DERROR("Error in generating CUDA kernel: " + std::to_string(err));
		}
		*/

		// Release mouse lockre
		controller->mouseLock.unlock();

		rgbaPixel *pixelBuffer = kernel->get_pixel_buffer();
		assert(pixelBuffer != nullptr);

		renderer->write_static_frame(pixelBuffer, controller->pixelLength, controller->pixelHeight);

#if defined(MEASURE_CUDA_EXECUTION_TIME)
		auto t2 = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		const double cudaExecTime = std::chrono::duration<double>(duration).count();
		render::cudaRenderingStats stats = {
			std::chrono::duration<double>(duration).count(),
			kernel->getOffsetX(),
			kernel->getOffsetY(),
			kernel->getScaleA(),
			kernel->getScaleB()
		};
		renderer->update_cuda_rendering_stats(stats);
#endif //MEASURE_CUDA_EXECUTION_TIME
	}
	DINFO("Terminating CUDA thread");
}

// Dump parameters to a new JSON file
error_t loopTimer::dump_parameters_json(void)
{
	//todo
	return 0;
}

// Mouse LEFT click control to offset fractal
void loopTimer::set_mouse_button_offset(uint32_t inMouseX, uint32_t inMouseY)
{
	assert(inMouseX <= RENDER_WINDOW_LENGTH && inMouseY <= RENDER_WINDOW_HEIGHT);

	this->inMouseX = inMouseX;
	this->inMouseY = inMouseY;
	// SDL2 starts at position (0,0), and our complex scale works from
	//  (-inf,inf), so we need to convert this type

	mouseLock.lock();

	if (inMouseX > 0.0f) {
		mouseX = (double)inMouseX * (2.0 / (double)RENDER_WINDOW_LENGTH) - 1.0;
	}

	if (inMouseY > 0.0f) {
		mouseY = (double)inMouseY * (2.0 / (double)RENDER_WINDOW_HEIGHT) - 1.0;
	}

	this->setMouseState = true;
	mouseLock.unlock();
	return;
}

void loopTimer::frame_render_thread(loopTimer *controller)
{
	while (controller->testFrameThread) {
		rgbaPixel *out = controller->generate_blank_frame(controller->pixelBufferRawSize);

		renderer->write_static_frame(out, controller->pixelLength, controller->pixelHeight);

		std::this_thread::sleep_for(std::chrono::milliseconds(STATIC_FRAME_RENDERING_WAIT_TIME));
	}
}

void loopTimer::set_user_io_state(USER_IO_STATE state)
{
	user_io_state = state;

	return;
}

error_t loopTimer::create_cuda_thread(void)
{
	assert(cudaKernel == nullptr);
	this->cudaKernel = new cuda::cudaKernel(
		origOffsetX, origOffsetY,
		pixelLength, pixelHeight,
		origScaleA, origScaleB);

	threadStateCuda = THREAD_STATE_RUNNING;
	this->cudaThread = new std::thread(&cuda_render_thread, this);
	DINFO("Created CUDA rendering thread");

	return 0;
}

error_t loopTimer::pause_cuda_thread(void)
{
	assert(threadStateCuda == THREAD_STATE_RUNNING);
	return 0;
}

error_t loopTimer::create_test_thread(void)
{
	runTestFrameThread = true;
	this->testFrameThread = new std::thread(frame_render_thread, this);
	DINFO("Created TEST rendering thread");

	return 0;
}

void loopTimer::stop_cuda_thread(void)
{
	assert(threadStateCuda == THREAD_STATE_RUNNING || threadStateCuda == THREAD_STATE_PAUSED);
	threadStateCuda = THREAD_STATE_TERMINATED;
}

error_t loopTimer::init_sdl2_renderer(const std::string windowName)
{
	windowName != "" ? sdlWindowTitle = windowName : sdlWindowTitle = DEFAULT_WINDOW_NAME;
	renderer = new render::sdlBase(pixelHeight, pixelLength, sdlWindowTitle);

	// Set the controller type for user I/O
	renderer->set_controller_obj((void *)this);

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