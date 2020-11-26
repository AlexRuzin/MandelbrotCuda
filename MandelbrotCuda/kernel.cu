#include "cudaMandelbrot.h"

#include "cuda_occupancy.h"
#include "cuda_runtime.h"
#include "cuda_profiler_api.h"

#define CUDA_MANDELBROT_INTERATIONS 32

float elapsedTime = 0;

using namespace cuda;

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
	cudaGetDevice(&device);
	cudaGetDeviceProperties(&props, device);

	int threadBlocks;
	if (props.major == 2)
	{
		threadBlocks = 8;
	}
	else if (props.major == 3)
	{
		threadBlocks = 16;
	}
	else
	{
		threadBlocks = 32;
	}

	threadBlocks = 8;

	int blockSize;
	std::uint32_t minGridSize;
	cudaOccupancyMaxPotentialBlockSize((int*)&minGridSize, &blockSize, kernel, 0, 0);
	
	int maxActiveBlocks = 0;
	do
	{
		cudaOccupancyMaxActiveBlocksPerMultiprocessor(&maxActiveBlocks, kernel, blockSize, 0);

		if (blockSize < props.warpSize || maxActiveBlocks >= threadBlocks)
		{
			break;
		}

		blockSize -= props.warpSize;
	} while (true);	

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

#ifdef CUDA_DEBUG_OUT
	float occupancy = (maxActiveBlocks * blockSize / props.warpSize) / (float)(props.maxThreadsPerMultiProcessor / props.warpSize);

	std::cout << "Grid of size " << grid.x * grid.y << std::endl;
	std::cout << "Launched blocks of size " << blockSize << std::endl;
	std::cout << "Theoretical occupancy " << occupancy * 100.0f << "%" << std::endl;
#endif //CUDA_DEBUG_OUT

	cudaEvent_t start;
	cudaEventCreate(&start);

	cudaEvent_t stop;
	cudaEventCreate(&stop);

	cudaEventRecord(start, 0);

	kernel << < grid, block >> > (std::forward<A>(args)...);

	cudaGetLastError();
	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);

	cudaEventElapsedTime(&elapsedTime, start, stop);

	cudaEventDestroy(start);
	cudaEventDestroy(stop);

	cudaProfilerStop();

	return 0;
}

error_t cudaKernel::generate_mandelbrot(void)
{
	rgbaPixel *cudaBuffer;

	cudaMalloc((void**)&cudaBuffer, pixelBufferRawSize);
	cudaMemset(cudaBuffer, 0, pixelBufferRawSize);

	error_t err = launch_kernel(mandelbrot_kernel,
		dim3((int32_t)pixelLength, (int32_t)pixelHeight), cudaBuffer,
		(int32_t)pixelLength, (int32_t)pixelHeight, scale, offsetX, offsetY);
	if (err != 0) {
		return err;
	}

	cudaMemcpy((void*)&this->pixelBuffer, cudaBuffer, pixelBufferRawSize, cudaMemcpyDeviceToHost);
	cudaFree(cudaBuffer);

	return 0;
}

__global__ void mandelbrot_kernel(rgbaPixel* image,
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

	const std::uint8_t max_iter = 255;
	const double y = (i - (height >> 1)) * scale + cy;
	const double x = (j - (width >> 1)) * scale + cx;

	double zx = hypot(x - 0.25, y);

	if (x < zx - 2.0 * zx * zx + 0.25 || (x + 1.0) * (x + 1.0) + y * y < 0.0625)
	{
		return;
	}

	std::uint8_t iter = 0;
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

		image[i * width + j] = pixel_colour[colour_idx];
	}
}