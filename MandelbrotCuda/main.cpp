
#include "main.h"

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <stdint.h>
#include <vector>

#define WINDOW_HEIGHT 255
#define WINDOW_LENGTH 255

uint32_t main()
{
    mandelbrotFractalCpu mFrac(34, WINDOW_LENGTH, WINDOW_HEIGHT);
    std::vector<std::vector<uint32_t>> *image = mFrac.compute_image();

    return 0;
}

//EOF