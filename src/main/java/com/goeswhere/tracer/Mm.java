package com.goeswhere.tracer;

public class Mm {
	public static SSEFloat _mm_and_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				i2f(f2i(a.r0) & f2i(b.r0)),
				i2f(f2i(a.r1) & f2i(b.r1)),
				i2f(f2i(a.r2) & f2i(b.r2)),
				i2f(f2i(a.r3) & f2i(b.r3)));
	}

	static SSEFloat _mm_add_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				a.r0 + b.r0,
				a.r1 + b.r1,
				a.r2 + b.r2,
				a.r3 + b.r3);
	}

	static SSEFloat _mm_cmpgt_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				(a.r0 > b.r0) ? 0xffffffff : 0x0,
				(a.r1 > b.r1) ? 0xffffffff : 0x0,
				(a.r2 > b.r2) ? 0xffffffff : 0x0,
				(a.r3 > b.r3) ? 0xffffffff : 0x0);
	}

	static SSEFloat _mm_cmplt_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				(a.r0 < b.r0) ? 0xffffffff : 0x0,
				(a.r1 < b.r1) ? 0xffffffff : 0x0,
				(a.r2 < b.r2) ? 0xffffffff : 0x0,
				(a.r3 < b.r3) ? 0xffffffff : 0x0);
	}

	static SSEFloat _mm_sub_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				a.r0 - b.r0,
				a.r1 - b.r1,
				a.r2 - b.r2,
				a.r3 - b.r3);
	}

	static SSEFloat _mm_set1_ps(float power) {
		return new SSEFloat(power, power, power, power);
	}

	static SSEFloat _mm_setzero_ps() {
		return new SSEFloat();
	}

	static SSEFloat _mm_mul_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				a.r0 * b.r0,
				a.r1 * b.r1,
				a.r2 * b.r2,
				a.r3 * b.r3);
	}

	static SSEFloat _mm_andnot_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				i2f(~f2i(a.r0) & f2i(b.r0)),
				i2f(~f2i(a.r1) & f2i(b.r1)),
				i2f(~f2i(a.r2) & f2i(b.r2)),
				i2f(~f2i(a.r3) & f2i(b.r3)));
	}

	private static float i2f(int i) {
		return Float.intBitsToFloat(i);
	}

	private static int f2i(float r0) {
		return Float.floatToIntBits(r0);
	}

	static SSEFloat _mm_cmpeq_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				(a.r0 == b.r0) ? 0xffffffff : 0x0,
				(a.r1 == b.r1) ? 0xffffffff : 0x0,
				(a.r2 == b.r2) ? 0xffffffff : 0x0,
				(a.r3 == b.r3) ? 0xffffffff : 0x0);
	}

	static SSEFloat _mm_cmpge_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				(a.r0 >= b.r0) ? 0xffffffff : 0x0,
				(a.r1 >= b.r1) ? 0xffffffff : 0x0,
				(a.r2 >= b.r2) ? 0xffffffff : 0x0,
				(a.r3 >= b.r3) ? 0xffffffff : 0x0);
	}

	static SSEFloat _mm_mul_ps(Object _mm_mul_ps, SSEFloat lightPower) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_cmpneq_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				(a.r0 != b.r0) ? 0xffffffff : 0x0,
				(a.r1 != b.r1) ? 0xffffffff : 0x0,
				(a.r2 != b.r2) ? 0xffffffff : 0x0,
				(a.r3 != b.r3) ? 0xffffffff : 0x0);
	}

	static __m128i _mm_set1_epi32(int x) {
		return new __m128i(x);
	}

	static __m128i _mm_loadu_si128(__m128i __m128i) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_setr_ps(int i, int j, int k, int l) {
		return new SSEFloat(l, k, j, i);
	}

	static SSEFloat _mm_min_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				Math.min(a.r0, b.r0),
				Math.min(a.r1, b.r1),
				Math.min(a.r2, b.r2),
				Math.min(a.r3, b.r3));
	}

	static SSEFloat _mm_rsqrt_ps(SSEFloat a) {
		return new SSEFloat(
				V3.InvSqrt(a.r0),
				V3.InvSqrt(a.r1),
				V3.InvSqrt(a.r2),
				V3.InvSqrt(a.r3));
	}

	static void _mm_store_ps(int[] p, SSEFloat a) {
		p[0] = (int) a.r0;
		p[1] = (int) a.r1;
		p[2] = (int) a.r2;
		p[3] = (int) a.r3;
	}

	static SSEFloat _mm_set_ps1(float maxValue) {
		return _mm_set1_ps(maxValue);
	}

	static SSEFloat _mm_sqrt_ps(SSEFloat a) {
		return new SSEFloat(
				sqrt(a.r0),
				sqrt(a.r1),
				sqrt(a.r2),
				sqrt(a.r3));
	}

	private static float sqrt(float r0) {
		return (float) Math.sqrt(r0);
	}

	static SSEFloat _mm_div_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				a.r0 / b.r0,
				a.r1 / b.r1,
				a.r2 / b.r2,
				a.r3 / b.r3);
	}

	static SSEFloat _mm_or_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				i2f(f2i(a.r0) | f2i(b.r0)),
				i2f(f2i(a.r1) | f2i(b.r1)),
				i2f(f2i(a.r2) | f2i(b.r2)),
				i2f(f2i(a.r3) | f2i(b.r3)));
	}

	static int _mm_movemask_ps(SSEFloat mask) {
		return sign(mask.r3) << 3 | sign(mask.r2) << 2 | sign(mask.r1) << 1 | sign(mask.r0);
	}

	private static int sign(final float f) {
		return (f > 0 ? 1 : 0);
	}
}
