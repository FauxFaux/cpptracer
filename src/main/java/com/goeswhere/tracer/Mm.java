package com.goeswhere.tracer;

public class Mm {
	public static SSEFloat _mm_and_ps(SSEFloat validHitMask, SSEFloat shadeGTZeroMask) {
		return null;
	}

	static SSEFloat _mm_add_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				a.get0() + b.get0(),
				a.get1() + b.get1(),
				a.get2() + b.get2(),
				a.get3() + b.get3());
	}

	static SSEFloat _mm_cmpgt_ps(SSEFloat distance, SSEFloat zero2) {
		return null;
	}

	static SSEFloat _mm_cmplt_ps(SSEFloat distance, SSEFloat nearestObstruction) {
		return null;
	}

	static SSEFloat _mm_sub_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				a.get0() - b.get0(),
				a.get1() - b.get1(),
				a.get2() - b.get2(),
				a.get3() + b.get3());
	}

	static SSEFloat _mm_set1_ps(float power) {
		return new SSEFloat(power, power, power, power);
	}

	static SSEFloat _mm_setzero_ps() {
		return new SSEFloat();
	}

	static SSEFloat _mm_mul_ps(SSEFloat a, SSEFloat b) {
		return new SSEFloat(
				a.get0() * b.get0(),
				a.get1() * b.get1(),
				a.get2() * b.get2(),
				a.get3() * b.get3());
	}

	static SSEFloat _mm_andnot_ps(SSEFloat control, SSEFloat v1) {
		return null;
	}

	static SSEFloat _mm_cmpeq_ps(SSEFloat sseNearest, SSEFloat sseSI) {
		return null;
	}

	static SSEFloat _mm_cmpge_ps(SSEFloat d, SSEFloat _mm_setzero_ps) {
		return null;
	}

	static SSEFloat _mm_mul_ps(Object _mm_mul_ps, SSEFloat lightPower) {
		return null;
	}

	static SSEFloat _mm_cmpneq_ps(SSEFloat positionX, SSEFloat miss2) {
		return null;
	}

	static __m128i _mm_set1_epi32(int x) {
		return null;
	}

	static __m128i _mm_loadu_si128(__m128i __m128i) {
		return null;
	}

	static SSEFloat _mm_setr_ps(int i, int j, int k, int l) {
		return new SSEFloat(l, k, j, i);
	}

	static SSEFloat _mm_min_ps(SSEFloat red, SSEFloat twoFiftyFive) {
		return null;
	}

	static SSEFloat _mm_rsqrt_ps(SSEFloat a) {
		return new SSEFloat(
				V3.InvSqrt(a.get0()),
				V3.InvSqrt(a.get1()),
				V3.InvSqrt(a.get2()),
				V3.InvSqrt(a.get3()));
	}

	static void _mm_store_ps(int[] nearest, SSEFloat sseNearest) {
	}

	static SSEFloat _mm_set_ps1(float maxValue) {
		return null;
	}

	static SSEFloat _mm_sqrt_ps(SSEFloat _mm_add_ps) {
		return null;
	}

	static SSEFloat _mm_div_ps(SSEFloat one, SSEFloat a) {
		return null;
	}

	static SSEFloat _mm_or_ps(SSEFloat vTemp1, SSEFloat vTemp2) {
		return null;
	}

	static int _mm_movemask_ps(SSEFloat mask) {
		return 0;
	}
}
