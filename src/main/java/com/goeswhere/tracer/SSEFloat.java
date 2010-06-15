package com.goeswhere.tracer;

public class SSEFloat {

	float r0, r1, r2, r3;

	public SSEFloat(__m128i v) {
		r0 = v.r0;
		r1 = v.r1;
		r2 = v.r2;
		r3 = v.r3;
	}

	public SSEFloat() {
		this(new __m128i(0));
	}

	public SSEFloat(float r0, float r1, float r2, float r3) {
		this.r0 = r0;
		this.r1 = r1;
		this.r2 = r2;
		this.r3 = r3;
	}

	public float r(int r) {
		switch (r) {
		case 0: return r0;
		case 1: return r1;
		case 2: return r2;
		case 3: return r3;
		}
		throw new IllegalArgumentException();
	}

	public void plusequal(int r, float f) {
		switch (r) {
		case 0: r0 += f; return;
		case 1: r1 += f; return;
		case 2: r2 += f; return;
		case 3: r3 += f; return;
		}
		throw new IllegalArgumentException();
	}
}
