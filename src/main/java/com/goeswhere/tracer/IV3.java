#pragma once

#include <xmmintrin.h>

struct V3 
{
	V3() : x(0), y(0), z(0) { };
	V3(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z) { };

	float x;
	float y;
	float z;
};

void Add(const V3& a, const V3& b, V3& out);
float Dot(const V3& a, const V3& b);
void Multiply(const V3& a, const float& b, V3& out);
void Normalize(V3 &out);
void Subtract(const V3& a, const V3& b, V3& out);
float InvSqrt(float x);

__m128 DotSSE(const __m128 &ax, const __m128 &ay, const __m128 &az, const __m128 &bx, const __m128 &by, const __m128 &bz);
__m128 LengthSSE(const __m128 &ax, const __m128 &ay, const __m128 &az);
void NormalizeSSE(__m128 &x, __m128 &y, __m128 &z);
void MultiplySSE(const __m128* xyzc, __m128* xyz);

void ReflectSSE(const __m128 &vx, const __m128 &vy, const __m128 &vz, 
				  const __m128 &nx, const __m128 &ny, const __m128 &nz,
				  __m128 &ox, __m128 &oy, __m128 &oz);

