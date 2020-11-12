
#include "main.h"
#include "mandelbrot_cpu.h"
#include "ppm.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <stdint.h>
#include <vector>

#define TEST_MANDELBROT_CPU

#define PPM_OUTPUT_FILE "output.ppm"

#define WINDOW_HEIGHT 255
#define WINDOW_LENGTH 255

int main()
{
    DEBUG_INFO("Starting...");

#if defined(TEST_MANDELBROT_CPU)
    mandelbrotFractalCpu mFrac(34, WINDOW_LENGTH, WINDOW_HEIGHT);
    std::vector<ppm::ppm_pixel> *image = mFrac.compute_image_ppm();
    ppm::writePPMFile ppmFile(PPM_OUTPUT_FILE, *image, WINDOW_LENGTH, WINDOW_HEIGHT);
    error_t err = ppmFile.write_file();
    if (err != 0) {
        return err;
    }
#endif //TEST_MANDELBROT_CPU

#if defined(TEST_MANDELBROT_CPU)
    delete image;
#endif //TEST_MANDELBROT_CPU
    return 0;
}

//EOF