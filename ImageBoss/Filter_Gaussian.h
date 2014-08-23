// Filter_Gaussian.h
// FS, 23-08-2014

#include "../../Inc/Filter.h"


#ifndef FILTER_GAUSSIAN_H
#define FILTER_GAUSSIAN_H

template<int var>
void GaussianFilter<var>::getCoeffs(FilterCoeffs &coeffs, float dPhase, float dDecimation) const
{
	static double width = 2.5 + var / 4.0;

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
		coeffs.coeffs.reserve(width * 2);
		for(int iStep = static_cast<int>(-width - 1); 
			iStep < static_cast<int>(width + 1); ++iStep)
		{
			if(fabs(iStep - dPhase) < width)
			{
				if(coeffs.coeffs.empty())
					coeffs.min = iStep;

				coeffs.coeffs.push_back(pow(2.0, var * pow2(iStep - dPhase)));
			}
		}
	}
	else
	{
		int iMin = static_cast<int>(-dDecimation * width + dPhase);
		int iMax = static_cast<int>(dDecimation * width + dPhase);
		coeffs.coeffs.reserve(iMax - iMin + 1);
		float invDecimation = 1.0f / dDecimation;
		for(int iStep = iMin; iStep <= iMax; ++iStep)
		{
			if(fabs((iStep - dPhase) / dDecimation) < width)
			{
				if(coeffs.coeffs.empty())
					coeffs.min = iStep;

				coeffs.coeffs.push_back(pow(2.0, var * pow2((iStep - dPhase) * invDecimation)));
			}
		}
	}

	//keep_coeffs = coeffs;
	//keep_phase = dPhase;
	//keep_decimation = dDecimation;

	return;
}

#endif // FILTER_GAUSSIAN_H
