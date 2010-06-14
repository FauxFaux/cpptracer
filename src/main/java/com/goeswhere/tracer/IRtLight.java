package com.goeswhere.tracer;

class RTLight
{
	RTLight() : position(0, 0, 0), power(1) {}
	RTLight(V3& _position, float _power) : position(_position), power(_power) { }

	V3 position;
	float power;
}
