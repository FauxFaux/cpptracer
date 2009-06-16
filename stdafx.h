#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <math.h>
#include "v3.h"
#include "ray.h"
#include <xmmintrin.h>
#include <iomanip>

#ifdef _WINDOWS
# define asFloatArray(x) ((x).m128_f32)
#else
# define asFloatArray(x) ((float*)(&x))
#endif


typedef __m128 SSEInt;
typedef unsigned char uchar;
