#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <math.h>
#include "V3.h"
#include "Ray.h"
#include <xmmintrin.h>
#include <iomanip>


typedef __m128 SSEInt;
typedef unsigned char uchar;