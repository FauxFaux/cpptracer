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

const SSEFloat sseOne = aj_set1_ps(1.0f);
const SSEFloat sseZero = aj_setzero_ps();
const SSEFloat sseTrue = aj_cmpeq_ps(sseZero, sseZero);
const SSEFloat sseMiss = aj_set1_ps(0xFFFFFFFF);


#ifdef _MSC_VER
# define asFloatArray(x) ((x).m128_f32)
# define asUIntArray(x) ((x).m128_u32)
#else
# define asFloatArray(x) ((float*)(&x))
# define asUIntArray(x) ((unsigned int*)(&x))
#endif


#define XM_CRMASK_CR6TRUE   0x00000080
#define XM_CRMASK_CR6FALSE  0x00000020
#define XMComparisonAnyTrue(CR)  (((CR) & XM_CRMASK_CR6FALSE) != XM_CRMASK_CR6FALSE)
#define XMComparisonAllTrue(CR)  (((CR) & XM_CRMASK_CR6TRUE) == XM_CRMASK_CR6TRUE)

static SSEFloat Select(SSEFloat v1, SSEFloat v2, SSEFloat control)
{
	SSEFloat vTemp1 = aj_andnot_ps(control, v1);
	SSEFloat vTemp2 = aj_and_ps(v2, control);
	return aj_or_ps(vTemp1, vTemp2);
}

static unsigned int MaskToUInt(SSEFloat mask)
{
	unsigned int CR = 0;
	int iTest = aj_movemask_ps(mask);
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

static bool AnyComponentGreaterThanZero(SSEFloat v1)
{
	SSEFloat mask = aj_cmpgt_ps(v1, aj_setzero_ps());

	return XMComparisonAnyTrue(MaskToUInt(mask));
}

static bool AllComponentGreaterEqualThanZero(SSEFloat v1)
{
	SSEFloat mask = aj_cmpge_ps(v1, aj_setzero_ps());

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
	return aj_add_ps(aj_add_ps(aj_mul_ps(ax, bx), aj_mul_ps(ay, by)), aj_mul_ps(az, bz));
}

SSEFloat LengthSSE(const SSEFloat &ax, const SSEFloat &ay, const SSEFloat &az)
{
	return aj_sqrt_ps(aj_add_ps(aj_add_ps(aj_mul_ps(ax, ax), aj_mul_ps(ay, ay)), aj_mul_ps(az, az)));
}

void Multiply(const V3 &a, const float &b, V3 &out)
{
	out.x = a.x * b;
	out.y = a.y * b;
	out.z = a.z * b;
}

void MultiplySSE(const SSEFloat *xyzc, SSEFloat *xyz)
{
	xyz[0] = aj_mul_ps(xyzc[0], xyzc[3]);
	xyz[1] = aj_mul_ps(xyzc[1], xyzc[3]);
	xyz[2] = aj_mul_ps(xyzc[2], xyzc[3]);
}

void NormalizeSSE(SSEFloat &x, SSEFloat &y, SSEFloat &z)
{
	SSEFloat oneOverLength = aj_rsqrt_ps(aj_add_ps(aj_add_ps(aj_mul_ps(x, x), aj_mul_ps(y, y)), aj_mul_ps(z, z)));

	x = aj_mul_ps(x, oneOverLength);
	y = aj_mul_ps(y, oneOverLength);
	z = aj_mul_ps(z, oneOverLength);
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
			aj_mul_ps(aj_add_ps(aj_add_ps(aj_mul_ps(rayDirX, normalX), aj_mul_ps(rayDirY, normalY)),
								aj_mul_ps(rayDirZ, normalZ)), aj_set1_ps(2));

	reflectedX = aj_sub_ps(reflectedX, aj_mul_ps(numByTwo, normalX));
	reflectedY = aj_sub_ps(reflectedY, aj_mul_ps(numByTwo, normalY));
	reflectedZ = aj_sub_ps(reflectedZ, aj_mul_ps(numByTwo, normalZ));
}

void Subtract(const V3 &a, const V3 &b, V3 &out)
{
	out.x = a.x - b.x;
	out.y = a.y - b.y;
	out.z = a.z - b.z;
}

static SSEFloat SetFromUInt(unsigned int x)
{
	SSEInt V = aj_set1_epi32(x);
	return reinterpret_cast<SSEFloat *>(&V)[0];
}

static SSEFloat SetFromUIntPtr(unsigned int *p)
{
	SSEInt V = aj_loadu_si((const SSEInt *) p);
	return reinterpret_cast<SSEFloat *>(&V)[0];
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

		SSEFloat gtZeroMask = aj_cmpgt_ps(distance, sseZero);
		SSEFloat ltNearestObs = aj_cmplt_ps(distance, nearestObstruction);
		SSEFloat mask = aj_and_ps(gtZeroMask, ltNearestObs);
		nearestObstruction = Select(nearestObstruction, distance, mask);
	}

	return nearestObstruction;
}


