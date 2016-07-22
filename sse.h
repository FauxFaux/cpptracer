#define USE_256

#ifdef __GNUC__
# define ALIGN16 __attribute__ ((aligned (16)))
# define ALIGN32 __attribute__ ((aligned (32)))
#else
# define ALIGN16 __declspec(align(16))
# define ALIGN32 __declspec(align(32))
#endif

#ifdef USE_256

#include <avx2intrin.h>

typedef __m256 SSEFloat;
typedef __m256i SSEInt;

#define aj_add_ps _mm256_add_ps
#define aj_andnot_ps _mm256_andnot_ps
#define aj_and_ps _mm256_and_ps
#define aj_cmpeq_ps(a, b) _mm256_cmp_ps(a, b, _CMP_EQ_OQ)
#define aj_cmpge_ps(a, b) _mm256_cmp_ps(a, b, _CMP_GE_OQ)
#define aj_cmpgt_ps(a, b) _mm256_cmp_ps(a, b, _CMP_GT_OQ)
#define aj_cmplt_ps(a, b) _mm256_cmp_ps(a, b, _CMP_LT_OQ)
#define aj_cmpneq_ps(a, b) _mm256_cmp_ps(a, b, _CMP_NEQ_UQ)
#define aj_div_ps _mm256_div_ps
#define aj_loadu_si _mm256_loadu_si256
#define aj_min_ps _mm256_min_ps
#define aj_movemask_ps _mm256_movemask_ps
#define aj_mul_ps _mm256_mul_ps
#define aj_or_ps _mm256_or_ps
#define aj_rsqrt_ps _mm256_rsqrt_ps
#define aj_set _mm256_set
#define aj_set1_epi32 _mm256_set1_epi32
#define aj_set1_ps _mm256_set1_ps
#define aj_set_ps _mm256_set_ps
#define aj_setr_ps _mm256_setr_ps
#define aj_setzero_ps _mm256_setzero_ps
#define aj_sqrt_ps _mm256_sqrt_ps
#define aj_store_ps _mm256_store_ps
#define aj_sub_ps _mm256_sub_ps

#define aj_set_offsets() _mm256_setr_ps(0, 1, 2, 3, 4, 5, 6, 7)

const int batch = 8;
#define SPHERES_HIT_INITIALISER {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF }

#define AJ_ALIGN ALIGN32

#else

#include <xmmintrin.h>

typedef __m128 SSEFloat;
typedef __m128i SSEInt;

#define aj_add_ps _mm_add_ps
#define aj_andnot_ps _mm_andnot_ps
#define aj_and_ps _mm_and_ps
#define aj_cmpeq_ps _mm_cmpeq_ps
#define aj_cmpge_ps _mm_cmpge_ps
#define aj_cmpgt_ps _mm_cmpgt_ps
#define aj_cmplt_ps _mm_cmplt_ps
#define aj_cmpneq_ps _mm_cmpneq_ps
#define aj_div_ps _mm_div_ps
#define aj_loadu_si _mm_loadu_si128
#define aj_min_ps _mm_min_ps
#define aj_movemask_ps _mm_movemask_ps
#define aj_mul_ps _mm_mul_ps
#define aj_or_ps _mm_or_ps
#define aj_rsqrt_ps _mm_rsqrt_ps
#define aj_set _mm_set
#define aj_set1_epi32 _mm_set1_epi32
#define aj_set1_ps _mm_set1_ps
#define aj_set_ps _mm_set_ps
#define aj_set_ps1 _mm_set_ps1
#define aj_setr_ps _mm_setr_ps
#define aj_setzero_ps _mm_setzero_ps
#define aj_sqrt_ps _mm_sqrt_ps
#define aj_store_ps _mm_store_ps
#define aj_sub_ps _mm_sub_ps

#define aj_set_offsets() _mm_setr_ps(0, 1, 2, 3)

#define AJ_ALIGN ALIGN16

const int batch = 4;
#define SPHERES_HIT_INITIALISER {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}
#endif