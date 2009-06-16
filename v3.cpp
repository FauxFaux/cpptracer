#include "StdAfx.h"

void Add(const V3& a, const V3& b, V3& out)
{
	out.x = a.x + b.x;
	out.y = a.y + b.y;
	out.z = a.z + b.z;
}

float Dot(const V3& a, const V3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

SSEInt DotSSE(const SSEInt &ax, const SSEInt &ay, const SSEInt &az, 
			  const SSEInt &bx, const SSEInt &by, const SSEInt &bz)
{
	return _mm_add_ps(_mm_add_ps(_mm_mul_ps(ax, bx), _mm_mul_ps(ay, by)), _mm_mul_ps(az, bz));
}

SSEInt LengthSSE(const SSEInt &ax, const SSEInt &ay, const SSEInt &az)
{
	return _mm_sqrt_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(ax, ax), _mm_mul_ps(ay, ay)), _mm_mul_ps(az, az)));
}

void Multiply(const V3& a, const float& b, V3& out)
{
	out.x = a.x * b;
	out.y = a.y * b;
	out.z = a.z * b;
}

void MultiplySSE(const SSEInt* xyzc, SSEInt* xyz)
{
	xyz[0] = _mm_mul_ps(xyzc[0], xyzc[3]);
	xyz[1] = _mm_mul_ps(xyzc[1], xyzc[3]);
	xyz[2] = _mm_mul_ps(xyzc[2], xyzc[3]);
}

void NormalizeSSE(SSEInt &x, SSEInt &y, SSEInt &z)
{
	SSEInt oneOverLength = _mm_rsqrt_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(x, x), _mm_mul_ps(y, y)), _mm_mul_ps(z, z)));

	x = _mm_mul_ps(x, oneOverLength);
	y = _mm_mul_ps(y, oneOverLength);
	z = _mm_mul_ps(z, oneOverLength);
}

void Normalize(V3 &out)
{
	float oneOverLength = InvSqrt(out.x * out.x + out.y * out.y + out.z * out.z);

	out.x *= oneOverLength;
	out.y *= oneOverLength;
	out.z *= oneOverLength;
}

// The formula for reflecting a vector in a normal.
void ReflectSSE(const SSEInt &rayDirX, const SSEInt &rayDirY, const SSEInt &rayDirZ, 
				const SSEInt &normalX, const SSEInt &normalY, const SSEInt &normalZ,
				SSEInt &reflectedX, SSEInt &reflectedY, SSEInt &reflectedZ)
{
	reflectedX = rayDirX; 
	reflectedY = rayDirY; 
	reflectedZ = rayDirZ;

	SSEInt numByTwo = 
		_mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(rayDirX, normalX), _mm_mul_ps(rayDirY, normalY)), _mm_mul_ps(rayDirZ, normalZ)), _mm_set_ps1(2));

	reflectedX = _mm_sub_ps(reflectedX, _mm_mul_ps(numByTwo, normalX));
	reflectedY = _mm_sub_ps(reflectedY, _mm_mul_ps(numByTwo, normalY));
	reflectedZ = _mm_sub_ps(reflectedZ, _mm_mul_ps(numByTwo, normalZ));
}

void Subtract(const V3& a, const V3& b, V3& out)
{
	out.x = a.x - b.x;
	out.y = a.y - b.y;
	out.z = a.z - b.z;
}

// The infamous "fast inverse square root" from Quake 3 source and numerous
// other articles.
float InvSqrt(float x)
{
	float xhalf = 0.5*x;
	int i = *(int*)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(float*)&i;
	x = x * (1.5 - xhalf * x * x);
	return x;
}