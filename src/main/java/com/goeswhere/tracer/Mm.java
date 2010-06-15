package com.goeswhere.tracer;

public class Mm {
	public static SSEFloat _mm_and_ps(SSEFloat validHitMask, SSEFloat shadeGTZeroMask) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_add_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				a.r0 + b.r0,
				a.r1 + b.r1,
				a.r2 + b.r2,
				a.r3 + b.r3);
	}

	static SSEFloat _mm_cmpgt_ps(SSEFloat distance, SSEFloat zero2) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_cmplt_ps(SSEFloat distance, SSEFloat nearestObstruction) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_sub_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				a.r0 - b.r0,
				a.r1 - b.r1,
				a.r2 - b.r2,
				a.r3 + b.r3);
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

	static SSEFloat _mm_andnot_ps(SSEFloat control, SSEFloat v1) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_cmpeq_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				(a.r0 == b.r0) ? 0xffffffff : 0x0,
				(a.r1 == b.r1) ? 0xffffffff : 0x0,
				(a.r2 == b.r2) ? 0xffffffff : 0x0,
				(a.r3 == b.r3) ? 0xffffffff : 0x0);
	}

	static SSEFloat _mm_cmpge_ps(SSEFloat d, SSEFloat _mm_setzero_ps) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_mul_ps(Object _mm_mul_ps, SSEFloat lightPower) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_cmpneq_ps(SSEFloat positionX, SSEFloat miss2) {
		throw new UnsupportedOperationException();
	}

	static __m128i _mm_set1_epi32(int x) {
		throw new UnsupportedOperationException();
	}

	static __m128i _mm_loadu_si128(__m128i __m128i) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_setr_ps(int i, int j, int k, int l) {
		return new SSEFloat(l, k, j, i);
	}

	static SSEFloat _mm_min_ps(SSEFloat red, SSEFloat twoFiftyFive) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_rsqrt_ps(SSEFloat a) {
		return new SSEFloat(
				V3.InvSqrt(a.r0),
				V3.InvSqrt(a.r1),
				V3.InvSqrt(a.r2),
				V3.InvSqrt(a.r3));
	}

	static void _mm_store_ps(int[] nearest, SSEFloat sseNearest) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_set_ps1(float maxValue) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_sqrt_ps(SSEFloat _mm_add_ps) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_div_ps(SSEFloat one, SSEFloat a) {
		throw new UnsupportedOperationException();
	}

	static SSEFloat _mm_or_ps(SSEFloat vTemp1, SSEFloat vTemp2) {
		throw new UnsupportedOperationException();
	}

	static int _mm_movemask_ps(SSEFloat mask) {
		throw new UnsupportedOperationException();
	}
}
