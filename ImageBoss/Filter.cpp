// Filter.cpp
// FS, 23-08-2014

#define DLLAPI_IMAGEBOSS __declspec(dllexport)

#include "../../Inc/Filter.h"
#include "Filter_Lanczos.h"
#include "Filter_Gaussian.h"
#include "../../Inc/ImageBoss.h"

DLLAPI_IMAGEBOSS float LanczosFilter<2>::m_cdPRECALC_RES = 0.1f;
DLLAPI_IMAGEBOSS float LanczosFilter<3>::m_cdPRECALC_RES = 0.1f;

template class DLLAPI_IMAGEBOSS LanczosFilter<2>;
template class DLLAPI_IMAGEBOSS LanczosFilter<3>;


void RasterFilter::getNormalizedCoeffs(FilterCoeffs &coeffs, float dPhase, float dDecimation) const
{
	//static double keep_phase = 0;
	//static double keep_decimation = -1;
	//static FilterCoeffs keep_coeffs;

	//if(keep_phase == dPhase && keep_decimation == dDecimation)
	//{
	//	coeffs = keep_coeffs;
	//	return;
	//}

	getCoeffs(coeffs, dPhase, dDecimation);

	// Calculate the sum of the coefficients
	float dSum = 0;
	for(std::vector<float>::iterator iterCoeff = coeffs.coeffs.begin();
		iterCoeff != coeffs.coeffs.end(); ++iterCoeff)
	{
		dSum += *iterCoeff;
	}

	// Apply the normalization
	float invSum = 1.0f / dSum;
	for(std::vector<float>::iterator iterCoeff = coeffs.coeffs.begin();
		iterCoeff != coeffs.coeffs.end(); ++iterCoeff)
	{
		*iterCoeff *= invSum;
	}

	//keep_coeffs = coeffs;
	//keep_phase = dPhase;
	//keep_decimation = dDecimation;

	return;
}


void BoxFilter::getCoeffs(FilterCoeffs &coeffs, float dPhase, float dDecimation) const
{
	coeffs.coeffs.clear();

	if(dDecimation <= 1)
	{
		coeffs.min = 0;
		coeffs.coeffs.push_back(1);
	}
	else
	{
		int iWidth = static_cast<int>(dDecimation);
		coeffs.min = -iWidth;
		coeffs.coeffs.reserve(2 * iWidth + 1);
		for(int iStep = -iWidth; iStep <= iWidth; ++iStep)
		{
			coeffs.coeffs.push_back(1);
		}
	}

	return;
}


void TentFilter::getCoeffs(FilterCoeffs &coeffs, float dPhase, float dDecimation) const
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
		coeffs.coeffs.reserve(2);
		if(dPhase >= 0)
		{
			coeffs.min = 0;
			coeffs.coeffs.push_back(1 - dPhase);
			coeffs.coeffs.push_back(dPhase);
		}
		else
		{
			coeffs.min = -1;
			coeffs.coeffs.push_back(-dPhase);
			coeffs.coeffs.push_back(1 + dPhase);
		}
	}
	else
	{
		int iMin = coeffs.min = static_cast<int>(-dDecimation + dPhase);
		int iMax = static_cast<int>(dDecimation + dPhase) + 1;
		coeffs.coeffs.resize(iMax - iMin);
		float invDecimation = 1.0f / dDecimation;
		for(int iStep = iMin; iStep < iMax; ++iStep)
		{
			float f = 1.0f - abs(iStep - dPhase) * invDecimation;
			coeffs.coeffs[iStep - iMin] = f;
		}
	}

	//keep_coeffs = coeffs;
	//keep_phase = dPhase;
	//keep_decimation = dDecimation;

	return;
}
