#pragma once

struct AJRGB 
{
	AJRGB() : red(0), green(0), blue(0) { }
	AJRGB(const uchar R, const uchar G, const uchar B) : red(R), green(G), blue(B) { }

	uchar blue;
	uchar green;
	uchar red;
};

struct SSERGB
{
	SSERGB(float r, float g, float b)
	{
		red = _mm_set1_ps(r);
		green = _mm_set1_ps(g);
		blue = _mm_set1_ps(b);
	}

	SSEFloat red, green, blue;
};