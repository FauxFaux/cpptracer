#pragma once

#include "v3.h"
#include "pixel.h"

class RTSphere 
{
public:

	RTSphere() : radius(0), diffuse(1), specular(0), reflection(0) { };
	RTSphere(const V3& _position, const float _radius, const RGBA& _colour) : 
				position(_position), colour(_colour), radius(_radius), diffuse(1), specular(0), reflection(0) { };

	V3& GetPosition() { return position; }
	RGBA& GetColour() { return colour; }
	float GetRadius() { return radius; }
	float GetDiffuse() { return diffuse; }
	float GetSpecular() { return specular; }
	float GetReflection() { return reflection; }

	void SetPosition(V3& _position) { position = _position; }
	void SetColour(RGBA& _colour) { colour = _colour; }
	void SetRadius(float _radius) { radius = _radius; }
	void SetDiffuse(float _diffuse) { diffuse = _diffuse; }
	void SetSpecular(float _specular) { specular = _specular; }
	void SetReflection(float _reflection) { reflection = _reflection; }

	SSEInt IntersectTest(const Ray& ray) const;
	void ReflectRayAtPoint(const SSEInt &rayDirx, const SSEInt &rayDirY, const SSEInt &rayDirZ, 
								const SSEInt &intPointX, const SSEInt &intPointY, const SSEInt &intPointZ,
								SSEInt &reflectedX, SSEInt &reflectedY, SSEInt &reflectedZ) const;

protected:
	V3 position;
	RGBA colour;
	float radius;
	float diffuse;
	float specular;
	float reflection;
};
