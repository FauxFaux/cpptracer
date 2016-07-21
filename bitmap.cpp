#include <cstdio>
#include "bitmap.h"

const int bytesInBitmapHeader = 54;

// Write the final bitmap to disk. Code adapted from another raytracer
// from www.superjer.com under a "do whatever you like with this code"
// license.
void writeBitmap(AJRGB *pixelData, const int w, const int h, const int tc)
{
	FILE *f;

	// 54 bytes in the bitmap file header plus 3 bytes per pixel.
	const int filesize = 3 * w * h + bytesInBitmapHeader;

	unsigned char bmpfileheader[14] = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
	unsigned char bmpinfoheader[40] = {40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0};

	bmpfileheader[2] = (unsigned char) (filesize);
	bmpfileheader[3] = (unsigned char) (filesize >> 8);
	bmpfileheader[4] = (unsigned char) (filesize >> 16);
	bmpfileheader[5] = (unsigned char) (filesize >> 24);

	bmpinfoheader[4] = (unsigned char) (w);
	bmpinfoheader[5] = (unsigned char) (w >> 8);
	bmpinfoheader[6] = (unsigned char) (w >> 16);
	bmpinfoheader[7] = (unsigned char) (w >> 24);
	bmpinfoheader[8] = (unsigned char) (h);
	bmpinfoheader[9] = (unsigned char) (h >> 8);
	bmpinfoheader[10] = (unsigned char) (h >> 16);
	bmpinfoheader[11] = (unsigned char) (h >> 24);

	// Open/Create a file resulting image to disk.
	char str[256];
	sprintf(str, "myFirstImg_%d.bmp", tc);

	f = fopen(str, "wb");

	// Write the header data.
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);

	// Every 'line' of bitmap data must have a multiple of 4 bytes, so we may
	// need to write up to 3 bytes of extra data.
	const unsigned char bmppad[3] =  {0, 0, 0};

	// Calculate how many bytes need to be written as padding.
	const int pad = (3 * w) % 4;

	// Bitmaps must be written from the bottom row upwards rather than the top
	// down.
	for (int y = h - 1; y > -1; y--)
	{
		// For each row in the image, calculate its position in the array
		// and write it out.
		const int rowNum = y * w;

		// Written in BGR order, 1 row at a time.
		fwrite(&pixelData[rowNum], 1, 3 * w, f);

		// If the rows need padding, write out 4 minus pad bytes of zero now.
		if (pad > 0)
			fwrite(bmppad, 1, 4 - pad, f);
	}

	fclose(f);
}
