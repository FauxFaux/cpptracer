package com.goeswhere.tracer;

class CppTracer {

	float EPSILON = 0.001f;
	float defaultViewportWidth = 0.1f;
	float defaultNearClip = 0.1f;
	int defaultThreads = 1;
	int maxThreads = 8;

	int defaultScreenWidth = 12800;
	int defaultScreenHeight = 7200;

	SSEFloat sseOne = _mm_set1_ps(1.0f);

	int bytesInBitmapHeader = 54;

	RtSphere[] spheres = new RtSphere[10];
	int numSpheres;

	RtLight[] lights = new RtLight[10];
	int numLights;

	class timer
	{
		long start;
		timer()
		{
			start = System.nanoTime();
		}

		double End()
		{
			final long end = System.nanoTime();
			final double freq = 1e9;
			return (double)(end-start)/freq;
		}

		void output(double seconds)
		{
			System.out.println(seconds * 1000 + "ms");
		}
	}


	SSEFloat SetFromUInt(int x)
	{
		__m128i V = _mm_set1_epi32( x );
	    return reinterpret_cast<__m128 *>(&V)[0];
	}

	SSEFloat SetFromUIntPtr(int p)
	{
		__m128i V = _mm_loadu_si128( (__m128i*)p );
	    return reinterpret_cast<__m128 *>(&V)[0];
	}





	public static void main(String... origv)
	{
		final int argc = argv.length + 1;
		final String[] argv = new String[argc];
		System.arraycopy(origv, 0, argv, 1, origv.length);

		if(argc == 1)
		{
			printf(" - cppraytracer[.exe] [width] [height] [runCount] [imageCount]\n");
			printf("[width] = Width of rendered image in pixels. Default = 12800\n");
			printf("[height] = Height of rendered image in pixels. Default = 7200\n");
			printf("[runCount] = Number of times to run each render at each thread count, lowest time is chosen. Has the effect of smoothing out the curve / ignoring sporadic CPU load. Default = 1\n");
			printf("[imageCount] = Writes out the rendered BMPs to disk for thread counts <= imageCount. eg '3' will render out images for threadCount 1, 2, 3. Default = 0\n");
			return;
		}

		int width = 0;
		int height = 0;
		int numRuns = 1;
		int writeImagesUpTo = 0;

		if(argc > 1)
			width = atoi(argv[1]);

		if(argc > 2)
			height = atoi(argv[2]);

		if(argc > 3)
			numRuns = atoi(argv[3]);

		if(argc > 4)
			writeImagesUpTo = atoi(argv[4]);

		// If no resolution specified, use defaults.
		if(width == 0 || height == 0)
		{
			width = defaultScreenWidth;
			height = defaultScreenHeight;
		}

		setupScene();

		// Render the image
		AJRGB* pixelData = new AJRGB[width * height];

		double lowest = std::numeric_limits<double>().max();
		int lowestThreads = 0;

		for (int i=1; i<=height; ++i)
			if (height%i == 0)
			{
				memset(pixelData, 0, sizeof(AJRGB) * width * height);

				double iLowest = std::numeric_limits<double>().max();

				for(int r = 0; r < numRuns; r++)
				{
					timer t;
					startRender(pixelData, width, height, i);
					double time = t.End();

					if(time < lowest)
					{
						lowest = time;
						lowestThreads = i;
					}
					if(time < iLowest)
						iLowest = time;
				}

				cout << setw(4) << right << i << ": " << iLowest * 1000 << std::endl;

				if(i <= writeImagesUpTo)
					writeBitmap(pixelData, width, height, i);
			}

			std::cout << setw(4) << right << "Interleaved version." << std::endl;
			std::cout << setw(4) << right << "Lowest: " << lowest * 1000 << "ms at " << lowestThreads << " threads." << std::endl;

		delete [] pixelData;

		return 0;
	}

