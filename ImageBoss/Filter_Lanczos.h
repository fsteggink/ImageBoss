// Filter_Lanczos.h
// FS, 23-08-2014

#include "../../Inc/Filter.h"
#include <cmath>


#ifndef FILTER_LANCZOS_H
#define FILTER_LANCZOS_H

////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////

inline double sinc(double x)
{
	double pi_x = PI * x;
	return sin(pi_x) / pi_x;
}

inline float sinc(float x)
{
	float pi_x = static_cast<float>(PI * x);
	return sin(pi_x) / pi_x;
}


////////////////////////////////////////////////////////////
// Template method implementations
////////////////////////////////////////////////////////////

template<int win>
LanczosFilter<win>::LanczosFilter()
{
	Init();
}


template<int win>
void LanczosFilter<win>::Init()
{
	if(!m_vecPrecalc_wsinc.empty())
		return;

	// sinc is symmetric at the y-axis, so only positive values are needed
	int iPrecalcSize = static_cast<int>((win + 1) / m_cdPRECALC_RES + 1);
	m_vecPrecalc_wsinc.reserve(iPrecalcSize);
	m_vecPrecalc_wsinc.push_back(1);
	for(int i = 1; i < iPrecalcSize; ++i)
	{
		m_vecPrecalc_wsinc.push_back(::sinc(i * m_cdPRECALC_RES) * ::sinc(i * m_cdPRECALC_RES / win));
	}

	return;
}


template<int win>
float LanczosFilter<win>::wsinc(float x) const
{
	static float invPrecalcRes = 1.0f / m_cdPRECALC_RES;
	int ix = static_cast<int>(fabs(x) * invPrecalcRes + 0.5f);
	return m_vecPrecalc_wsinc[ix];
}


template<int win>
void LanczosFilter<win>::getCoeffs(FilterCoeffs &coeffs, float dPhase, float dDecimation) const
{
	//static float keep_phase = 0;
	//static float keep_decimation = -1;
	//static FilterCoeffs keep_coeffs;

	//if(keep_phase == dPhase && keep_decimation == dDecimation)
	//{
	//	coeffs = keep_coeffs;
	//	return;
	//}

	coeffs.coeffs.clear();

	if(dDecimation <= 1)
	{
		coeffs.coeffs.reserve(win * 2);
		for(int iStep = -win - 1; iStep <= win; ++iStep)
		{
			float x = iStep - dPhase;
			if(fabs(x) <= win)
			{
				if(coeffs.coeffs.empty())
					coeffs.min = iStep;

				coeffs.coeffs.push_back(wsinc(x));
			}
		}
	}
	else
	{
		int iMin = static_cast<int>(-dDecimation * win + dPhase);
		int iMax = static_cast<int>(dDecimation * win + dPhase);
		coeffs.coeffs.reserve(iMax - iMin + 1);
		float invDecimation = 1.0f / dDecimation;
		for(int iStep = iMin; iStep <= iMax; ++iStep)
		{
			float x = (iStep - dPhase) * invDecimation;
			if(fabs(x) <= win)
			{
				if(coeffs.coeffs.empty())
					coeffs.min = iStep;

				coeffs.coeffs.push_back(wsinc(x));
			}
		}
	}

	//keep_coeffs = coeffs;
	//keep_phase = dPhase;
	//keep_decimation = dDecimation;

	return;
}

#endif // FILTER_LANCZOS_H
