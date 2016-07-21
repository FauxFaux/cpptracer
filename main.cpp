// A C++ Raytracer written by Adam Miles.

#include <iostream>

#include "bitmap.h"
#include "tracer.h"


// OpenMP doesn't support > 64 threads on my machine, and is thus cheating :p.
//#define USEOPENMP

#if defined(USEOPENMP)
#include <omp.h>
#endif

#include <thread>
#include <boost/ptr_container/ptr_vector.hpp>

#ifndef _WINDOWS
# if defined(WIN32) || defined(_WIN32)
#  define _WINDOWS
# endif
#endif


const int defaultScreenWidth = 12800;
const int defaultScreenHeight = 7200;

//
//void render(AJRGB *pixelData, const int screenWidth, const int screenHeight, const int threadID, const int numThreads);
//
//inline SSEFloat getNearestObstruction(const Ray &rays);
//
//void raytrace(SSERGB &colour, const Ray &rays, const int iteration, const int w, const int h);
//
//void raytraceNonSSE(AJRGB &p, const Ray &ray);
//
//void setupScene();
//
//void startRender(AJRGB *pixelData, const int width, const int height, int numThreads);
//
//void writeBitmap(AJRGB *pixelData, const int screenWidth, const int screenHeight, const int threadCount);

#ifndef _WIN32

# include <ctime>
# include <sys/time.h>
#include <iomanip>

#else
# include <windows.h>
#endif

struct timer
{
#ifndef _WIN32
	double start;

	timer()
	{
		timeval now;
		gettimeofday(&now, 0);
		start = now.tv_sec + (now.tv_usec / 1000000.0);
	}

	double End()
	{
		timeval now;
		gettimeofday(&now, 0);
		double end = now.tv_sec + (now.tv_usec / 1000000.0);
		return static_cast<double>(end - start);
	}

#else
	LARGE_INTEGER start;
	timer()
	{
		QueryPerformanceCounter(&start);
	}

	double End()
	{
		LARGE_INTEGER end, freq;
		QueryPerformanceCounter(&end);
		QueryPerformanceFrequency(&freq);
		return static_cast<double>(end.QuadPart-start.QuadPart)/freq.QuadPart;
	}
#endif

	void output(double seconds)
	{
		std::cout << seconds * 1000 << "ms" << std::endl;
	}
};


int main(int argc, char *argv[])
{
	using std::setw;
	using std::right;

	if (argc == 1)
	{
		printf(" - cppraytracer[.exe] [width] [height] [runCount] [imageCount]\n");
		printf("[width] = Width of rendered image in pixels. Default = %i\n", defaultScreenWidth);
		printf("[height] = Height of rendered image in pixels. Default = %i\n", defaultScreenHeight);
		printf("[runCount] = Number of times to run each render at each thread count, lowest time is chosen. Has the effect of smoothing out the curve / ignoring sporadic CPU load. Default = 1\n");
		printf("[imageCount] = Writes out the rendered BMPs to disk for thread counts <= imageCount. eg '3' will render out images for threadCount 1, 2, 3. Default = 0\n");
	}

	int width = 0;
	int height = 0;
	int numRuns = 1;
	int writeImagesUpTo = 0;
	bool bench = false;

	if (argc == 2 && argv[1][0] == 'b')
	{
		bench = true;
	} else
	{

		if (argc > 1)
			width = atoi(argv[1]);

		if (argc > 2)
			height = atoi(argv[2]);

		if (argc > 3)
			numRuns = atoi(argv[3]);

		if (argc > 4)
			writeImagesUpTo = atoi(argv[4]);
	}

	// If no resolution specified, use defaults.
	if (width == 0 || height == 0)
	{
		width = defaultScreenWidth;
		height = defaultScreenHeight;
	}

	setupScene();

	// Render the image
	AJRGB *pixelData = new AJRGB[width * height];

	double lowest = std::numeric_limits<double>().max();
	int lowestThreads = 0;

	int min_val;
	int max_val;
	if (bench)
	{
		min_val = 8;
		max_val = 8;
	} else
	{
		min_val = 1;
		max_val = height;
	}

	for (int i = min_val; i <= max_val; ++i)
		if (height % i == 0)
		{
			memset(pixelData, 0, sizeof(AJRGB) * width * height);

			double iLowest = std::numeric_limits<double>().max();

			for (int r = 0; r < numRuns; r++)
			{
				timer t;
				startRender(pixelData, width, height, i);
				double time = t.End();

				if (time < lowest)
				{
					lowest = time;
					lowestThreads = i;
				}
				if (time < iLowest)
					iLowest = time;
			}

			std::cout << setw(4) << right << i << ": " << iLowest * 1000 << std::endl;

			if (i <= writeImagesUpTo)
				writeBitmap(pixelData, width, height, i);
		}

	std::cout << setw(4) << right << "Interleaved version." << std::endl;
	std::cout << setw(4) << right << "Lowest: " << lowest * 1000 << "ms at " << lowestThreads << " threads."
			  << std::endl;

	delete[] pixelData;

	return 0;
}
