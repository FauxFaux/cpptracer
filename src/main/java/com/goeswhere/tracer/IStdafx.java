package com.goeswhere.tracer;

class Stdafx {
	//typedef __m128i SSEInt;
	typedef __m128 SSEFloat;
	typedef unsigned char uchar;

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
	    if (iTest==0xf)
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
		SSEFloat mask = _mm_cmpgt_ps(v1,_mm_setzero_ps());

	    return XMComparisonAnyTrue(MaskToUInt(mask));
	}

	inline bool AllComponentGreaterEqualThanZero(SSEFloat v1)
	{
		SSEFloat mask = _mm_cmpge_ps(v1,_mm_setzero_ps());

	    return XMComparisonAllTrue(MaskToUInt(mask));
	}
}