#include "main.h"
#include "mandelbrot_cpu.h"
#include "ppm.h"
#include "cudaMandelbrot.h"
#include "sdl_render.h"

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

#define WINDOW_HEIGHT 10000
#define WINDOW_LENGTH 10000

#define FRACTAL_OFFSET_X -0.6
#define FRACTAL_OFFSET_Y 0.0

int main()
{
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
    frame::image frameBuf(WINDOW_LENGTH, WINDOW_HEIGHT);
    cuda::cudaKernel kernel(&frameBuf, FRACTAL_OFFSET_X, FRACTAL_OFFSET_Y);
    if (kernel.generate_mandelbrot() != 0) {
        return -1;
    }

    uint32_t dataChecksum = frameBuf.get_checksum();

    if (frameBuf.write_to_file(PPM_OUTPUT_FILE) != 0) {
        return 0;
    }
#endif //TEST_MANDELBROT_CPU


#if defined(TEST_MANDELBROT_CPU_PPM)
    delete image;
#endif //TEST_MANDELBROT_CPU_PPM
    return 0;
}

//EOF