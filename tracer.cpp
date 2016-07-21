#include <thread>
#include <boost/ptr_container/ptr_vector.hpp>

#include "objects.h"
#include "tracer.h"

using std::thread;
using std::bind;

const float EPSILON = 0.001f;
const float defaultViewportWidth = 0.1f;
const float defaultNearClip = 0.1f;

RTSphere spheres[10];
unsigned int numSpheres;

RTLight lights[10];
unsigned int numLights;

const SSEFloat sseOne = _mm_set1_ps(1.0f);
const SSEFloat sseZero = _mm_setzero_ps();
const SSEFloat sseTrue = _mm_cmpeq_ps(sseZero, sseZero);
const SSEFloat sseMiss = _mm_set1_ps(0xFFFFFFFF);


#ifdef _MSC_VER
# define asFloatArray(x) ((x).m128_f32)
# define asUIntArray(x) ((x).m128_u32)
#else
# define asFloatArray(x) ((float*)(&x))
# define asUIntArray(x) ((unsigned int*)(&x))
#endif

#ifdef __GNUC__
# define ALIGN16 __attribute__ ((aligned (16)))
#else
# define ALIGN16 __declspec(align(16))
#endif

#define XM_CRMASK_CR6TRUE   0x00000080
#define XM_CRMASK_CR6FALSE  0x00000020
#define XMComparisonAnyTrue(CR)  (((CR) & XM_CRMASK_CR6FALSE) != XM_CRMASK_CR6FALSE)
#define XMComparisonAllTrue(CR)  (((CR) & XM_CRMASK_CR6TRUE) == XM_CRMASK_CR6TRUE)

inline SSEFloat Select(SSEFloat v1, SSEFloat v2, SSEFloat control)
{
	SSEFloat vTemp1 = _mm_andnot_ps(control, v1);
	SSEFloat vTemp2 = _mm_and_ps(v2, control);
	return _mm_or_ps(vTemp1, vTemp2);
}

inline unsigned int MaskToUInt(SSEFloat mask)
{
	unsigned int CR = 0;
	int iTest = _mm_movemask_ps(mask);
	if (iTest == 0xf)
	{
		CR = XM_CRMASK_CR6TRUE;
	}
	else if (!iTest)
	{
		CR = XM_CRMASK_CR6FALSE;
	}

	return CR;
}

inline bool AnyComponentGreaterThanZero(SSEFloat v1)
{
	SSEFloat mask = _mm_cmpgt_ps(v1, _mm_setzero_ps());

	return XMComparisonAnyTrue(MaskToUInt(mask));
}

inline bool AllComponentGreaterEqualThanZero(SSEFloat v1)
{
	SSEFloat mask = _mm_cmpge_ps(v1, _mm_setzero_ps());

	return XMComparisonAllTrue(MaskToUInt(mask));
}


void Add(const V3 &a, const V3 &b, V3 &out)
{
	out.x = a.x + b.x;
	out.y = a.y + b.y;
	out.z = a.z + b.z;
}

float Dot(const V3 &a, const V3 &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

SSEFloat DotSSE(const SSEFloat &ax, const SSEFloat &ay, const SSEFloat &az,
				const SSEFloat &bx, const SSEFloat &by, const SSEFloat &bz)
{
	return _mm_add_ps(_mm_add_ps(_mm_mul_ps(ax, bx), _mm_mul_ps(ay, by)), _mm_mul_ps(az, bz));
}

SSEFloat LengthSSE(const SSEFloat &ax, const SSEFloat &ay, const SSEFloat &az)
{
	return _mm_sqrt_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(ax, ax), _mm_mul_ps(ay, ay)), _mm_mul_ps(az, az)));
}

void Multiply(const V3 &a, const float &b, V3 &out)
{
	out.x = a.x * b;
	out.y = a.y * b;
	out.z = a.z * b;
}

void MultiplySSE(const SSEFloat *xyzc, SSEFloat *xyz)
{
	xyz[0] = _mm_mul_ps(xyzc[0], xyzc[3]);
	xyz[1] = _mm_mul_ps(xyzc[1], xyzc[3]);
	xyz[2] = _mm_mul_ps(xyzc[2], xyzc[3]);
}

