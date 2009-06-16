// A C++ Raytracer written by Adam Miles.

#include "stdafx.h"
#include "intersectioninfo.h"
#include "pixel.h"
#include "rtsphere.h"
#include "rtlight.h"


#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/foreach.hpp>

using namespace boost;

using namespace std;

const float EPSILON = 0.001f;
const float defaultViewportWidth = 0.1f;
const float defaultNearClip = 0.1f;
const int defaultThreads = 1;
const int maxThreads = 8;

const int defaultScreenWidth = 1280;
const int defaultScreenHeight = 720;

const int bytesInBitmapHeader = 54;

vector<RTSphere> spheres;
vector<RTLight> lights;

void render(RGBA* pixelData, const int screenWidth, const int screenHeight, const int from, const int numRows);
SSEInt getNearestObstruction(const Ray& rays);
void raytrace(RGBA* pixelData, const Ray& rays, const int iteration, const int w, const int h);
void raytraceNonSSE(RGBA &p, const Ray &ray);
void setupScene();
RGBA* startRender(const int width, const int height, int numThreads);
void writeBitmap(RGBA* pixelData, const int screenWidth, const int screenHeight);

#ifndef _WINDOWS
# include <ctime>
#else
# include <windows.h>
#endif

struct timer
{
#ifndef _WINDOWS
	clock_t start;
	timer() : start(clock())
	{
	}

	~timer()
	{
		std::cout << static_cast<float>(clock()-start)/CLOCKS_PER_SEC << std::endl;	
	}
#else
#	error no timer
#endif
};

int main(int argc, char *argv[])
{	
	int width = 0;
	int height = 0;
	int numThreads = 0;

	if(argc > 1)
		width = atoi(argv[1]);

	if(argc > 2)
		height = atoi(argv[2]);

	if(argc > 3)
		numThreads = atoi(argv[3]);

	// If no resolution specified, use defaults.
	if(width == 0 || height == 0)
	{
		width = defaultScreenWidth;
		height = defaultScreenHeight;
	}

	if(numThreads == 0)
		numThreads = defaultThreads;

	setupScene();

	// Render the image
	//RGBA* pixelData = startRender(width, height, numThreads);

	for (int i=1; i<=height; ++i)
		if (height%i == 0)
		{
			timer t;
			delete[] startRender(width, height, i);
			cout << setw(4) << right << i << ": ";
		}

	// Write image to disk
	//writeBitmap(pixelData, width, height);

//	delete [] pixelData;

	return 0;
}

void setupScene()
{
	// Just some code to put a sphere and a light into the scene, ideally
	// this would be read in from some kind of script / scene file rather
	// than being hard-coded :)

	RTSphere sphere(V3(0, 0, 1.75f), 0.15f, RGBA(255, 255, 255));
	sphere.SetSpecular(0.25f); sphere.SetReflection(0.9f); sphere.SetDiffuse(0.25f);
	spheres.push_back(sphere);

	RTSphere sphere2(V3(-0.3f, 0, 1.75f), 0.15f, RGBA(255, 255, 0));
	sphere2.SetSpecular(0.25f); sphere2.SetReflection(0.25f);
	spheres.push_back(sphere2);

	RTSphere sphere3(V3(0.3f, 0, 1.75f), 0.15f, RGBA(0, 0, 255));
	sphere3.SetSpecular(0.25f); sphere3.SetReflection(0.25f);
	spheres.push_back(sphere3);

	RTSphere sphere4(V3(0, 0.3f, 1.75f), 0.15f, RGBA(255, 0, 0));
	sphere4.SetSpecular(0.25f); sphere4.SetReflection(0.25f);
	spheres.push_back(sphere4);

	RTSphere sphere5(V3(0, -0.3f, 1.75f), 0.15f, RGBA(0, 255, 0));
	sphere5.SetSpecular(0.25f); sphere5.SetReflection(0.25f);
	spheres.push_back(sphere5);

	//RTSphere sphere6(V3(-0.3f, 0.3f, 1.75f), 0.15f, RGBA(255, 0, 255));
	//sphere6.SetSpecular(0.25f); sphere6.SetReflection(0.25f);
	//spheres.push_back(sphere6);

	//RTSphere sphere7(V3(0.3f, 0.3f, 1.75f), 0.15f, RGBA(0, 255, 255));
	//sphere7.SetSpecular(0.25f); sphere7.SetReflection(0.25f);
	//spheres.push_back(sphere7);

	//RTSphere sphere8(V3(0.3f, -0.3f, 1.75f), 0.15f, RGBA(255, 255, 255));
	//sphere8.SetSpecular(0.25f); sphere8.SetReflection(0.25f);
	//spheres.push_back(sphere8);

	//RTSphere sphere9(V3(-0.3f, -0.3f, 1.75f), 0.15f, RGBA(0, 0, 0));
	//sphere9.SetSpecular(0.25f); sphere9.SetReflection(0.25f);
	//spheres.push_back(sphere9);

	RTLight light(V3(0, 0, 1), 1);
	lights.push_back(light);
	// End of scene data.
}