	void setupScene()
	{
		// Just some code to put a sphere and a light into the scene, ideally
		// this would be read in from some kind of script / scene file rather
		// than being hard-coded :)

		RtSphere sphere = new RtSphere(new V3(0, 0, 1.75f), 0.15f, new SSERGB(1, 1, 1));
		sphere.SetSpecular(0.25f); sphere.SetReflection(0.9f); sphere.SetDiffuse(0.25f);
		spheres[0] = sphere;

		RtSphere sphere2(V3(-0.3f, 0, 1.75f), 0.15f, SSERGB(1, 1, 0));
		sphere2.SetSpecular(0.25f); sphere2.SetReflection(0.25f);
		spheres[1] = sphere2;

		RtSphere sphere3(V3(0.3f, 0, 1.75f), 0.15f, SSERGB(0, 0, 1));
		sphere3.SetSpecular(0.25f); sphere3.SetReflection(0.25f);
		spheres[2] = sphere3;

		RtSphere sphere4(V3(0, 0.3f, 1.75f), 0.15f, SSERGB(1, 0, 0));
		sphere4.SetSpecular(0.25f); sphere4.SetReflection(0.25f);
		spheres[3] = sphere4;

		RtSphere sphere5(V3(0, -0.3f, 1.75f), 0.15f, SSERGB(0, 1, 0));
		sphere5.SetSpecular(0.25f); sphere5.SetReflection(0.25f);
		spheres[4] = sphere5;

		numSpheres = 5;

		//RtSphere sphere6(V3(-0.3f, 0.3f, 1.75f), 0.15f, AJRGB(255, 0, 255));
		//sphere6.SetSpecular(0.25f); sphere6.SetReflection(0.25f);
		//spheres.push_back(sphere6);

		//RtSphere sphere7(V3(0.3f, 0.3f, 1.75f), 0.15f, AJRGB(0, 255, 255));
		//sphere7.SetSpecular(0.25f); sphere7.SetReflection(0.25f);
		//spheres.push_back(sphere7);

		//RtSphere sphere8(V3(0.3f, -0.3f, 1.75f), 0.15f, AJRGB(255, 255, 255));
		//sphere8.SetSpecular(0.25f); sphere8.SetReflection(0.25f);
		//spheres.push_back(sphere8);

		//RtSphere sphere9(V3(-0.3f, -0.3f, 1.75f), 0.15f, AJRGB(0, 0, 0));
		//sphere9.SetSpecular(0.25f); sphere9.SetReflection(0.25f);
		//spheres.push_back(sphere9);

		lights[0] = RTLight(V3(0, 0, 1), 1);

		numLights = 1;
		// End of scene data.
	}

	void startRender(AJRGB* pixelData, int width, int height, int numThreads)
	{
		if(numThreads < 1)
			numThreads = 1;

	#if defined(USEOPENMP)
	omp_set_num_threads(numThreads);
	#pragma omp parallel for
		for(int t = 0; t < numThreads; ++t)
		{
			render(pixelData, width, height, t * (height / numThreads), (height / numThreads));
		}
	#else
		ptr_vector<thread> threads;

		for(int t = 0; t < numThreads; ++t)
			threads.push_back(new thread(bind(&render, pixelData, width, height, t, numThreads)));

		BOOST_FOREACH(thread& t, threads)
			t.join();
	#endif
	}


