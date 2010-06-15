package com.goeswhere.tracer;

import static com.goeswhere.tracer.Mm._mm_set1_ps;

class AJRGB
{
	static int toRgb(byte[] bs, int off) {
		return (bs[off + 0] << 16 & 0x00ff0000) + (bs[off + 1] << 8 & 0x0000ff00) + (bs[off + 2] & 0x000000ff);
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

	@Override public String toString() {
		return red + ":" + green + ":" + blue;
	}
}