void NormalizeSSE(SSEFloat &x, SSEFloat &y, SSEFloat &z)
{
	SSEFloat oneOverLength = _mm_rsqrt_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(x, x), _mm_mul_ps(y, y)), _mm_mul_ps(z, z)));

	x = _mm_mul_ps(x, oneOverLength);
	y = _mm_mul_ps(y, oneOverLength);
	z = _mm_mul_ps(z, oneOverLength);
}

// The formula for reflecting a vector in a normal.
void ReflectSSE(const SSEFloat &rayDirX, const SSEFloat &rayDirY, const SSEFloat &rayDirZ,
				const SSEFloat &normalX, const SSEFloat &normalY, const SSEFloat &normalZ,
				SSEFloat &reflectedX, SSEFloat &reflectedY, SSEFloat &reflectedZ)
{
	reflectedX = rayDirX;
	reflectedY = rayDirY;
	reflectedZ = rayDirZ;

	SSEFloat numByTwo =
			_mm_mul_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(rayDirX, normalX), _mm_mul_ps(rayDirY, normalY)),
								  _mm_mul_ps(rayDirZ, normalZ)), _mm_set_ps1(2));

	reflectedX = _mm_sub_ps(reflectedX, _mm_mul_ps(numByTwo, normalX));
	reflectedY = _mm_sub_ps(reflectedY, _mm_mul_ps(numByTwo, normalY));
	reflectedZ = _mm_sub_ps(reflectedZ, _mm_mul_ps(numByTwo, normalZ));
}

void Subtract(const V3 &a, const V3 &b, V3 &out)
{
	out.x = a.x - b.x;
	out.y = a.y - b.y;
	out.z = a.z - b.z;
}

inline SSEFloat SetFromUInt(unsigned int x)
{
	__m128i V = _mm_set1_epi32(x);
	return reinterpret_cast<__m128 *>(&V)[0];
}

inline SSEFloat SetFromUIntPtr(unsigned int *p)
{
	__m128i V = _mm_loadu_si128((const __m128i *) p);
	return reinterpret_cast<__m128 *>(&V)[0];
}

void setupScene()
{
	// Just some code to put a sphere and a light into the scene, ideally
	// this would be read in from some kind of script / scene file rather
	// than being hard-coded :)

	spheres[0] = RTSphere(V3(0, 0, 1.75f), 0.15f, SSERGB(1, 1, 1), 0.25f, 0.9f, 0.25f);
	spheres[1] = RTSphere(V3(-0.3f, 0, 1.75f), 0.15f, SSERGB(1, 1, 0), 0.25f, 0.25f);
	spheres[2] = RTSphere(V3(0.3f, 0, 1.75f), 0.15f, SSERGB(0, 0, 1), 0.25f, 0.25f);
	spheres[3] = RTSphere(V3(0, 0.3f, 1.75f), 0.15f, SSERGB(1, 0, 0), 0.25f, 0.25f);
	spheres[4] = RTSphere(V3(0, -0.3f, 1.75f), 0.15f, SSERGB(0, 1, 0), 0.25f, 0.25f);

	numSpheres = 5;

	//RTSphere sphere6(V3(-0.3f, 0.3f, 1.75f), 0.15f, AJRGB(255, 0, 255));
	//sphere6.SetSpecular(0.25f); sphere6.SetReflection(0.25f);
	//spheres.push_back(sphere6);

	//RTSphere sphere7(V3(0.3f, 0.3f, 1.75f), 0.15f, AJRGB(0, 255, 255));
	//sphere7.SetSpecular(0.25f); sphere7.SetReflection(0.25f);
	//spheres.push_back(sphere7);

	//RTSphere sphere8(V3(0.3f, -0.3f, 1.75f), 0.15f, AJRGB(255, 255, 255));
	//sphere8.SetSpecular(0.25f); sphere8.SetReflection(0.25f);
	//spheres.push_back(sphere8);

	//RTSphere sphere9(V3(-0.3f, -0.3f, 1.75f), 0.15f, AJRGB(0, 0, 0));
	//sphere9.SetSpecular(0.25f); sphere9.SetReflection(0.25f);
	//spheres.push_back(sphere9);

	lights[0] = RTLight(V3(0, 0, 1), 1);

	numLights = 1;
	// End of scene data.
}

