package com.goeswhere.tracer;

public class IntersectionInfo
{

	IntersectionInfo(RtObject objects, AJRGB c) { objectPtrs = (objects); colours = (c); }

	void SetDistances(SSEFloat d) { distance = d; }
	void SetNormal(SSEFloat x, SSEFloat y, SSEFloat z) { normalX = x; normalY = y; normalZ = z; }


	SSEFloat distance;
	SSEFloat normalX;
	SSEFloat normalY;
	SSEFloat normalZ;
	RtObject objectPtrs;
	AJRGB colours;

}
