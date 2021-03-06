#include "cudaMandelbrot.h"
#include "debug.h"
#include "main.h"

#include "cuda_occupancy.h"
#include "cuda_runtime.h"
#include "cuda_profiler_api.h"

#include <assert.h>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>      // std::stringstream

float elapsedTime = 0;

using namespace cuda;

#if !defined(_DEBUG)
#define _DEBUG
#endif //_DEBUG

#if _DEBUG
#   define cudaCall(cuda_func, ...) { cudaError_t status = cuda_func(__VA_ARGS__); cudaAssert((status), __FILE__, #cuda_func, __LINE__); }
#else
#   define cudaCall(cuda_func, ...) { cudaError_t status = cuda_func(__VA_ARGS__); }
#endif

inline void cudaAssert(cudaError_t status, const char* file, const char* func, int line)
{
	if (status != cudaSuccess)
	{
		std::stringstream ss;
		ss << "Error: " << cudaGetErrorString(status) << std::endl;
		ss << "Func: " << func << std::endl;
		ss << "File: " << file << std::endl;
		ss << "Line: " << line << std::endl;

		DERROR(ss.str());

		throw std::runtime_error(ss.str());
	}
}

int get_thread_compute_capability(int major, int minor);

__constant__ rgbaPixel pixel_colour[16] =
{
	{ 66,  30,  15 },
	{ 25,   7,  26 },
	{ 9,   1,  47 },
	{ 4,   4,  73 },
	{ 0,   7, 100 },
	{ 12,  44, 138 },
	{ 24,  82, 177 },
	{ 57, 125, 209 },
	{ 134, 181, 229 },
	{ 211, 236, 248 },
	{ 241, 233, 191 },
	{ 248, 201,  95 },
	{ 255, 170,   0 },
	{ 204, 128,   0 },
	{ 153,  87,   0 },
	{ 106,  52,   3 }
};

__global__ void mandelbrot_kernel(rgbaPixel* image,
	int32_t width, int32_t height,
	double scale,
	double cx, double cy);

template<class T, typename... A>
error_t cudaKernel::launch_kernel(T& kernel, dim3 work, A&&... args)
{
	int device;
	cudaDeviceProp props;
	cudaCall(cudaGetDevice, &device);
	cudaCall(cudaGetDeviceProperties, &props, device);

	int threadBlocks = get_thread_compute_capability(props.major, props.minor);

	int blockSize;
	std::uint32_t minGridSize;
	cudaOccupancyMaxPotentialBlockSize((int*)&minGridSize, &blockSize, kernel, 0, 0);
	
	
	int maxActiveBlocks = 0;
	cudaOccupancyMaxActiveBlocksPerMultiprocessor(&maxActiveBlocks, kernel, blockSize, 0);

	/*
	do
	{
		cudaOccupancyMaxActiveBlocksPerMultiprocessor(&maxActiveBlocks, kernel, blockSize, 0);

		if (blockSize < props.warpSize || maxActiveBlocks >= threadBlocks)
		{
			break;
		}

		blockSize -= props.warpSize;
	} while (true);	
	*/

	int blockSizeDimX, blockSizeDimY;
	blockSizeDimX = blockSizeDimY = (int)pow(2, ceil(log(sqrt(blockSize)) / log(2)));

	while (blockSizeDimX * blockSizeDimY > blockSize)
	{
		blockSizeDimY--;
	}

	dim3 block(blockSizeDimX, blockSizeDimY);
	dim3 grid((work.x + block.x - 1) / block.x, (work.y + block.y - 1) / block.y);
	grid.x = grid.x > minGridSize ? grid.x : minGridSize;
	grid.y = grid.y > minGridSize ? grid.y : minGridSize;

#undef CUDA_DEBUG_OUT
#ifdef CUDA_DEBUG_OUT
	float occupancy = (maxActiveBlocks * blockSize / props.warpSize) / (float)(props.maxThreadsPerMultiProcessor / props.warpSize);

	DINFO("Grid of size: " + std::to_string(grid.x * grid.y));
	DINFO("Launched blocks of size " + std::to_string(blockSize));
	DINFO("Theoretical occupancy " + std::to_string(occupancy * 100.0f) + "%");
#endif //CUDA_DEBUG_OUT

	cudaEvent_t start;
	cudaCall(cudaEventCreate, &start);

	cudaEvent_t stop;
	cudaCall(cudaEventCreate, &stop);

	cudaCall(cudaEventRecord, start, 0);

	kernel << < grid, block >> > (std::forward<A>(args)...);

	cudaCall(cudaGetLastError);
	cudaCall(cudaEventRecord, stop, 0);
	cudaCall(cudaEventSynchronize, stop);

	cudaCall(cudaEventElapsedTime, &elapsedTime, start, stop);

	cudaCall(cudaEventDestroy, start);
	cudaCall(cudaEventDestroy, stop);

	cudaCall(cudaProfilerStop);

	return 0;
}