RGBA* startRender(const int width, const int height, int numThreads)
{
	RGBA* pixelData = new RGBA[width * height];
 
	if(numThreads < 1)
		numThreads = 1;
 
	//cout << "Using " << numThreads << " threads\n";
 
	ptr_vector<thread> threads;
 
	for(int t = 0; t < numThreads; ++t)
		threads.push_back(new thread(bind(&render, pixelData, width, height, t * (height / numThreads), (height / numThreads))));
 
	BOOST_FOREACH(thread& t, threads)
		t.join();
 
	return pixelData;
}


void render(RGBA* pixelData, const int width, const int height, const int from, const int numRows)
{
	// Calculate the height of the viewport depending on its width and the aspect
	// ratio of the image.
	const float viewportWidth = defaultViewportWidth;
	const float viewportHeight = viewportWidth / ((float)width / height);

	// Calculate the width and height of a pixel, normally square.
	const float pixelWidth = viewportWidth / width;
	const float pixelHeight = viewportHeight / height;

	// Constants used in calculating each ray's direction.
	const float halfX = (width - 1.0f) / 2;
	const float halfY = (height - 1.0f) / 2;

	// A packet of four rays used for the SSE version.
	Ray rayPacket;

	// Scanning across in rows from the top
	for(int y = from; y < from + numRows; y++)
	{
		// Four pixels at a time.
		for(int x = 0; x < width; x+=4)
		{
			// Position in the pixelData array for the first ray in the packet.
			int pixelNum = y * width + x;

			for(int a = 0; a < 4; a++)
			{
				asFloatArray(rayPacket.directionX)[a] = (x + a - halfX) * pixelWidth;
				asFloatArray(rayPacket.directionY)[a] = -((y - halfY) * pixelHeight);				
			}

			rayPacket.directionZ = _mm_set1_ps(defaultNearClip);
			rayPacket.positionX = _mm_setzero_ps();
			rayPacket.positionY = _mm_setzero_ps();
			rayPacket.positionZ = _mm_setzero_ps();

			NormalizeSSE(rayPacket.directionX, rayPacket.directionY, rayPacket.directionZ);

			// Raytrace the packet of four rays
			raytrace(pixelData + pixelNum, rayPacket, 0, x, y);
		}
	}
}

//int hitLimit = 0;
//int maxBounces = 0;
//int maxX = 0;
//int maxY = 0;

#ifdef _WINDOWS
# define asFloatArray(x) ((x).m128_f32)
#else
# define asFloatArray(x) ((float*)(&x))
#endif

