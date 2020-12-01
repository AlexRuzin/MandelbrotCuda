//#define _WIN32
#if defined(_WIN32)
#include "windows.h"
#endif //__WIN32

#undef USE_COMPLEXT_DEBUGGING
#if !defined(DEBUG_LIBRARY_INCLUDED)
#include "debug.h"
#endif //DEBUG_LIBRARY_INCLUDED

#include "main.h"
#include "mandelbrot_cpu.h"
#include "ppm.h"
#include "controller.h"
//#include "cudaMandelbrot.h"
//#include "sdl_render.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>
#include <iomanip>

int WINAPI WinMain(HINSTANCE hinstance,
    HINSTANCE hprevinstance,
    LPSTR lpcmdline,
    int ncmdshow)
{ 
    error_t err = 0;
    DINFO("Starting application");


    std::stringstream testSS(std::stringstream::in | std::stringstream::out);
    double testDouble = 0.000000000000000534;
    //OutputDebugStringA(to_string_with_precision(testDouble, 64).c_str());




#if defined(TEST_MANDELBROT_CPU_PPM)
    mandelbrotFractalCpu mFrac(34, WINDOW_LENGTH, WINDOW_HEIGHT);
    std::vector<ppm::ppm_pixel> *image = mFrac.compute_image_ppm();
    ppm::writePPMFile ppmFile(PPM_OUTPUT_FILE, *image, WINDOW_LENGTH, WINDOW_HEIGHT);
    error_t err = ppmFile.write_file();
    if (err != 0) {
        return err;
    }
#endif //TEST_MANDELBROT_CPU_PPM

#if defined(TEST_MANDELBROT_CPU_FRAME)
    mandelbrotFractalCpu mFrac(34, WINDOW_LENGTH, WINDOW_HEIGHT);
    frame::image frameBuf(WINDOW_LENGTH, WINDOW_HEIGHT);
    error_t err = mFrac.compute_image_frame(&frameBuf);
    if (err != 0) {
        return err;
    }
    if (frameBuf.write_to_file(PPM_OUTPUT_FILE) != 0) {
        return -1;
    }
#endif //TEST_MANDELBROT_CPU_FRAME

#if defined(TEST_CONTROLLER_PATH)
    controller::loopTimer controller(
        FRACTAL_OFFSET_X, FRACTAL_OFFSET_Y, 
        RENDER_WINDOW_LENGTH, RENDER_WINDOW_HEIGHT,
        IMAGE_SCALEA, IMAGE_SCALEB);

    err = controller.create_cuda_thread();
    if (err != 0) {
        DERROR("Error creating thread: " + std::to_string(err));
        return err;
    }

    // Main thread does not leave this loop
    err = controller.init_sdl2_renderer(WINDOW_NAME);
    if (err != 0) {
        DERROR("SDL2 engine exited with error:" + std::to_string(err));
        return err;
    }

#endif //TEST_CONTROLLER_PATH

#if defined(TEST_MANDELBROT_GPU)
    frame::image frameBuf(FRAME_BUFFER_LENGTH, FRAME_BUFFER_HEIGHT);
    cuda::cudaKernel kernel(&frameBuf, FRACTAL_OFFSET_X, FRACTAL_OFFSET_Y);
    if (kernel.generate_mandelbrot() != 0) {
        return -1;
    }

    const std::vector<uint8_t> rawFrameBuffer = frameBuf.export_raw_frame_buffer();
    DINFO("Frame buffer pixel count: " + std::to_string(rawFrameBuffer.size()));

    size_t dataChecksum = frameBuf.get_checksum();
    DINFO("Writing file to: " PPM_OUTPUT_FILE);
    if (frameBuf.write_to_file(PPM_OUTPUT_FILE) != 0) {
        DERROR("Failed to write file: " PPM_OUTPUT_FILE);
        return 0;
    }

    render::sdlBase renderer(RENDER_WINDOW_HEIGHT, RENDER_WINDOW_LENGTH, WINDOW_NAME);
    if (renderer.init_window() != 0) {
        DERROR("Failed to create sdlBase renderer");
        return -1;
    }

    renderer.write_static_frame(rawFrameBuffer, FRAME_BUFFER_LENGTH, FRAME_BUFFER_HEIGHT);

    // Enter rendering loop
    error_t renderErr = renderer.enter_render_loop();
    if (renderErr != 0) {
        DERROR("Renderer returned error: " + std::to_string(renderErr));
        return renderErr;
    }
#endif //TEST_MANDELBROT_CPU

#if defined(TEST_MANDELBROT_CPU_PPM)
    delete image;
#endif //TEST_MANDELBROT_CPU_PPM
    DINFO("Engine halted. Clean exit");
    return 0;
}

//EOF