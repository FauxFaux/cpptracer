package com.goeswhere.tracer;

import static com.goeswhere.tracer.Mm.*;

class AJRGB
{
	AJRGB() { this(0,0,0); }
	AJRGB(int R, int G, int B) { red = (R); green = (G); blue = (B); }

	int blue;
	int green;
	int red;

	public void zero() {
		red = green = blue = 0;
	}
	public int toRgb() {
		return (red << 16) + (green << 8) + blue;
	}

	@Override public String toString() {
		return red + "-" + green + "-" + blue;
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
