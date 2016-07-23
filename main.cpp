// A C++ Raytracer written by Adam Miles.

#ifdef _MSC_VER
#	if _MSC_VER < 1700
#		error Visual Studio before 2015 has crappy timers
#	endif
#endif

#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>

#include <boost/ptr_container/ptr_vector.hpp>

#include "bitmap.h"


// OpenMP doesn't support > 64 threads on my machine, and is thus cheating :p.
//#define USEOPENMP

#if defined(USEOPENMP)
#  include <omp.h>
#endif


const int defaultScreenWidth = 12800;
const int defaultScreenHeight = 7200;

typedef std::chrono::high_resolution_clock our_clock;
typedef std::chrono::duration<double, std::milli> duration_t;

const duration_t very_large_time = std::chrono::seconds(std::numeric_limits<int>::max());

struct timer
{
	const our_clock::time_point start;

	timer() : start(our_clock::now())
	{}

	duration_t End()
	{
		return std::chrono::duration<double, std::milli>(our_clock::now() - start);
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
	}
	else
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

	duration_t overall_lowest = very_large_time;
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

			duration_t iteration_lowest = very_large_time;

			for (int r = 0; r < numRuns; r++)
			{
				timer t;
				startRender(pixelData, width, height, i);
				duration_t time = t.End();

				if (time < overall_lowest)
				{
					overall_lowest = time;
					lowestThreads = i;
				}
				if (time < iteration_lowest)
					iteration_lowest = time;
			}

			std::cout << setw(4) << right << i << ": " << iteration_lowest.count() << std::endl;

			if (i <= writeImagesUpTo)
				writeBitmap(pixelData, width, height, i);
		}

	std::cout << "Atomic row read, 256bit AVX version." << std::endl;
	std::cout << setw(4) << right << "Lowest: " << overall_lowest.count() << "ms at " << lowestThreads << " threads."
			  << std::endl;

	delete[] pixelData;

	return 0;
}