void raytrace(SSERGB &colour, const Ray &rays, const int iteration, const int w, const int h)
{
	if (iteration > 10)
		return;

	SSEFloat isTracingMask = aj_cmpneq_ps(rays.positionX, sseMiss);
	SSEFloat sseNearest = sseTrue;

	unsigned int uniqueSpheres = 0;

	// Set the nearest intersection to as large as possible.
	SSEFloat nearestDistance = aj_set1_ps(std::numeric_limits<float>().max());

	// For every sphere in the scene see if the ray intersects it.
	for (unsigned int s = 0; s < numSpheres; s++)
	{
		// Intersect the packet of rays with the sphere and have the distance to
		// the intersection point returned.
		RTSphere &sphere = spheres[s];
		SSEFloat distance = sphere.IntersectTest(rays);

		SSEFloat distGTZeroMask = aj_cmpgt_ps(distance, sseZero);
		SSEFloat distLTNearestDistMask = aj_cmplt_ps(distance, nearestDistance);
		SSEFloat mask = aj_and_ps(distGTZeroMask, distLTNearestDistMask);

		sseNearest = Select(sseNearest, SetFromUInt(s), mask);
		nearestDistance = Select(nearestDistance, distance, mask);
	}

	sseNearest = Select(sseTrue, sseNearest, isTracingMask);
	AJ_ALIGN unsigned int nearest[batch];
	aj_store_ps((float *) nearest, sseNearest);

	AJ_ALIGN unsigned int spheresHit[batch] = SPHERES_HIT_INITIALISER;

	// NO idea how to sse this.
	for (int n = 0; n < batch; n++)
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
		SSEFloat nearestMask = aj_cmpeq_ps(sseNearest, sseSI);

		// Calculate the distance to intersection point minus the small amount
		// to avoid the intersection point actually intersecting the sphere.
		SSEFloat mult = aj_mul_ps(nearestDistance, aj_set1_ps(1.0f - EPSILON));

		// The normalised ray vector multiplied by the distance to the
		// intersection point, less the small "nearly 1" multiplier - 'mult'.
		SSEFloat extraX = aj_mul_ps(rays.directionX, mult);
		SSEFloat extraY = aj_mul_ps(rays.directionY, mult);
		SSEFloat extraZ = aj_mul_ps(rays.directionZ, mult);

		// The intersection point is the ray's initial position plus the ray's
		// direction, after being multiplied by mult.
		SSEFloat intPointX = aj_add_ps(extraX, rays.positionX);
		SSEFloat intPointY = aj_add_ps(extraY, rays.positionY);
		SSEFloat intPointZ = aj_add_ps(extraZ, rays.positionZ);

		// For every light in the scene, we need to check if it is casting
		// light onto an object for both diffuse and specular lighting.
		for (unsigned int i = 0; i < numLights; i++)
		{
			RTLight &light = lights[i];

			// The i'th light's position.
			SSEFloat lightPosX = aj_set1_ps(light.position.x);
			SSEFloat lightPosY = aj_set1_ps(light.position.y);
			SSEFloat lightPosZ = aj_set1_ps(light.position.z);

			// The i'th light's direction.
			SSEFloat lightDirX = aj_sub_ps(lightPosX, intPointX);
			SSEFloat lightDirY = aj_sub_ps(lightPosY, intPointY);
			SSEFloat lightDirZ = aj_sub_ps(lightPosZ, intPointZ);

			SSEFloat distanceToLight = LengthSSE(lightDirX, lightDirY, lightDirZ);

			// Normalise the light direction.
			NormalizeSSE(lightDirX, lightDirY, lightDirZ);

			const V3 &spherePosition = sphere.GetPosition();

			// Calculate the normal at the intersection point, for a sphere this is
			// simply the intersection point - sphere centre, normalised.
			SSEFloat sphereNormalX = aj_sub_ps(intPointX, aj_set1_ps(spherePosition.x));
			SSEFloat sphereNormalY = aj_sub_ps(intPointY, aj_set1_ps(spherePosition.y));
			SSEFloat sphereNormalZ = aj_sub_ps(intPointZ, aj_set1_ps(spherePosition.z));

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

				SSEFloat sseRF = aj_set1_ps(reflectionFactor);

				SSEFloat newRed = aj_mul_ps(aj_mul_ps(colour.red, sseRF), sphereColour.red);
				SSEFloat newGreen = aj_mul_ps(aj_mul_ps(colour.green, sseRF), sphereColour.green);
				SSEFloat newBlue = aj_mul_ps(aj_mul_ps(colour.blue, sseRF), sphereColour.blue);

				colour.red = Select(colour.red, newRed, nearestMask);
				colour.green = Select(colour.green, newGreen, nearestMask);
				colour.blue = Select(colour.blue, newBlue, nearestMask);
			}

			// Calculate dot product for Diffuse Lighting.
			const SSEFloat dotProduct = DotSSE(sphereNormalX, sphereNormalY, sphereNormalZ,
											   lightDirX, lightDirY, lightDirZ);

			SSEFloat dpgtZeroMask = aj_cmpgt_ps(dotProduct, aj_setzero_ps());

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
				SSEFloat validHitMask = aj_and_ps(nearestMask, dpgtZeroMask);

				SSEFloat obstructedMask = aj_cmplt_ps(nearestObstruction, distanceToLight);
				SSEFloat shade = Select(sseOne, aj_setzero_ps(), obstructedMask);
				SSEFloat shadeGTZeroMask = aj_cmpgt_ps(shade, aj_setzero_ps());

				validHitMask = aj_and_ps(validHitMask, shadeGTZeroMask);

				SSEFloat lightPower = aj_set1_ps(light.power);
				SSEFloat sphereDiffuse = aj_set1_ps(sphere.GetDiffuse());

				SSEFloat newRed = aj_add_ps(aj_mul_ps(
						aj_mul_ps(aj_mul_ps(aj_mul_ps(sphereColour.red, dotProduct), lightPower), sphereDiffuse),
						shade), colour.red);
				SSEFloat newGreen = aj_add_ps(aj_mul_ps(
						aj_mul_ps(aj_mul_ps(aj_mul_ps(sphereColour.green, dotProduct), lightPower), sphereDiffuse),
						shade), colour.green);
				SSEFloat newBlue = aj_add_ps(aj_mul_ps(
						aj_mul_ps(aj_mul_ps(aj_mul_ps(sphereColour.blue, dotProduct), lightPower), sphereDiffuse),
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
				SSEFloat toLightVectorX = aj_sub_ps(lightPosX, intPointX);
				SSEFloat toLightVectorY = aj_sub_ps(lightPosY, intPointY);
				SSEFloat toLightVectorZ = aj_sub_ps(lightPosZ, intPointZ);

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

				for (int r = 0; r < batch; r++)
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
	SSEFloat pixelWidth = aj_set1_ps(viewportWidth / width);
	SSEFloat pixelHeight = aj_set1_ps(viewportHeight / height);

	// Constants used in calculating each ray's direction.
	SSEFloat halfX = aj_set1_ps((width - 1) / 2);
	SSEFloat halfY = aj_set1_ps((height - 1) / 2);

	// A packet of four rays used for the SSE version.
	Ray rayPacket;

	SSEFloat batch_indexes = aj_set_offsets();
	SSEFloat twoFiftyFive = aj_set1_ps(255.0f);

	SSERGB colourPacket(0, 0, 0);

	// Scanning across in rows from the top
	for (unsigned int y = threadID; y < height; y += numThreads)
	{
		SSEFloat sseY = aj_set1_ps(y);

		// Four pixels at a time.
		for (unsigned int x = 0; x < width; x += batch)
		{
			SSEFloat sseX = aj_set1_ps(x);

			rayPacket.directionX = aj_mul_ps(aj_sub_ps(aj_add_ps(sseX, batch_indexes), halfX), pixelWidth);
			rayPacket.directionY = aj_sub_ps(aj_setzero_ps(), aj_mul_ps(aj_sub_ps(sseY, halfY), pixelHeight));

			rayPacket.directionZ = aj_set1_ps(defaultNearClip);
			rayPacket.positionX = aj_setzero_ps();
			rayPacket.positionY = aj_setzero_ps();
			rayPacket.positionZ = aj_setzero_ps();

			NormalizeSSE(rayPacket.directionX, rayPacket.directionY, rayPacket.directionZ);

			colourPacket.red = aj_setzero_ps();
			colourPacket.green = aj_setzero_ps();
			colourPacket.blue = aj_setzero_ps();

			// Raytrace the packet of four rays
			raytrace(colourPacket, rayPacket, 0, x, y);

			colourPacket.red = aj_mul_ps(colourPacket.red, twoFiftyFive);
			colourPacket.green = aj_mul_ps(colourPacket.green, twoFiftyFive);
			colourPacket.blue = aj_mul_ps(colourPacket.blue, twoFiftyFive);

			colourPacket.red = aj_min_ps(colourPacket.red, twoFiftyFive);
			colourPacket.green = aj_min_ps(colourPacket.green, twoFiftyFive);
			colourPacket.blue = aj_min_ps(colourPacket.blue, twoFiftyFive);

			AJRGB *pixelPtr = pixelData + (x + y * width);

			for (unsigned int i = 0; i < batch; i++)
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
	SSEFloat normalX = aj_sub_ps(intPointX, aj_set1_ps(position.x));
	SSEFloat normalY = aj_sub_ps(intPointY, aj_set1_ps(position.y));
	SSEFloat normalZ = aj_sub_ps(intPointZ, aj_set1_ps(position.z));

	// Normalise the sphere normal.
	NormalizeSSE(normalX, normalY, normalZ);

	// Reflect the ray rayDir in normal and store in reflected.
	ReflectSSE(rayDirX, rayDirY, rayDirZ, normalX, normalY, normalZ, reflectedX, reflectedY, reflectedZ);

	// Normalise the reflected ray.
	NormalizeSSE(reflectedX, reflectedY, reflectedZ);
}

SSEFloat minusOne = aj_set1_ps(-1);
SSEFloat two = aj_set1_ps(2);

// An SSE-optimised version of an already optimised ray-sphere intersection
// algorithm. Also taken from PixelMachine at www.superjer.com.
// The variable names are poor but they are in the quadratic formula too.
SSEFloat RTSphere::IntersectTest(const Ray &rays) const
{
	SSEFloat t = minusOne;

	SSEFloat sPosX = aj_set1_ps(position.x);
	SSEFloat sPosY = aj_set1_ps(position.y);
	SSEFloat sPosZ = aj_set1_ps(position.z);

	SSEFloat ox = aj_sub_ps(rays.positionX, sPosX);
	SSEFloat oy = aj_sub_ps(rays.positionY, sPosY);
	SSEFloat oz = aj_sub_ps(rays.positionZ, sPosZ);

	SSEFloat rDirXS = aj_mul_ps(rays.directionX, rays.directionX);
	SSEFloat rDirYS = aj_mul_ps(rays.directionY, rays.directionY);
	SSEFloat rDirZS = aj_mul_ps(rays.directionZ, rays.directionZ);

	SSEFloat a = aj_mul_ps(aj_add_ps(aj_add_ps(rDirXS, rDirYS), rDirZS), two);

	SSEFloat rDirOx = aj_mul_ps(rays.directionX, ox);
	SSEFloat rDirOy = aj_mul_ps(rays.directionY, oy);
	SSEFloat rDirOz = aj_mul_ps(rays.directionZ, oz);

	SSEFloat b = aj_add_ps(rDirOx, rDirOy);
	b = aj_add_ps(b, rDirOz);
	b = aj_mul_ps(b, two);

	ox = aj_mul_ps(ox, ox);
	oy = aj_mul_ps(oy, oy);
	oz = aj_mul_ps(oz, oz);

	SSEFloat c = aj_add_ps(ox, oy);
	c = aj_add_ps(c, oz);
	c = aj_sub_ps(c, radiusSq);

	SSEFloat twoac = aj_mul_ps(aj_mul_ps(a, c), two);

	SSEFloat d = aj_sub_ps(aj_mul_ps(b, b), twoac);

	SSEFloat answerUnknownMask = aj_cmpge_ps(d, aj_setzero_ps());

	SSEFloat one = aj_set1_ps(1);
	a = aj_div_ps(one, a);
	d = aj_sqrt_ps(d);

	SSEFloat newerT = aj_mul_ps(aj_sub_ps(aj_sub_ps(aj_setzero_ps(), d), b), a);
	t = Select(t, newerT, answerUnknownMask);

	answerUnknownMask = aj_cmplt_ps(t, aj_setzero_ps());

	newerT = aj_mul_ps(aj_sub_ps(d, b), a);
	return Select(t, newerT, answerUnknownMask);
}
