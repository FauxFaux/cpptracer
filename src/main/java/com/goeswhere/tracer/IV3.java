package com.goeswhere.tracer;

class V3
{
	V3() { this(0, 0, 0); }
	V3(float _x, float _y, float _z) { x = (_x); y = (_y); z = (_z); }

	float x;
	float y;
	float z;
}

interface IV3Extra {
	void Add(V3& a, V3& b, V3& out);
	float Dot(V3& a, V3& b);
	void Multiply(V3& a, float& b, V3& out);
	void Normalize(V3 &out);
	void Subtract(V3& a, V3& b, V3& out);
	float InvSqrt(float x);

	__m128 DotSSE(__m128 &ax, __m128 &ay, __m128 &az, __m128 &bx, __m128 &by, __m128 &bz);
	__m128 LengthSSE(__m128 &ax, __m128 &ay, __m128 &az);
	void NormalizeSSE(__m128 &x, __m128 &y, __m128 &z);
	void MultiplySSE(__m128* xyzc, __m128* xyz);

	void ReflectSSE(__m128 &vx, __m128 &vy, __m128 &vz,
					  __m128 &nx, __m128 &ny, __m128 &nz,
					  __m128 &ox, __m128 &oy, __m128 &oz);
}
