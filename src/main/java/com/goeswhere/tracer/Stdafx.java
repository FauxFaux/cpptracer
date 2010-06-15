package com.goeswhere.tracer;

import static com.goeswhere.tracer.Mm._mm_and_ps;
import static com.goeswhere.tracer.Mm._mm_andnot_ps;
import static com.goeswhere.tracer.Mm._mm_cmpge_ps;
import static com.goeswhere.tracer.Mm._mm_cmpgt_ps;
import static com.goeswhere.tracer.Mm._mm_movemask_ps;
import static com.goeswhere.tracer.Mm._mm_or_ps;
import static com.goeswhere.tracer.Mm._mm_setzero_ps;

public class Stdafx {
	public final static long XM_CRMASK_CR6TRUE = 0x00000080;
	public final static long XM_CRMASK_CR6FALSE = 0x00000020;
	static boolean XMComparisonAnyTrue(long CR) { return  (((CR) & XM_CRMASK_CR6FALSE) != XM_CRMASK_CR6FALSE); }
	static boolean XMComparisonAllTrue(long CR) { return  (((CR) & XM_CRMASK_CR6TRUE) == XM_CRMASK_CR6TRUE); }

	static SSEFloat Select(SSEFloat v1, SSEFloat v2, SSEFloat control)
	{
		SSEFloat vTemp1 = _mm_andnot_ps(control, v1);
		SSEFloat vTemp2 = _mm_and_ps(v2, control);
		return _mm_or_ps(vTemp1, vTemp2);
	}

	static long MaskToUInt(SSEFloat mask)
	{
		long CR = 0;
	    int iTest = _mm_movemask_ps(mask);
	    if (iTest==0xf)
		{
	        CR = XM_CRMASK_CR6TRUE;
	    }
	    else if (0 != iTest)
	    {
	        CR = XM_CRMASK_CR6FALSE;
	    }

	    return CR;
	}

	static boolean AnyComponentGreaterThanZero(SSEFloat v1)
	{
		SSEFloat mask = _mm_cmpgt_ps(v1,_mm_setzero_ps());

	    return XMComparisonAnyTrue(MaskToUInt(mask));
	}

	static boolean AllComponentGreaterEqualThanZero(SSEFloat v1)
	{
		SSEFloat mask = _mm_cmpge_ps(v1,_mm_setzero_ps());

	    return XMComparisonAllTrue(MaskToUInt(mask));
	}
}