SSEFloat getNearestObstruction(const Ray &rays)
{
	SSEFloat nearestObstruction = sseMiss;

	for (unsigned int s = 0; s < numSpheres; s++)
	{
		RTSphere &sphere = spheres[s];
		SSEFloat distance = sphere.IntersectTest(rays);

		SSEFloat gtZeroMask = _mm_cmpgt_ps(distance, sseZero);
		SSEFloat ltNearestObs = _mm_cmplt_ps(distance, nearestObstruction);
		SSEFloat mask = _mm_and_ps(gtZeroMask, ltNearestObs);
		nearestObstruction = Select(nearestObstruction, distance, mask);
	}

	return nearestObstruction;
}


void raytrace(SSERGB &colour, const Ray &rays, const int iteration, const int w, const int h)
{
	if (iteration > 10)
		return;

	SSEFloat isTracingMask = _mm_cmpneq_ps(rays.positionX, sseMiss);
	SSEFloat sseNearest = sseTrue;

	unsigned int uniqueSpheres = 0;

	// Set the nearest intersection to as large as possible.
	SSEFloat nearestDistance = _mm_set_ps1(std::numeric_limits<float>().max());

	// For every sphere in the scene see if the ray intersects it.
	for (unsigned int s = 0; s < numSpheres; s++)
	{
		// Intersect the packet of rays with the sphere and have the distance to
		// the intersection point returned.
		RTSphere &sphere = spheres[s];
		SSEFloat distance = sphere.IntersectTest(rays);

		SSEFloat distGTZeroMask = _mm_cmpgt_ps(distance, sseZero);
		SSEFloat distLTNearestDistMask = _mm_cmplt_ps(distance, nearestDistance);
		SSEFloat mask = _mm_and_ps(distGTZeroMask, distLTNearestDistMask);

		sseNearest = Select(sseNearest, SetFromUInt(s), mask);
		nearestDistance = Select(nearestDistance, distance, mask);
	}

	sseNearest = Select(sseTrue, sseNearest, isTracingMask);
	ALIGN16 unsigned int nearest[4];
	_mm_store_ps((float *) nearest, sseNearest);

	ALIGN16 unsigned int spheresHit[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

	// NO idea how to sse this.
	for (int n = 0; n < 4; n++)
	{
		if (nearest[n] != 0xFFFFFFFF)
		{
			bool alreadyIn = false;

			for (int s = 0; s < n; s++)
				if (spheresHit[s] == nearest[n])
				{
					alreadyIn = true;
					continue;
				}

			if (!alreadyIn)
				spheresHit[uniqueSpheres++] = nearest[n];
		}
	}

	for (unsigned int sh = 0; sh < uniqueSpheres; sh++)
	{
		unsigned int sphereIndex = spheresHit[sh];
		RTSphere &sphere = spheres[sphereIndex];    // The sphere to be tested.

		SSEFloat sseSI = SetFromUInt(sphereIndex);
		SSEFloat nearestMask = _mm_cmpeq_ps(sseNearest, sseSI);

		// Calculate the distance to intersection point minus the small amount
		// to avoid the intersection point actually intersecting the sphere.
		SSEFloat mult = _mm_mul_ps(nearestDistance, _mm_set_ps1(1.0f - EPSILON));

		// The normalised ray vector multiplied by the distance to the
		// intersection point, less the small "nearly 1" multiplier - 'mult'.
		SSEFloat extraX = _mm_mul_ps(rays.directionX, mult);
		SSEFloat extraY = _mm_mul_ps(rays.directionY, mult);
		SSEFloat extraZ = _mm_mul_ps(rays.directionZ, mult);

		// The intersection point is the ray's initial position plus the ray's
		// direction, after being multiplied by mult.
		SSEFloat intPointX = _mm_add_ps(extraX, rays.positionX);
		SSEFloat intPointY = _mm_add_ps(extraY, rays.positionY);
		SSEFloat intPointZ = _mm_add_ps(extraZ, rays.positionZ);

		// For every light in the scene, we need to check if it is casting
		// light onto an object for both diffuse and specular lighting.
		for (unsigned int i = 0; i < numLights; i++)
		{
			RTLight &light = lights[i];

			// The i'th light's position.
			SSEFloat lightPosX = _mm_set_ps1(light.position.x);
			SSEFloat lightPosY = _mm_set_ps1(light.position.y);
			SSEFloat lightPosZ = _mm_set_ps1(light.position.z);

			// The i'th light's direction.
			SSEFloat lightDirX = _mm_sub_ps(lightPosX, intPointX);
			SSEFloat lightDirY = _mm_sub_ps(lightPosY, intPointY);
			SSEFloat lightDirZ = _mm_sub_ps(lightPosZ, intPointZ);

			SSEFloat distanceToLight = LengthSSE(lightDirX, lightDirY, lightDirZ);

			// Normalise the light direction.
			NormalizeSSE(lightDirX, lightDirY, lightDirZ);

			const V3 &spherePosition = sphere.GetPosition();

			// Calculate the normal at the intersection point, for a sphere this is
			// simply the intersection point - sphere centre, normalised.
			SSEFloat sphereNormalX = _mm_sub_ps(intPointX, _mm_set_ps1(spherePosition.x));
			SSEFloat sphereNormalY = _mm_sub_ps(intPointY, _mm_set_ps1(spherePosition.y));
			SSEFloat sphereNormalZ = _mm_sub_ps(intPointZ, _mm_set_ps1(spherePosition.z));

			NormalizeSSE(sphereNormalX, sphereNormalY, sphereNormalZ);

			const SSERGB &sphereColour = sphere.GetColour();
			float reflectionFactor = sphere.GetReflection();

			if (reflectionFactor > 0)
			{
				SSEFloat reflectionX, reflectionY, reflectionZ;

				ReflectSSE(rays.directionX, rays.directionY, rays.directionZ,
						   sphereNormalX, sphereNormalY, sphereNormalZ,
						   reflectionX, reflectionY, reflectionZ);

				NormalizeSSE(reflectionX, reflectionY, reflectionZ);

				Ray reflectedPacket;

				reflectedPacket.directionX = reflectionX;
				reflectedPacket.directionY = reflectionY;
				reflectedPacket.directionZ = reflectionZ;
				reflectedPacket.positionX = Select(sseMiss, intPointX, nearestMask);
				reflectedPacket.positionY = intPointY;
				reflectedPacket.positionZ = intPointZ;

				raytrace(colour, reflectedPacket, iteration + 1, w, h);

				SSEFloat sseRF = _mm_set1_ps(reflectionFactor);

				SSEFloat newRed = _mm_mul_ps(_mm_mul_ps(colour.red, sseRF), sphereColour.red);
				SSEFloat newGreen = _mm_mul_ps(_mm_mul_ps(colour.green, sseRF), sphereColour.green);
				SSEFloat newBlue = _mm_mul_ps(_mm_mul_ps(colour.blue, sseRF), sphereColour.blue);

				colour.red = Select(colour.red, newRed, nearestMask);
				colour.green = Select(colour.green, newGreen, nearestMask);
				colour.blue = Select(colour.blue, newBlue, nearestMask);
			}

			// Calculate dot product for Diffuse Lighting.
			const SSEFloat dotProduct = DotSSE(sphereNormalX, sphereNormalY, sphereNormalZ,
											   lightDirX, lightDirY, lightDirZ);

			SSEFloat dpgtZeroMask = _mm_cmpgt_ps(dotProduct, _mm_setzero_ps());

			if (AnyComponentGreaterThanZero(dotProduct))
			{
				Ray obstructionPacket;
				obstructionPacket.directionX = lightDirX;
				obstructionPacket.directionY = lightDirY;
				obstructionPacket.directionZ = lightDirZ;
				obstructionPacket.positionX = intPointX;
				obstructionPacket.positionY = intPointY;
				obstructionPacket.positionZ = intPointZ;

				SSEFloat nearestObstruction = getNearestObstruction(obstructionPacket);
				SSEFloat validHitMask = _mm_and_ps(nearestMask, dpgtZeroMask);

				SSEFloat obstructedMask = _mm_cmplt_ps(nearestObstruction, distanceToLight);
				SSEFloat shade = Select(sseOne, _mm_setzero_ps(), obstructedMask);
				SSEFloat shadeGTZeroMask = _mm_cmpgt_ps(shade, _mm_setzero_ps());

				validHitMask = _mm_and_ps(validHitMask, shadeGTZeroMask);

				SSEFloat lightPower = _mm_set1_ps(light.power);
				SSEFloat sphereDiffuse = _mm_set1_ps(sphere.GetDiffuse());

				SSEFloat newRed = _mm_add_ps(_mm_mul_ps(
						_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(sphereColour.red, dotProduct), lightPower), sphereDiffuse),
						shade), colour.red);
				SSEFloat newGreen = _mm_add_ps(_mm_mul_ps(
						_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(sphereColour.green, dotProduct), lightPower), sphereDiffuse),
						shade), colour.green);
				SSEFloat newBlue = _mm_add_ps(_mm_mul_ps(
						_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(sphereColour.blue, dotProduct), lightPower), sphereDiffuse),
						shade), colour.blue);

				colour.red = Select(colour.red, newRed, validHitMask);
				colour.green = Select(colour.green, newGreen, validHitMask);
				colour.blue = Select(colour.blue, newBlue, validHitMask);
			}

			// We only need calculate the specular lighting component for this
			// sphere if it has a specular value greater than zero.
			if (sphere.GetSpecular() > 0)
			{
				// Calculate the vector from intersection point back to the light.
				SSEFloat toLightVectorX = _mm_sub_ps(lightPosX, intPointX);
				SSEFloat toLightVectorY = _mm_sub_ps(lightPosY, intPointY);
				SSEFloat toLightVectorZ = _mm_sub_ps(lightPosZ, intPointZ);

				NormalizeSSE(toLightVectorX, toLightVectorY, toLightVectorZ);

				// Calculate the vector along which light will be reflected off the
				// surface of the sphere and store it in reflectionX/Y/Z.
				SSEFloat reflectionX, reflectionY, reflectionZ;
				sphere.ReflectRayAtPoint(toLightVectorX, toLightVectorY, toLightVectorZ,
										 intPointX, intPointY, intPointZ,
										 reflectionX, reflectionY, reflectionZ);

				// Calculate the specular dot product
				const SSEFloat specDP = DotSSE(rays.directionX, rays.directionY, rays.directionZ,
											   reflectionX, reflectionY, reflectionZ);

				for (int r = 0; r < 4; r++)
				{
					// If this ray is hitting this sphere, and it's dot product > 0
					if (nearest[r] == sphereIndex && asFloatArray(specDP)[r] > 0)
					{
						const float specular = powf(asFloatArray(specDP)[r], 10) * sphere.GetSpecular();

						//SSEFloat sseSpecular
						asFloatArray(colour.red)[r] += specular;
						asFloatArray(colour.green)[r] += specular;
						asFloatArray(colour.blue)[r] += specular;
					}
				}
			}
		}
	}
}

