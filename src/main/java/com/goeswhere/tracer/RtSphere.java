package com.goeswhere.tracer;

import static com.goeswhere.tracer.Mm.*;
import static com.goeswhere.tracer.Stdafx.Select;

class RtSphere {

	 V3 GetPosition() { return position; }
	 SSERGB GetColour() { return colour; }
	 float GetRadius() { return radius; }
	 float GetDiffuse() { return diffuse; }
	 float GetSpecular() { return specular; }
	 float GetReflection() { return reflection; }

	 void SetPosition(V3 _position) { position = _position; }
	 void SetColour(SSERGB _colour) { colour = _colour; }
	 void SetRadius(float _radius) { radius = _radius; radiusSq = _mm_set1_ps(radius * radius); }
	 void SetDiffuse(float _diffuse) { diffuse = _diffuse; }
	 void SetSpecular(float _specular) { specular = _specular; }
	 void SetReflection(float _reflection) { reflection = _reflection; }

	RtSphere() { radius = (0); diffuse = (1); specular = (0); reflection = (0); colour = new SSERGB(1,1,1);
		radiusSq = _mm_set1_ps(0);
	}

	RtSphere(V3 _position, float _radius, SSERGB _colour) {
				position=(_position); colour=(_colour); radius=(_radius); diffuse=(1); specular=(0); reflection=(0);
		radiusSq = _mm_set1_ps(_radius * _radius);
	}

	V3 position;
	SSERGB colour;
	float radius;
	float diffuse;
	float specular;
	float reflection;
	SSEFloat radiusSq;

	// At a given point in the world, reflect a ray off the sphere's normal to that point.
	void ReflectRayAtPoint(SSEFloat rayDirX, SSEFloat rayDirY, SSEFloat rayDirZ,
										SSEFloat intPointX, SSEFloat intPointY, SSEFloat intPointZ,
										SSEFloat reflectedX, SSEFloat reflectedY, SSEFloat reflectedZ)
	{
		// Calculate the sphere normal at this point;
		SSEFloat normalX = _mm_sub_ps(intPointX, _mm_set_ps1(position.x));
		SSEFloat normalY = _mm_sub_ps(intPointY, _mm_set_ps1(position.y));
		SSEFloat normalZ = _mm_sub_ps(intPointZ, _mm_set_ps1(position.z));

		// Normalise the sphere normal.
		NormalizeSSE(normalX, normalY, normalZ);

		// Reflect the ray rayDir in normal and store in reflected.
		ReflectSSE(rayDirX, rayDirY, rayDirZ, normalX, normalY, normalZ, reflectedX, reflectedY, reflectedZ);

		// Normalise the reflected ray.
		NormalizeSSE(reflectedX, reflectedY, reflectedZ);
	}

	SSEFloat minusOne = _mm_set_ps1(-1);
	SSEFloat two = _mm_set_ps1(2);

	// An SSE-optimised version of an already optimised ray-sphere intersection
	// algorithm. Also taken from PixelMachine at www.superjer.com.
	// The variable names are poor but they are in the quadratic formula too.
	SSEFloat IntersectTest(Ray rays)
	{
		SSEFloat t = minusOne;

		SSEFloat sPosX = _mm_set_ps1(position.x);
		SSEFloat sPosY = _mm_set_ps1(position.y);
		SSEFloat sPosZ = _mm_set_ps1(position.z);

		SSEFloat ox = _mm_sub_ps(rays.positionX, sPosX);
		SSEFloat oy = _mm_sub_ps(rays.positionY, sPosY);
		SSEFloat oz = _mm_sub_ps(rays.positionZ, sPosZ);

		SSEFloat rDirXS = _mm_mul_ps(rays.directionX, rays.directionX);
		SSEFloat rDirYS = _mm_mul_ps(rays.directionY, rays.directionY);
		SSEFloat rDirZS = _mm_mul_ps(rays.directionZ, rays.directionZ);

		SSEFloat a = _mm_mul_ps(_mm_add_ps(_mm_add_ps(rDirXS, rDirYS), rDirZS), two);

		SSEFloat rDirOx = _mm_mul_ps(rays.directionX, ox);
		SSEFloat rDirOy = _mm_mul_ps(rays.directionY, oy);
		SSEFloat rDirOz = _mm_mul_ps(rays.directionZ, oz);

		SSEFloat b = _mm_add_ps(rDirOx, rDirOy);
		b = _mm_add_ps(b, rDirOz);
		b = _mm_mul_ps(b, two);

		ox = _mm_mul_ps(ox, ox);
		oy = _mm_mul_ps(oy, oy);
		oz = _mm_mul_ps(oz, oz);

		SSEFloat c = _mm_add_ps(ox, oy);
		c = _mm_add_ps(c, oz);
		c = _mm_sub_ps(c, radiusSq);

		SSEFloat twoac = _mm_mul_ps(_mm_mul_ps(a, c), two);

		SSEFloat d = _mm_sub_ps(_mm_mul_ps(b, b), twoac);

		SSEFloat answerUnknownMask = _mm_cmpge_ps(d, _mm_setzero_ps());

		SSEFloat one = _mm_set_ps1(1);
		a = _mm_div_ps(one, a);
		d = _mm_sqrt_ps(d);

		SSEFloat newerT = _mm_mul_ps(_mm_sub_ps(_mm_sub_ps(_mm_setzero_ps(), d), b), a);
		t = Select(t, newerT, answerUnknownMask);

		answerUnknownMask = _mm_cmplt_ps(t, _mm_setzero_ps());

		newerT = _mm_mul_ps(_mm_sub_ps(d, b), a);
		return Select(t, newerT, answerUnknownMask);
	}
}