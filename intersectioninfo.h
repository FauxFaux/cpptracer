#pragma once

#include "Pixel.h"
#include "RTObject.h"

class IntersectionInfo
{
public:
	IntersectionInfo(const RTObject* objects, const RGBA* c) : objectPtrs(objects), colours(c) { };

	~IntersectionInfo() 
	{ 
		std::cout << "1337\n";
	} ;
	
	void SetDistances(__m128 d) { distance = d; }
	void SetNormal(__m128 x, __m128 y, __m128 z) { normalX = x; normalY = y; normalZ = z; }

protected:
	
	__m128 distance;
	__m128 normalX;
	__m128 normalY;
	__m128 normalZ;
	const RTObject* objectPtrs;
	const RGBA* colours;
	
};
