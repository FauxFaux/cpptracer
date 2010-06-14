#pragma once

struct RTLight 
{
	RTLight() : position(0, 0, 0), power(1) {}
	RTLight(const V3& _position, const float _power) : position(_position), power(_power) { };

	V3 position;
	float power;
};