int get_thread_compute_capability(int major, int minor)
{
	switch (major) {
	case 1:
		return 0;
	case 2:
	case 3:
		switch (minor) {
		case 0:
			return 16;
		case 2:
			return 4;
		case 5:
		case 7:
			return 32;
		default:
			return 0;
		}
	case 5:
		switch (minor) {
		case 0:
		case 2:
			return 32;
		case 3:
			return 16;
		}
	case 6:
		switch (minor) {
		case 0:
			return 128;
		case 1:
			return 32;
		case 2:
			return 16;
		default:
			return 0;
		}
	case 7:
		if (minor == 2) return 16;
		return 128;
	default:
		return 0;
	}

	return 0;
}

error_t cudaKernel::generate_mandelbrot(void)
{
	rgbaPixel *cudaBuffer;

	cudaCall(cudaMalloc, (void**)&cudaBuffer, pixelBufferRawSize);
	cudaCall(cudaMemset, cudaBuffer, 0x0, pixelBufferRawSize);

	scale = scaleA / ((double)pixelLength / scaleB);
	error_t err = launch_kernel(mandelbrot_kernel,
		dim3((int32_t)pixelLength, (int32_t)pixelHeight), 
		cudaBuffer,
		(int32_t)pixelLength, (int32_t)pixelHeight, 
		scale, 
		offsetX, offsetY);
	if (err != 0) {
		return err;
	}

	pixelBuffer = (rgbaPixel *)std::malloc(pixelBufferRawSize);
	std::memset(pixelBuffer, 0x0, pixelBufferRawSize);
	cudaCall(cudaMemcpy, (void*)&pixelBuffer[0], (const void *)cudaBuffer, 
		(const size_t)pixelBufferRawSize, cudaMemcpyDeviceToHost);
	cudaCall(cudaFree, cudaBuffer);

	return 0;
}

__global__ void mandelbrot_kernel(rgbaPixel *image,
	int32_t width, int32_t height,
	double scale,
	double cx, double cy)
{
	const int i = threadIdx.y + blockIdx.y * blockDim.y;
	const int j = threadIdx.x + blockIdx.x * blockDim.x;

	if (i >= height || j >= width)
	{
		return;
	}

	const std::uint32_t max_iter = CUDA_MANDELBROT_INTERATIONS;
	const double y = ((double)i - (double)(height >> 1)) * scale + cy;
	const double x = ((double)j - (double)(width >> 1)) * scale + cx;

	double zx = hypot(x - 0.25, y);

	if (x < zx - 2.0 * zx * zx + 0.25 || (x + 1.0) * (x + 1.0) + y * y < 0.0625)
	{
		return;
	}

	std::uint32_t iter = 0;
	double zy, zx2, zy2;
	zx = zy = zx2 = zy2 = 0.0;

	do {
		zy = 2.0 * zx * zy + y;
		zx = zx2 - zy2 + x;
		zx2 = zx * zx;
		zy2 = zy * zy;
	} while (iter++ < max_iter && zx2 + zy2 < 4.0);

	if (iter > 0 && iter < max_iter)
	{
		const std::uint8_t colour_idx = iter % 16;

		image[i * width + j].red = pixel_colour[colour_idx].red;
		image[i * width + j].green = pixel_colour[colour_idx].green;
		image[i * width + j].blue = pixel_colour[colour_idx].blue;
		image[i * width + j].alpha = 0x0;
	}
}