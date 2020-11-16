#define _WIN32
#if defined(_WIN32)
#include "windows.h"
#endif //__WIN32

#include "main.h"
#include "mandelbrot_cpu.h"
#include "ppm.h"
#include "cudaMandelbrot.h"
#include "sdl_render.h"
#include "debug.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <stdint.h>
#include <vector>

// Tests the mandelbrot_cpu.h structures using PPM (obsolete)
#undef TEST_MANDELBROT_CPU_PPM

// Tests the frame:: class to write data into a file
#undef TEST_MANDELBROT_CPU_FRAME

// Tests the mandelbrot GPU renderer
#define TEST_MANDELBROT_CPU

#define PPM_OUTPUT_FILE "output.ppm"

#define FRAME_BUFFER_HEIGHT     4096 //10000
#define FRAME_BUFFER_LENGTH     4096 //10000
#define WINDOW_NAME             "v0.1"

#define RENDER_WINDOW_HEIGHT    1024
#define RENDER_WINDOW_LENGTH    2048

#define FRACTAL_OFFSET_X        -0.6
#define FRACTAL_OFFSET_Y        0.0

#define DEBUG_LOG_FILE          "debug.log"

int WINAPI WinMain(HINSTANCE hinstance,
    HINSTANCE hprevinstance,
    LPSTR lpcmdline,
    int ncmdshow)
{
    const std::string* debugLogFile = new std::string(DEBUG_LOG_FILE);
    debug::init_debug(debugLogFile, DEBUG_FLAG_STDOUT | DEBUG_FLAG_LOGFILE | DEBUG_FLAG_CLEANLOGFILE | DEBUG_FLAG_TIMESTAMP);  
    DINFO("Starting application");

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


#if defined(TEST_MANDELBROT_CPU)
    frame::image frameBuf(FRAME_BUFFER_LENGTH, FRAME_BUFFER_HEIGHT);
    cuda::cudaKernel kernel(&frameBuf, FRACTAL_OFFSET_X, FRACTAL_OFFSET_Y);
    if (kernel.generate_mandelbrot() != 0) {
        return -1;
    }

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

    if (renderer.create_render_loop() != 0) {
        DERROR("Failed to create rendering loop");
        return -1;
    }
    DINFO("Main thread waiting for renderer exit signal");
    renderer.wait_for_render_exit();
#endif //TEST_MANDELBROT_CPU


#if defined(TEST_MANDELBROT_CPU_PPM)
    delete image;
#endif //TEST_MANDELBROT_CPU_PPM
    return 0;
}

//EOF