package com.goeswhere.tracer;

public class IntersectionInfo
{

	IntersectionInfo(RtObject objects, AJRGB c) { objectPtrs = (objects); colours = (c); }

	void SetDistances(__m128 d) { distance = d; }
	void SetNormal(__m128 x, __m128 y, __m128 z) { normalX = x; normalY = y; normalZ = z; }


	__m128 distance;
	__m128 normalX;
	__m128 normalY;
	__m128 normalZ;
	RtObject objectPtrs;
	AJRGB colours;

}
