package com.goeswhere.tracer;

public class RtLight
{
	RtLight() { this(new V3(0, 0, 0), (1)); }
	RtLight(V3 _position, float _power) { position = (_position); power = (_power); }

	V3 position;
	float power;
}