void raytrace(RGBA* pixelData, const Ray& rays, const int iteration, const int w, const int h)
{
	if(iteration > 10)
		return;

	bool isTracing[4] = { true, true, true, true };

	for(int r = 0; r < 4; r++)
		if(asFloatArray(rays.positionX)[r] == 0xffffffff)
			isTracing[r] = false;

	// Initialisation state of no intersection, -1.
	int nearest[4] = { -1, -1, -1, -1 };

	int uniqueSpheres = 0;
	int spheresHit[4] = { -1, -1, -1, -1 };
	
	// Set the nearest intersection to as large as possible.
	SSEInt nearestDistance = _mm_set_ps1(0xffffffff);

	// Used to store which ray hits which sphere in the vector.
	int sIndex = 0;

	// For every sphere in the scene see if the ray intersects it.
	for(vector<RTSphere>::const_iterator s = spheres.begin(); s != spheres.end(); s++)
	{
		// Intersect the packet of rays with the sphere and have the distance to 
		// the intersection point returned.
		SSEInt distance = s->IntersectTest(rays);

		for(int r = 0; r < 4; r++)
		{
			if(isTracing[r])
			{
				// If the intersection distance is greater than zero and it is nearer
				// than the previous nearest intersection, this is our nearest sphere.
				if(asFloatArray(distance)[r] > 0 && 
					asFloatArray(distance)[r] < asFloatArray(nearestDistance)[r])
				{
					nearest[r] = sIndex;	// Store index of the nearest sphere.
					asFloatArray(nearestDistance)[r] = asFloatArray(distance)[r];
				}
			}
		}

		sIndex++;
	}

	for(int n = 0; n < 4; n++)
	{
		if(nearest[n] != -1)
		{
			bool alreadyIn = false;

			for(int s = 0; s < n; s++)
				if(spheresHit[s] == nearest[n])
				{
					alreadyIn = true;
					continue;
				}

			if(!alreadyIn)
				spheresHit[uniqueSpheres++] = nearest[n];
		}
	}

	for(int sh = 0; sh < uniqueSpheres; sh++)
	{
		int sphereIndex = spheresHit[sh];
		RTSphere& sphere = spheres[sphereIndex];	// The sphere to be tested.
		
		// Calculate the distance to intersection point minus the small amount
		// to avoid the intersection point actually intersecting the sphere.
		SSEInt mult = _mm_mul_ps(nearestDistance, _mm_set_ps1(1.0f - EPSILON));

		// The normalised ray vector multiplied by the distance to the
		// intersection point, less the small "nearly 1" multiplier - 'mult'.
		SSEInt extraX = _mm_mul_ps(rays.directionX, mult);
		SSEInt extraY = _mm_mul_ps(rays.directionY, mult);
		SSEInt extraZ = _mm_mul_ps(rays.directionZ, mult);

		// The intersection point is the ray's initial position plus the ray's
		// direction, after being multiplied by mult.
		SSEInt intPointX = _mm_add_ps(extraX, rays.positionX);
		SSEInt intPointY = _mm_add_ps(extraY, rays.positionY);
		SSEInt intPointZ = _mm_add_ps(extraZ, rays.positionZ);

		// For every light in the scene, we need to check if it is casting
		// light onto an object for both diffuse and specular lighting.
		for (std::vector<RTLight>::size_type i = 0; i != lights.size(); ++i)
		{
			RTLight light = lights[i];

			// The i'th light's position.
			SSEInt lightPosX = _mm_set_ps1(light.position.x);
			SSEInt lightPosY = _mm_set_ps1(light.position.y);
			SSEInt lightPosZ = _mm_set_ps1(light.position.z);

			// The i'th light's direction.
			SSEInt lightDirX = _mm_sub_ps(lightPosX, intPointX);
			SSEInt lightDirY = _mm_sub_ps(lightPosY, intPointY);
			SSEInt lightDirZ = _mm_sub_ps(lightPosZ, intPointZ);

			SSEInt distanceToLight = LengthSSE(lightDirX, lightDirY, lightDirZ);

			// Normalise the light direction.
			NormalizeSSE(lightDirX, lightDirY, lightDirZ);

			V3& spherePosition = sphere.GetPosition();

			// Calculate the normal at the intersection point, for a sphere this is
			// simply the intersection point - sphere centre, normalised.
			SSEInt sphereNormalX = _mm_sub_ps(intPointX, _mm_set_ps1(spherePosition.x));
			SSEInt sphereNormalY = _mm_sub_ps(intPointY, _mm_set_ps1(spherePosition.y));
			SSEInt sphereNormalZ = _mm_sub_ps(intPointZ, _mm_set_ps1(spherePosition.z));

			NormalizeSSE(sphereNormalX, sphereNormalY, sphereNormalZ);

			RGBA& sphereColour = sphere.GetColour();
			float reflectionFactor = sphere.GetReflection();

			if(reflectionFactor > 0)
			{
				SSEInt reflectionX, reflectionY, reflectionZ;

				ReflectSSE(rays.directionX, rays.directionY, rays.directionZ, 
							sphereNormalX, sphereNormalY, sphereNormalZ,
							reflectionX, reflectionY, reflectionZ);

				NormalizeSSE(reflectionX, reflectionY, reflectionZ);

				Ray reflectedPacket;
				
				reflectedPacket.directionX = reflectionX;
				reflectedPacket.directionY = reflectionY;
				reflectedPacket.directionZ = reflectionZ;
				reflectedPacket.positionX = intPointX;
				reflectedPacket.positionY = intPointY;
				reflectedPacket.positionZ = intPointZ;

				for(int r = 0; r < 4; r++)
					if(nearest[r] != sphereIndex)
						asFloatArray(reflectedPacket.positionX)[r] = 0xffffffff;

				raytrace(pixelData, reflectedPacket, iteration + 1, w, h);
				
				for(int r = 0; r < 4; r++)
				{
					if(nearest[r] == sphereIndex)
					{
						pixelData[r].blue *= reflectionFactor;
						pixelData[r].green *= reflectionFactor;
						pixelData[r].red *= reflectionFactor;
					}
				}
			}

			// Calculate dot product for Diffuse Lighting.
			const SSEInt dotProduct = DotSSE(sphereNormalX, sphereNormalY, sphereNormalZ, 
										lightDirX, lightDirY, lightDirZ);

			if(asFloatArray(dotProduct)[0] > 0 || asFloatArray(dotProduct)[1] > 0 || asFloatArray(dotProduct)[2] > 0 || asFloatArray(dotProduct)[3] > 0)
			{
				Ray obstructionPacket;
				obstructionPacket.directionX = lightDirX;
				obstructionPacket.directionY = lightDirY;
				obstructionPacket.directionZ = lightDirZ;
				obstructionPacket.positionX = intPointX;
				obstructionPacket.positionY = intPointY;
				obstructionPacket.positionZ = intPointZ;

				SSEInt nearestObstruction = getNearestObstruction(obstructionPacket);

				// Since not every ray in the packet of four strikes this particular sphere
				// they need to be dealt with seperately right at the end.
				for(int r = 0; r < 4; r++)
				{
					// If this ray hit this sphere, and it's dot product indicates it is lit...
					if(nearest[r] == sphereIndex && asFloatArray(dotProduct)[r] > 0)
					{
						float sphereDiffuse = sphere.GetDiffuse();

						float shade = 1; // Full illumination;

						if(asFloatArray(nearestObstruction)[r] < asFloatArray(distanceToLight)[r])
							shade = 0; // Obstructed.

						if(shade > 0)
						{
							// Set the red, green and blue component in an int so as to avoid
							// overflowing a byte in case of "brighter than white" pixels.
							int red = pixelData[r].red + (sphereColour.red * asFloatArray(dotProduct)[r] * light.power * sphereDiffuse * shade);
							int green = pixelData[r].green + (sphereColour.green * asFloatArray(dotProduct)[r] * light.power * sphereDiffuse * shade);
							int blue = pixelData[r].blue + (sphereColour.blue * asFloatArray(dotProduct)[r] * light.power * sphereDiffuse * shade);

							// Clamp the RGB components back within the 0 - 255 range.
							if(red > 255)
								red = 255;
							if(green > 255)
								green = 255;
							if(blue > 255)
								blue = 255;

							// Finally set the pixel data.
							pixelData[r].red = red; pixelData[r].green = green; pixelData[r].blue = blue;
						}

					}
				}
			}

			// We only need calculate the specular lighting component for this
			// sphere if it has a specular value greater than zero.
			if(sphere.GetSpecular() > 0)
			{
				// No shadowing yet...
				float specularShade = 1;

				// Calculate the vector from intersection point back to the light.
				SSEInt toLightVectorX = _mm_sub_ps(lightPosX, intPointX);
				SSEInt toLightVectorY = _mm_sub_ps(lightPosY, intPointY);
				SSEInt toLightVectorZ = _mm_sub_ps(lightPosZ, intPointZ);

				NormalizeSSE(toLightVectorX, toLightVectorY, toLightVectorZ);

				// Calculate the vector along which light will be reflected off the
				// surface of the sphere and store it in reflectionX/Y/Z.
				SSEInt reflectionX, reflectionY, reflectionZ;
				sphere.ReflectRayAtPoint(toLightVectorX, toLightVectorY, toLightVectorZ, 
											intPointX, intPointY, intPointZ, 
											reflectionX, reflectionY, reflectionZ);

				// Calculate the specular dot product 
				const SSEInt specDP = DotSSE(rays.directionX, rays.directionY, rays.directionZ, 
												reflectionX, reflectionY, reflectionZ);

				for(int r = 0; r < 4; r++)
				{
					// If this ray is hitting this sphere, and it's dot product > 0
					if(nearest[r] == sphereIndex && asFloatArray(specDP)[r] > 0)
					{
						const float specular = pow(asFloatArray(specDP)[r], 10) * sphere.GetSpecular();

						// Calculate the new RGB components, being sure not to overflow.
						int red = pixelData[r].red + (specular * 255);
						int green = pixelData[r].green + (specular * 255);
						int blue = pixelData[r].blue + (specular * 255);

						// Clamp back within 0 - 255 range.
						if(red > 255)
							red = 255;
						if(green > 255)
							green = 255;
						if(blue > 255)
							blue = 255;

						// Set the pixel data with the new values.
						pixelData[r].red = red; pixelData[r].green = green; pixelData[r].blue = blue;
					}
				}
			}
		}
	}
}

