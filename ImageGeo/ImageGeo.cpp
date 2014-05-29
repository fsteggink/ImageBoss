// ImageGeo.cpp
// FS, 14-07-2007

#define DLLAPI_IMAGEGEO __declspec(dllexport)

#include "ImageGeo_internal.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_ATTACH:
		break;
	
	case DLL_THREAD_DETACH:
		break;
	
	case DLL_PROCESS_DETACH:
		break;
	}

    return TRUE;
}


Rubbersheet::Rubbersheet(const std::vector<PixelCoord> &_params, int _sectorsX, int _sectorsY,
						 int _imageWidth, int _imageHeight):
	params(_params), sectorsX(_sectorsX), sectorsY(_sectorsY), imageWidth(_imageWidth),
		imageHeight(_imageHeight)
{
	if(sectorsX <= 0 || sectorsY <= 0)
		throw "SectorsX and SectorsY should be larger than zero";
	if(imageWidth <= 0 || imageHeight <= 0)
		throw "ImageWidth and ImageHeight should be larger than zero";
	else if(params.size() != (sectorsX + 1) * (sectorsY + 1))
		throw "Number of parameters and sectors aren't equal";

	pm = PixelMatrix(
		static_cast<double>(sectorsX) / static_cast<double>(imageWidth), 0,
		0, static_cast<double>(sectorsY) / static_cast<double>(imageHeight),
		0, 0);
}


PixelCoord Rubbersheet::execute(const PixelCoord &input) const
{
	if(input.x < 0 || input.x >= imageWidth || input.y < 0 || input.y >= imageHeight)
		throw "Given pixel outside image range";

	PixelCoord ps = pm.execute(input);
	int ps_x = static_cast<int>(ps.x);
	int ps_y = static_cast<int>(ps.y);
	double dx = ps.x - ps_x;
	double dy = ps.y - ps_y;
	const PixelCoord &A = params[ps_y * (sectorsX + 1) + ps_x];
	const PixelCoord &B = params[ps_y * (sectorsX + 1) + ps_x + 1];
	const PixelCoord &C = params[(ps_y + 1) * (sectorsX + 1) + ps_x];
	const PixelCoord &D = params[(ps_y + 1) * (sectorsX + 1) + ps_x + 1];

	PixelCoord output;
	output.x = (A.x * (1 - dx) + B.x * dx) * (1 - dy) + (C.x * (1 - dx) + D.x * dx) * dy;
	output.y = (A.y * (1 - dy) + C.y * dy) * (1 - dx) + (B.y * (1 - dy) + D.y * dy) * dx;

	return output;
}


CoordXY ImageMatrix::execute(const PixelCoord &input) const
{
	return CoordXY(input.x * a + input.y * c + e, input.x * b + input.y * d + f);
}


CoordXY ImageMatrixI::execute(const PixelCoordI &input) const
{
	return CoordXY(input.x * a + input.y * c + e, input.x * b + input.y * d + f);
}


PixelCoord GeoMatrix::execute(const CoordXY &input) const
{
	return PixelCoord(input.x * a + input.y * c + e, input.x * b + input.y * d + f);
}


PixelCoordI GeoMatrixI::execute(const CoordXY &input) const
{
	return PixelCoordI(
		static_cast<int>(input.x * a + input.y * c + e),
		static_cast<int>(input.x * b + input.y * d + f));
}


PixelCoord PixelMatrix::execute(const PixelCoord &input) const
{
	return PixelCoord(input.x * a + input.y * c + e, input.x * b + input.y * d + f);
}
