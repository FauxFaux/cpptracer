#pragma once

struct RGBA 
{
	RGBA() : red(0), green(0), blue(0) { }
	RGBA(const uchar R, const uchar G, const uchar B) : red(R), green(G), blue(B) { }

	uchar blue;
	uchar green;
	uchar red;
};
