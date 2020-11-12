
#include "main.h"
#include "mandelbrot_cpu.h"
#include "ppm.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <stdint.h>
#include <vector>

// Tests the mandelbrot_cpu.h structures using PPM (obsolete)
#undef TEST_MANDELBROT_CPU_PPM

// Tests the frame:: class to write data into a file
#define TEST_MANDELBROT_CPU_FRAME

#define PPM_OUTPUT_FILE "output.ppm"

#define WINDOW_HEIGHT 4096
#define WINDOW_LENGTH 4096

int main()
{
    DEBUG_INFO("Starting...");

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

#if defined(TEST_MANDELBROT_CPU_PPM)
    delete image;
#endif //TEST_MANDELBROT_CPU_PPM
    return 0;
}

//EOF