SSEInt getNearestObstruction(const Ray& rays)
{
	SSEInt nearestObstruction = _mm_set1_ps(0xffffffff);

	for(vector<RTSphere>::const_iterator s = spheres.begin(); s != spheres.end(); s++)
	{
		SSEInt distance = s->IntersectTest(rays);

		for(int r = 0; r < 4; r++)
			if(asFloatArray(distance)[r] > 0 && asFloatArray(distance)[r] < asFloatArray(nearestObstruction)[r])
				asFloatArray(nearestObstruction)[r] = asFloatArray(distance)[r];
	}

	return nearestObstruction;
}

// Old version/code before performance started becoming an issue and I moved to
// using SSE to send packets of four rays at a time. Very much an equivalent 
// algorithm to as above. The 'old' IntersectTest method has since been removed
// and as such wouldn't compile with the line left in.

/*
void raytraceNonSSE(RGBA &pixel, const Ray &ray)
{
	for(int s = 0; s < spheres.size(); s++)
	{
		RTSphere sphere = spheres[s];

		//float distance = IntersectTest(ray, sphere); // Method since removed. 
		float distance = 0;

		if(distance > 0)
		{
			V3 extra;
			Multiply(ray.direction, distance * (1- EPSILON), extra);

			V3 intersectionPoint;
			Add(ray.position, extra, intersectionPoint);

			for (std::vector<RTLight>::size_type i = 0; i != lights.size(); ++i)
			{
				RTLight light = lights[i];

				V3 lightVector;
				Subtract(light.position, intersectionPoint, lightVector);
				Normalize(lightVector);

				V3 sphereNormal;
				Subtract(intersectionPoint, sphere.GetPosition(), sphereNormal);
				Normalize(sphereNormal);

				float dotProduct = Dot(sphereNormal, lightVector);

				if(dotProduct > 0)
				{
					RGBA sphereColour = sphere.GetColour();

					pixel.red += sphereColour.red * dotProduct;
					pixel.green += sphereColour.green * dotProduct;
					pixel.blue += sphereColour.blue * dotProduct;
				}
			}
		}
	}
}
*/

