#pragma once

#include <cstdint>

struct AJRGB
{
	AJRGB() : blue(0), green(0), red(0)
	{}

	AJRGB(const uint8_t R, const uint8_t G, const uint8_t B) : blue(B), green(G), red(R)
	{}

	uint8_t blue;
	uint8_t green;
	uint8_t red;
};

void setupScene();

void startRender(AJRGB *pixelData, const int width, const int height, int numThreads);