void render(AJRGB *pixelData,
			const uint32_t width, const uint32_t height,
			const uint32_t threadID,
			const uint32_t numThreads)
{
	// Calculate the height of the viewport depending on its width and the aspect
	// ratio of the image.
	const float viewportWidth = defaultViewportWidth;
	const float viewportHeight = viewportWidth / ((float) width / height);

	// Calculate the width and height of a pixel, normally square.
	SSEFloat pixelWidth = _mm_set1_ps(viewportWidth / width);
	SSEFloat pixelHeight = _mm_set1_ps(viewportHeight / height);

	// Constants used in calculating each ray's direction.
	SSEFloat halfX = _mm_set1_ps((width - 1) / 2);
	SSEFloat halfY = _mm_set1_ps((height - 1) / 2);

	// A packet of four rays used for the SSE version.
	Ray rayPacket;

	SSEFloat a = _mm_setr_ps(0, 1, 2, 3);
	SSEFloat twoFiftyFive = _mm_set1_ps(255.0f);

	SSERGB colourPacket(0, 0, 0);

	// Scanning across in rows from the top
	for (unsigned int y = threadID; y < height; y += numThreads)
	{
		SSEFloat sseY = _mm_set1_ps(y);

		// Four pixels at a time.
		for (unsigned int x = 0; x < width; x += 4)
		{
			SSEFloat sseX = _mm_set1_ps(x);

			rayPacket.directionX = _mm_mul_ps(_mm_sub_ps(_mm_add_ps(sseX, a), halfX), pixelWidth);
			rayPacket.directionY = _mm_sub_ps(_mm_setzero_ps(), _mm_mul_ps(_mm_sub_ps(sseY, halfY), pixelHeight));

			rayPacket.directionZ = _mm_set1_ps(defaultNearClip);
			rayPacket.positionX = _mm_setzero_ps();
			rayPacket.positionY = _mm_setzero_ps();
			rayPacket.positionZ = _mm_setzero_ps();

			NormalizeSSE(rayPacket.directionX, rayPacket.directionY, rayPacket.directionZ);

			colourPacket.red = _mm_setzero_ps();
			colourPacket.green = _mm_setzero_ps();
			colourPacket.blue = _mm_setzero_ps();

			// Raytrace the packet of four rays
			raytrace(colourPacket, rayPacket, 0, x, y);

			colourPacket.red = _mm_mul_ps(colourPacket.red, twoFiftyFive);
			colourPacket.green = _mm_mul_ps(colourPacket.green, twoFiftyFive);
			colourPacket.blue = _mm_mul_ps(colourPacket.blue, twoFiftyFive);

			colourPacket.red = _mm_min_ps(colourPacket.red, twoFiftyFive);
			colourPacket.green = _mm_min_ps(colourPacket.green, twoFiftyFive);
			colourPacket.blue = _mm_min_ps(colourPacket.blue, twoFiftyFive);

			AJRGB *pixelPtr = pixelData + (x + y * width);

			for (unsigned int i = 0; i < 4; i++)
			{
				pixelPtr[i].red = (uint8_t) asFloatArray(colourPacket.red)[i];
				pixelPtr[i].green = (uint8_t) asFloatArray(colourPacket.green)[i];
				pixelPtr[i].blue = (uint8_t) asFloatArray(colourPacket.blue)[i];
			}
		}
	}
}

void startRender(AJRGB *pixelData, const int width, const int height, int numThreads)
{
	if (numThreads < 1)
		numThreads = 1;

#if defined(USEOPENMP)
	omp_set_num_threads(numThreads);
#pragma omp parallel for
		for(int t = 0; t < numThreads; ++t)
		{
			render(pixelData, width, height, t * (height / numThreads), (height / numThreads));
		}
#else
	boost::ptr_vector<thread> threads;

	for (int t = 0; t < numThreads; ++t)
		threads.push_back(new thread(std::bind(&render, pixelData, width, height, t, numThreads)));

	for (std::thread &t : threads)
		t.join();
#endif
}


// At a given point in the world, reflect a ray off the sphere's normal to that point.
void RTSphere::ReflectRayAtPoint(const SSEFloat &rayDirX, const SSEFloat &rayDirY, const SSEFloat &rayDirZ,
								 const SSEFloat &intPointX, const SSEFloat &intPointY, const SSEFloat &intPointZ,
								 SSEFloat &reflectedX, SSEFloat &reflectedY, SSEFloat &reflectedZ) const
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
SSEFloat RTSphere::IntersectTest(const Ray &rays) const
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