// Write the final bitmap to disk. Code adapted from another raytracer
// from www.superjer.com under a "do whatever you like with this code"
// license.
void writeBitmap(RGBA* pixelData, const int w, const int h)
{
	FILE *f;

	// 54 bytes in the bitmap file header plus 3 bytes per pixel.
	const int filesize = 3 * w * h + bytesInBitmapHeader;	

	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};

	bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(w    );
    bmpinfoheader[ 5] = (unsigned char)(w>> 8);
    bmpinfoheader[ 6] = (unsigned char)(w>>16);
    bmpinfoheader[ 7] = (unsigned char)(w>>24);
    bmpinfoheader[ 8] = (unsigned char)(h    );
    bmpinfoheader[ 9] = (unsigned char)(h>> 8);
    bmpinfoheader[10] = (unsigned char)(h>>16);
    bmpinfoheader[11] = (unsigned char)(h>>24);

	// Open/Create a file resulting image to disk.
	f = fopen("myFirstImg.bmp", "wb");

	// Write the header data.
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);

	// Every 'line' of bitmap data must have a multiple of 4 bytes, so we may
	// need to write up to 3 bytes of extra data.
	const unsigned char bmppad[3] = {0,0,0};

	// Calculate how many bytes need to be written as padding.
	const int pad = (3 * w) % 4;

	// Bitmaps must be written from the bottom row upwards rather than the top
	// down. 
	for(int y = h - 1; y > -1; y--)
	{
		// For each row in the image, calculate its position in the array
		// and write it out.
		const int rowNum = y * w;

		// Written in BGR order, 1 row at a time.
		fwrite(&pixelData[rowNum], 1, 3 * w, f);
			
		// If the rows need padding, write out 4 minus pad bytes of zero now.
		if(pad > 0)
			fwrite(bmppad, 1, 4 - pad, f);
	}

	fclose(f);
}
