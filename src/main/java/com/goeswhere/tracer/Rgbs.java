package com.goeswhere.tracer;

class AJRGB
{
	AJRGB() { this(0,0,0); }
	AJRGB(int R, int G, int B) { red = (R); green = (G); blue = (B); }

	int blue;
	int green;
	int red;

	public void zero() {
		throw new UnsupportedOperationException();
	}
}

class SSERGB
{
	SSERGB(float r, float g, float b)
	{
		red = _mm_set1_ps(r);
		green = _mm_set1_ps(g);
		blue = _mm_set1_ps(b);
	}

	SSEFloat red, green, blue;
}
