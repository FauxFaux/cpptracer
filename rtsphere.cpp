#include "stdafx.h"
#include "rtsphere.h"

using namespace std;


// At a given point in the world, reflect a ray off the sphere's normal to that point.
void RTSphere::ReflectRayAtPoint(const SSEInt &rayDirX, const SSEInt &rayDirY, const SSEInt &rayDirZ, 
									const SSEInt &intPointX, const SSEInt &intPointY, const SSEInt &intPointZ,
									SSEInt &reflectedX, SSEInt &reflectedY, SSEInt &reflectedZ) const
{
	// Calculate the sphere normal at this point;
	SSEInt normalX = _mm_sub_ps(intPointX, _mm_set_ps1(position.x));
	SSEInt normalY = _mm_sub_ps(intPointY, _mm_set_ps1(position.y));
	SSEInt normalZ = _mm_sub_ps(intPointZ, _mm_set_ps1(position.z));

	// Normalise the sphere normal.
	NormalizeSSE(normalX, normalY, normalZ);

	// Reflect the ray rayDir in normal and store in reflected. 
	ReflectSSE(rayDirX, rayDirY, rayDirZ, normalX, normalY, normalZ, reflectedX, reflectedY, reflectedZ);

	// Normalise the reflected ray.
	NormalizeSSE(reflectedX, reflectedY, reflectedZ);
}

// An SSE-optimised version of an already optimised ray-sphere intersection
// algorithm. Also taken from PixelMachine at www.superjer.com.
// The variable names are poor but they are in the quadratic formula too.
SSEInt RTSphere::IntersectTest(const Ray& rays) const
{	
	SSEInt t = _mm_set_ps1(-1);
	
	SSEInt sPosX = _mm_set_ps1(position.x);
	SSEInt sPosY = _mm_set_ps1(position.y);
	SSEInt sPosZ = _mm_set_ps1(position.z);

	SSEInt ox = _mm_sub_ps(rays.positionX, sPosX);
	SSEInt oy = _mm_sub_ps(rays.positionY, sPosY);
	SSEInt oz = _mm_sub_ps(rays.positionZ, sPosZ);

	SSEInt rDirXS = _mm_mul_ps(rays.directionX, rays.directionX);
	SSEInt rDirYS = _mm_mul_ps(rays.directionY, rays.directionY);
	SSEInt rDirZS = _mm_mul_ps(rays.directionZ, rays.directionZ);

	SSEInt two = _mm_set_ps1(2);

	SSEInt a = _mm_mul_ps(_mm_add_ps(_mm_add_ps(rDirXS, rDirYS), rDirZS), two);

	SSEInt rDirOx = _mm_mul_ps(rays.directionX, ox);
	SSEInt rDirOy = _mm_mul_ps(rays.directionY, oy);
	SSEInt rDirOz = _mm_mul_ps(rays.directionZ, oz);

	SSEInt b = _mm_add_ps(rDirOx, rDirOy);
	b = _mm_add_ps(b, rDirOz);
	b = _mm_mul_ps(b, two);

	ox = _mm_mul_ps(ox, ox);
	oy = _mm_mul_ps(oy, oy);
	oz = _mm_mul_ps(oz, oz);

	SSEInt rad = _mm_set_ps1(radius);
	rad = _mm_mul_ps(rad, rad);

	SSEInt c = _mm_add_ps(ox, oy);
	c = _mm_add_ps(c, oz);
	c = _mm_sub_ps(c, rad);

	SSEInt twoac = _mm_mul_ps(_mm_mul_ps(a, c), two);

	SSEInt d = _mm_sub_ps(_mm_mul_ps(b, b), twoac);

	bool each[4];

	for(int r = 0; r < 4; r++)
	{
		if(asFloatArray(d)[r] >= 0)
			each[r] = false;
		else
			each[r] = true;
	}

	if(each[0] && each[1] && each[2] && each[3])
		return t;

	SSEInt one = _mm_set_ps1(1);
	a = _mm_div_ps(one, a);

	d = _mm_sqrt_ps(d);

	for(int r = 0; r < 4; r++)
	{
		if(!each[r])
		{
			asFloatArray(t)[r] = (-asFloatArray(d)[r] - asFloatArray(b)[r]) * asFloatArray(a)[r];

			if(asFloatArray(t)[r] >= 0)
				each[r] = true;
		}
	}

	if(each[0] && each[1] && each[2] && each[3])
		return t;

	for(int r = 0; r < 4; r++)
	{
		if(!each[r])
		{
			asFloatArray(t)[r] = (asFloatArray(d)[r] - asFloatArray(b)[r]) * asFloatArray(a)[r];

			if(asFloatArray(t)[r] >= 0)
				each[r] = true;
		}
	}

	return t;
}
