package com.goeswhere.tracer;

import static com.goeswhere.tracer.Mm._mm_add_ps;
import static com.goeswhere.tracer.Mm._mm_mul_ps;
import static com.goeswhere.tracer.Mm._mm_rsqrt_ps;
import static com.goeswhere.tracer.Mm._mm_set_ps1;
import static com.goeswhere.tracer.Mm._mm_sqrt_ps;
import static com.goeswhere.tracer.Mm._mm_sub_ps;

class V3
{
	V3() { this(0, 0, 0); }
	V3(float _x, float _y, float _z) { x = (_x); y = (_y); z = (_z); }

	float x;
	float y;
	float z;

	void Add(V3 a, V3 b, V3 out)
	{
		out.x = a.x + b.x;
		out.y = a.y + b.y;
		out.z = a.z + b.z;
	}

	float Dot(V3 a, V3 b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	static SSEFloat DotSSE(SSEFloat ax, SSEFloat ay, SSEFloat az,
				  SSEFloat bx, SSEFloat by, SSEFloat bz)
	{
		return _mm_add_ps(_mm_add_ps(_mm_mul_ps(ax, bx), _mm_mul_ps(ay, by)), _mm_mul_ps(az, bz));
	}

	static SSEFloat LengthSSE(SSEFloat ax, SSEFloat ay, SSEFloat az)
	{
		return _mm_sqrt_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(ax, ax), _mm_mul_ps(ay, ay)), _mm_mul_ps(az, az)));
	}

	void Multiply(V3 a, float b, V3 out)
	{
		out.x = a.x * b;
		out.y = a.y * b;
		out.z = a.z * b;
	}

	void MultiplySSE(SSEFloat[] xyzc, SSEFloat[] xyz)
	{
		xyz[0] = _mm_mul_ps(xyzc[0], xyzc[3]);
		xyz[1] = _mm_mul_ps(xyzc[1], xyzc[3]);
		xyz[2] = _mm_mul_ps(xyzc[2], xyzc[3]);
	}

	static void NormalizeSSE(SSEFloat x, SSEFloat y, SSEFloat z)
	{
		SSEFloat oneOverLength = _mm_rsqrt_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(x, x), _mm_mul_ps(y, y)), _mm_mul_ps(z, z)));

		x = _mm_mul_ps(x, oneOverLength);
		y = _mm_mul_ps(y, oneOverLength);
		z = _mm_mul_ps(z, oneOverLength);
	}

	void Normalize(V3 out)
	{
		float oneOverLength = InvSqrt(out.x * out.x + out.y * out.y + out.z * out.z);

		out.x *= oneOverLength;
		out.y *= oneOverLength;
		out.z *= oneOverLength;
	}

	// The formula for reflecting a vector in a normal.
	static void ReflectSSE(SSEFloat rayDirX, SSEFloat rayDirY, SSEFloat rayDirZ,
					SSEFloat normalX, SSEFloat normalY, SSEFloat normalZ,
					SSEFloat reflectedX, SSEFloat reflectedY, SSEFloat reflectedZ)
	{
		reflectedX = rayDirX;
		reflectedY = rayDirY;
		reflectedZ = rayDirZ;

		SSEFloat numByTwo =
			_mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(rayDirX, normalX), _mm_mul_ps(rayDirY, normalY)), _mm_mul_ps(rayDirZ, normalZ)), _mm_set_ps1(2));

		reflectedX = _mm_sub_ps(reflectedX, _mm_mul_ps(numByTwo, normalX));
		reflectedY = _mm_sub_ps(reflectedY, _mm_mul_ps(numByTwo, normalY));
		reflectedZ = _mm_sub_ps(reflectedZ, _mm_mul_ps(numByTwo, normalZ));
	}

	void Subtract(V3 a, V3 b, V3 out)
	{
		out.x = a.x - b.x;
		out.y = a.y - b.y;
		out.z = a.z - b.z;
	}

	static float InvSqrt(float x)
	{
		return (float) (1.0/Math.sqrt(x));
	}
}