	void render(AJRGB* pixelData, int width, int height, int threadID, int numThreads)
	{
		// Calculate the height of the viewport depending on its width and the aspect
		// ratio of the image.
		float viewportWidth = defaultViewportWidth;
		float viewportHeight = viewportWidth / ((float)width / height);

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

		SSERGB colourPacket(0,0,0);

		// Scanning across in rows from the top
		for(unsigned int y = threadID; y < height; y+=numThreads)
		{
			SSEFloat sseY = _mm_set1_ps(y);

			// Four pixels at a time.
			for(unsigned int x = 0; x < width; x+=4)
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
				colourPacket.green =  _mm_min_ps(colourPacket.green, twoFiftyFive);
				colourPacket.blue =  _mm_min_ps(colourPacket.blue, twoFiftyFive);

				AJRGB* pixelPtr = pixelData + (x + y * width);

				for(unsigned int i = 0; i < 4; i++)
				{
					pixelPtr[i].red = (uchar)asFloatArray(colourPacket.red)[i];
					pixelPtr[i].green = (uchar)asFloatArray(colourPacket.green)[i];
					pixelPtr[i].blue = (uchar)asFloatArray(colourPacket.blue)[i];
				}
			}
		}
	}


	SSEFloat zero = _mm_setzero_ps();
	SSEFloat trues = _mm_cmpeq_ps(zero, zero);
	SSEFloat miss = _mm_set1_ps(0xFFFFFFFF);

	void raytrace(SSERGB& colour, Ray& rays, int iteration, int w, int h)
	{
		if(iteration > 10)
			return;

		SSEFloat isTracingMask = _mm_cmpneq_ps(rays.positionX, miss);
		SSEFloat sseNearest = trues;

		unsigned int uniqueSpheres = 0;

		// Set the nearest intersection to as large as possible.
		SSEFloat nearestDistance = _mm_set_ps1(std::numeric_limits<float>().max());

		// For every sphere in the scene see if the ray intersects it.
		for(unsigned int s = 0; s < numSpheres; s++)
		{
			// Intersect the packet of rays with the sphere and have the distance to
			// the intersection point returned.
			RtSphere& sphere = spheres[s];
			SSEFloat distance = sphere.IntersectTest(rays);

			SSEFloat distGTZeroMask = _mm_cmpgt_ps(distance, zero);
			SSEFloat distLTNearestDistMask = _mm_cmplt_ps(distance, nearestDistance);
			SSEFloat mask = _mm_and_ps(distGTZeroMask, distLTNearestDistMask);

			sseNearest = Select(sseNearest, SetFromUInt(s), mask);
			nearestDistance = Select(nearestDistance, distance, mask);
		}

		sseNearest = Select(trues, sseNearest, isTracingMask);
		ALIGN16 unsigned int nearest[4];
		_mm_store_ps((float*)nearest, sseNearest);

		ALIGN16 unsigned int spheresHit[4] = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}

		// NO idea how to sse this.
		for(int n = 0; n < 4; n++)
		{
			if(nearest[n] != 0xFFFFFFFF)
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

		for(unsigned int sh = 0; sh < uniqueSpheres; sh++)
		{
			unsigned int sphereIndex = spheresHit[sh];
			RtSphere& sphere = spheres[sphereIndex];	// The sphere to be tested.

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
				RTLight& light = lights[i];

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

				V3& spherePosition = sphere.GetPosition();

				// Calculate the normal at the intersection point, for a sphere this is
				// simply the intersection point - sphere centre, normalised.
				SSEFloat sphereNormalX = _mm_sub_ps(intPointX, _mm_set_ps1(spherePosition.x));
				SSEFloat sphereNormalY = _mm_sub_ps(intPointY, _mm_set_ps1(spherePosition.y));
				SSEFloat sphereNormalZ = _mm_sub_ps(intPointZ, _mm_set_ps1(spherePosition.z));

				NormalizeSSE(sphereNormalX, sphereNormalY, sphereNormalZ);

				SSERGB& sphereColour = sphere.GetColour();
				float reflectionFactor = sphere.GetReflection();

				if(reflectionFactor > 0)
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
					reflectedPacket.positionX = Select(miss, intPointX, nearestMask);
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
				SSEFloat dotProduct = DotSSE(sphereNormalX, sphereNormalY, sphereNormalZ,
											lightDirX, lightDirY, lightDirZ);

				SSEFloat dpgtZeroMask = _mm_cmpgt_ps(dotProduct, _mm_setzero_ps());

				if(AnyComponentGreaterThanZero(dotProduct))
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

					SSEFloat newRed = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(sphereColour.red, dotProduct), lightPower), sphereDiffuse), shade), colour.red);
					SSEFloat newGreen = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(sphereColour.green, dotProduct), lightPower), sphereDiffuse), shade), colour.green);
					SSEFloat newBlue = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(_mm_mul_ps(sphereColour.blue, dotProduct), lightPower), sphereDiffuse), shade), colour.blue);

					colour.red = Select(colour.red, newRed, validHitMask);
					colour.green = Select(colour.green, newGreen, validHitMask);
					colour.blue = Select(colour.blue, newBlue, validHitMask);
				}

				// We only need calculate the specular lighting component for this
				// sphere if it has a specular value greater than zero.
				if(sphere.GetSpecular() > 0)
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
					SSEFloat specDP = DotSSE(rays.directionX, rays.directionY, rays.directionZ,
													reflectionX, reflectionY, reflectionZ);

					for(int r = 0; r < 4; r++)
					{
						// If this ray is hitting this sphere, and it's dot product > 0
						if(nearest[r] == sphereIndex && asFloatArray(specDP)[r] > 0)
						{
							float specular = pow(asFloatArray(specDP)[r], 10) * sphere.GetSpecular();

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

	SSEFloat getNearestObstruction(Ray& rays)
	{
		SSEFloat nearestObstruction = _mm_set1_ps(0xffffffff);
		SSEFloat zero = _mm_setzero_ps();

		for(unsigned int s = 0; s < numSpheres; s++)
		{
			RtSphere& sphere = spheres[s];
			SSEFloat distance = sphere.IntersectTest(rays);

			SSEFloat gtZeroMask = _mm_cmpgt_ps(distance, zero);
			SSEFloat ltNearestObs = _mm_cmplt_ps(distance, nearestObstruction);
			SSEFloat mask = _mm_and_ps(gtZeroMask, ltNearestObs);
			nearestObstruction = Select(nearestObstruction, distance, mask);
		}

		return nearestObstruction;
	}

	// Write the final bitmap to disk. Code adapted from another raytracer
	// from www.superjer.com under a "do whatever you like with this code"
	// license.
	void writeBitmap(AJRGB* pixelData, int w, int h, int tc)
	{
		FILE *f;

		// 54 bytes in the bitmap file header plus 3 bytes per pixel.
		int filesize = 3 * w * h + bytesInBitmapHeader;

		unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0}
		unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0}

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
		char str[256];
		sprintf(str, "myFirstImg_%d.bmp", tc);

		f = fopen(str, "wb");

		// Write the header data.
		fwrite(bmpfileheader, 1, 14, f);
		fwrite(bmpinfoheader, 1, 40, f);

		// Every 'line' of bitmap data must have a multiple of 4 bytes, so we may
		// need to write up to 3 bytes of extra data.
		unsigned char bmppad[3] = {0,0,0}

		// Calculate how many bytes need to be written as padding.
		int pad = (3 * w) % 4;

		// Bitmaps must be written from the bottom row upwards rather than the top
		// down.
		for(int y = h - 1; y > -1; y--)
		{
			// For each row in the image, calculate its position in the array
			// and write it out.
			int rowNum = y * w;

			// Written in BGR order, 1 row at a time.
			fwrite(&pixelData[rowNum], 1, 3 * w, f);

			// If the rows need padding, write out 4 minus pad bytes of zero now.
			if(pad > 0)
				fwrite(bmppad, 1, 4 - pad, f);
		}

		fclose(f);
	}
}