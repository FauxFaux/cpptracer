#include <gtest/gtest.h>

#include "tracer.h"

static bool generate = false;

static AJRGB *runRender(int width, int height, int threads)
{
	setupScene();

	AJRGB *pixelData = new AJRGB[width * height];
	startRender(pixelData, width, height, threads);
	return pixelData;
}

static const char *const ppm_header_format = "P6\n%d %d\n255\n";

static void ppm_write(const char *file, AJRGB *value, uint32_t width, uint32_t height)
{

	FILE *f = fopen(file, "wb");
	ASSERT_NE(nullptr, f);
	ASSERT_GE(fprintf(f, ppm_header_format, width, height), 0);

	for (size_t i = 0; i < width * height; ++i)
	{
		ASSERT_NE(EOF, fputc(int8_t(value[i].red), f));
		ASSERT_NE(EOF, fputc(int8_t(value[i].green), f));
		ASSERT_NE(EOF, fputc(int8_t(value[i].blue), f));
	}

	ASSERT_EQ(0, fclose(f));
}

static void ppm_read(const char *file, AJRGB *&img, int &width, int &height)
{
	FILE *f = fopen(file, "rb");
	ASSERT_NE(nullptr, f);
	ASSERT_EQ('P', fgetc(f));
	ASSERT_EQ('6', fgetc(f));
	ASSERT_EQ('\n', fgetc(f));

	int rgb;
	ASSERT_EQ(3, fscanf(f, "%d %d %d", &width, &height, &rgb));
	ASSERT_EQ(255, rgb);
	ASSERT_EQ('\n', fgetc(f));
	img = new AJRGB[width * height];

	for (int i = 0; i < width * height; ++i)
	{
		img[i].red = fgetc(f);
		img[i].green = fgetc(f);
		img[i].blue = fgetc(f);
	}

	ASSERT_EQ(0, fclose(f));
}

static int square(int val)
{
	return val * val;
}

static double mse(const uint32_t test_width, const uint32_t test_height,
				  const AJRGB *actual, const AJRGB *expected)
{
	uint64_t mse_sum = 0;
	for (off_t i = 0; i < test_width * test_height; ++i)
	{
		mse_sum += square(expected[i].red - actual[i].red);
		mse_sum += square(expected[i].green - actual[i].green);
		mse_sum += square(expected[i].blue - actual[i].blue);
	}

	double result = mse_sum / double(test_width) / test_height / 3;
	return result;
}

static AJRGB *load_expected(const char *path)
{
	AJRGB *expected;
	int ref_width, ref_height;
	ppm_read(path, expected, ref_width, ref_height);
	return expected;
}

const uint32_t test_width = 320;
const uint32_t test_height = 240;
AJRGB *expected_image = load_expected("ref-320x240.ppm");

TEST(tracer, single_thread)
{
	AJRGB *actual = runRender(test_width, test_height, 1);
	if (generate)
	{
		ppm_write("ref.ppm", actual, test_width, test_height);
	}

	EXPECT_DOUBLE_EQ(0, mse(test_width, test_height, actual, expected_image));
	delete[] actual;
}

TEST(tracer, sensible_threads)
{
	AJRGB *actual = runRender(test_width, test_height, 8);
	EXPECT_DOUBLE_EQ(0, mse(test_width, test_height, actual, expected_image));
	delete[] actual;
}
