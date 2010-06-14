package com.goeswhere.tracer;

class RTSphere
{
public:

	RTSphere() : radius(0), diffuse(1), specular(0), reflection(0), colour(1,1,1)
	{
		radiusSq = _mm_set1_ps(0);
	};

	RTSphere(const V3& _position, const float _radius, const SSERGB& _colour) :
				position(_position), colour(_colour), radius(_radius), diffuse(1), specular(0), reflection(0)
	{
		radiusSq = _mm_set1_ps(_radius * _radius);
	};

	V3& GetPosition() { return position; }
	SSERGB& GetColour() { return colour; }
	float GetRadius() { return radius; }
	float GetDiffuse() { return diffuse; }
	float GetSpecular() { return specular; }
	float GetReflection() { return reflection; }

	void SetPosition(V3& _position) { position = _position; }
	void SetColour(SSERGB& _colour) { colour = _colour; }
	void SetRadius(float _radius) { radius = _radius; radiusSq = _mm_set1_ps(radius * radius); }
	void SetDiffuse(float _diffuse) { diffuse = _diffuse; }
	void SetSpecular(float _specular) { specular = _specular; }
	void SetReflection(float _reflection) { reflection = _reflection; }

	SSEFloat IntersectTest(const Ray& ray) const;
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
