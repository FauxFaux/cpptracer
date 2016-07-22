#pragma once

#include "sse.h"

struct V3
{
	V3() : x(0), y(0), z(0)
	{};

	V3(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z)
	{};

	float x;
	float y;
	float z;
};

struct SSERGB
{
	SSERGB(float r, float g, float b)
	{
		red = aj_set1_ps(r);
		green = aj_set1_ps(g);
		blue = aj_set1_ps(b);
	}

	SSEFloat red, green, blue;
};

struct Ray
{
	SSEFloat positionX, positionY, positionZ;
	SSEFloat directionX, directionY, directionZ;
};


class RTObject
{
public:
	RTObject(void)
	{};
public:
	~RTObject(void)
	{};
};

class RTSphere
{
public:

	RTSphere() : RTSphere(V3(), 0, SSERGB(1, 1, 1))
	{};

	RTSphere(const V3 &_position, const float _radius, const SSERGB &_colour,
			 float _specular = 0, float _reflection = 0, float _diffuse = 1) :
			position(_position), colour(_colour), radius(_radius),
			diffuse(_diffuse), specular(_specular), reflection(_reflection), radiusSq(aj_set1_ps(_radius * _radius))
	{};
	
	const V3 &GetPosition()
	{ return position; }

	const SSERGB &GetColour()
	{ return colour; }

	float GetRadius()
	{ return radius; }

	float GetDiffuse()
	{ return diffuse; }

	float GetSpecular()
	{ return specular; }

	float GetReflection()
	{ return reflection; }

	SSEFloat IntersectTest(const Ray &ray) const;

	void ReflectRayAtPoint(const SSEFloat &rayDirx, const SSEFloat &rayDirY, const SSEFloat &rayDirZ,
						   const SSEFloat &intPointX, const SSEFloat &intPointY, const SSEFloat &intPointZ,
						   SSEFloat &reflectedX, SSEFloat &reflectedY, SSEFloat &reflectedZ) const;

protected:
	V3 position;
	SSERGB colour;
	float radius;
	float diffuse;
	float specular;
	float reflection;
	SSEFloat radiusSq;
};

struct RTLight
{
	RTLight() : position(0, 0, 0), power(1)
	{}

	RTLight(const V3 &_position, const float _power) : position(_position), power(_power)
	{};

	V3 position;
	float power;
};
