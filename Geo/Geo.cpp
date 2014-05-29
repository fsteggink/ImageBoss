// Geo.cpp
// FS, 30-06-2007

#define DLLAPI_GEO __declspec(dllexport)

#include "Geo_internal.h"

#include <iostream>
#define showVal(val) std::cout << #val " = " << (val) << std::endl
DLLAPI_GEO Spheroid g_spheroidWGS84;
DLLAPI_GEO Datum g_datumWGS84;
DLLAPI_GEO boost::shared_ptr<GeographicCS> g_csWGS84;
DLLAPI_GEO boost::shared_ptr<ProjectedCS> g_csMercatorWGS84;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		initDLL();
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


void initDLL()
{
	g_spheroidWGS84.parseWKT(g_SPHEROID_WGS84_);
	g_datumWGS84.parseWKT(g_DATUM_WGS84_);

	g_csWGS84 = boost::shared_ptr<GeographicCS>(new GeographicCS());
	g_csMercatorWGS84 = boost::shared_ptr<ProjectedCS>(new ProjectedCS());

	g_csWGS84->parseWKT(g_GEOGCS_WGS84_);
	g_csMercatorWGS84->parseWKT(g_MERCATOR_WGS84_);

	return;
}


Ellipsoid::Ellipsoid(double _a, double _invF): m_a(_a), m_invF(_invF)
{
	m_b = m_a - m_a / m_invF;
	m_e2 = 2 / m_invF - 1 / pow2(m_invF);
	m_epsilon2 = (pow2(m_a) - pow2(b())) / pow2(b());
}


Bonne::Bonne(const std::map<ProjParam, double> &_params): params(_params)
{
	e2   = params.find(pparE2)->second;
	a    = params.find(pparA)->second;
	pm   = params.find(pparPM)->second;
	lat0 = torad(params.find(pparLat0)->second);
	lon0 = torad(params.find(pparLon0)->second);
	FE   = params.find(pparFE)->second;
	FN   = params.find(pparFN)->second;
	k0   = params.find(pparK)->second;

	m0 = cos(lat0) / sqrt(1 - e2 * pow2(sin(lat0)));
	M0 = a * (
		(1 -     (e2) / 4 -  3 * pow2(e2) /  64 -  5 * pow3(e2) /  256) * lat0 -
		(    3 * (e2) / 8 +  3 * pow2(e2) /  32 + 45 * pow3(e2) / 1024) * sin(2 * lat0) +
		(                   15 * pow2(e2) / 256 + 45 * pow3(e2) / 1024) * sin(4 * lat0) -
		(                                         35 * pow3(e2) / 3072) * sin(6 * lat0));
	e1 = (1 - sqrt(1 - e2)) / (1 + sqrt(1 - e2));
}


ToBonne::ToBonne(const std::map<ProjParam, double> &_params): Bonne(_params)
{
}


CoordXY ToBonne::execute(const CoordLL &input) const
{
	double lat = torad(input.lat);
	double lon = torad(input.lon - pm);

	double m  = cos(lat)  / sqrt(1 - e2 * pow2(sin(lat)));
	double M = a * (
		(1 -     (e2) / 4 -  3 * pow2(e2) /  64 -  5 * pow3(e2) /  256) * lat -
		(    3 * (e2) / 8 +  3 * pow2(e2) /  32 + 45 * pow3(e2) / 1024) * sin(2 * lat) +
		(                   15 * pow2(e2) / 256 + 45 * pow3(e2) / 1024) * sin(4 * lat) -
		(                                         35 * pow3(e2) / 3072) * sin(6 * lat));

	double rho = a * m0 / sin(lat0) + M0 - M;
	double T = a * m * (normalizeLonRad(lon - lon0, 0)) / rho;

	double E = k0 * (rho * sin(T)) + FE;
	double N = k0 * (a * m0 / sin(lat0) - rho * cos(T)) + FN;

	return CoordXY(E, N);
}


FromBonne::FromBonne(const std::map<ProjParam, double> &_params): Bonne(_params)
{
}


CoordLL FromBonne::execute(const CoordXY &input) const
{
	double E = input.x;
	double N = input.y;

	double X = (E - FE) / k0;
	double Y = (N - FN) / k0;

	double rho = ((lat0 > 0) ? 1 : -1) * sqrt(pow2(X) + pow2(a * m0 / sin(lat0) - Y));
	double M = a * m0 / sin(lat0) + M0 - rho;
	double mu = M / (a * (1 - e2 / 4 - 3 * pow2(e2) / 64 - 5 * pow3(e2) / 256));
	
	double lat = 
		mu + 
		( 3 *      e1  /  2 -   27 * pow3(e1) /  32) * sin(2 * mu) +
		(21 * pow2(e1) / 16 -   55 * pow4(e1) /  32) * sin(4 * mu) +
		(                      151 * pow3(e1) /  96) * sin(6 * mu) +
		(                     1097 * pow4(e1) / 512) * sin(8 * mu);

	double m = cos(lat) / sqrt(1 - e2 * pow2(sin(lat)));
	double lon = (lat > 0) ?
		lon0 + rho * (atan( X / (a * m0 / sin(lat0) - Y))) / (a * m) :
		lon0 + rho * (atan(-X / (Y - a * m0 / sin(lat0)))) / (a * m);

	return CoordLL(todeg(lat), todeg(lon) + pm);
}


Mercator::Mercator(const std::map<ProjParam, double> &_params): params(_params)
{
	pm   = 0.0;
	lon0 = torad(params.find(pparLon0)->second);
	FE   = params.find(pparFE)->second;
	FN   = params.find(pparFN)->second;
}


ToMercator::ToMercator(const std::map<ProjParam, double> &_params): Mercator(_params)
{
}


CoordXY ToMercator::execute(const CoordLL &input) const
{
	double lat = torad(input.lat);
	double lon = torad(input.lon - pm);

	double E = lon - lon0 + FE;
	double N = log(tan(0.25 * PI + 0.5 * lat)) + FN;

	return CoordXY(E, N);
}


FromMercator::FromMercator(const std::map<ProjParam, double> &_params): Mercator(_params)
{
}


CoordLL FromMercator::execute(const CoordXY &input) const
{
	double x = input.x - FE;
	double y = input.y - FN;

	double lat = 2 * atan(exp(y)) - 0.5 * PI;
	double lon = x + lon0;

	return CoordLL(todeg(lat), todeg(lon) + pm);
}


ObliqueStereographic::ObliqueStereographic(const std::map<ProjParam, double> &_params):
	params(_params)
{
	e2   = params.find(pparE2)->second;
	a    = params.find(pparA)->second;
	pm   = params.find(pparPM)->second;
	lat0 = torad(params.find(pparLat0)->second);
	lon0 = torad(params.find(pparLon0)->second);
	k0   = params.find(pparK)->second;
	FE   = params.find(pparFE)->second;
	FN   = params.find(pparFN)->second;

	rho0 = a * (1 - e2) / pow(1 - e2 * pow2(sin(lat0)), 1.5);
	nu0 = a / sqrt(1 - e2 * pow2(sin(lat0)));

	R = sqrt(rho0 * nu0);
	n = sqrt(1 + ((e2 * pow4(cos(lat0))) / (1 - e2)));
	e = sqrt(e2);
	S1 = (1 + sin(lat0)) / (1 - sin(lat0));
	S2 = (1 - e * sin(lat0)) / (1 + e * sin(lat0));
	w1 = pow(S1 * pow(S2, e), n);
	sin_chi0 = (w1 - 1) / (w1 + 1);
	c = (n + sin(lat0)) * (1 - sin_chi0) / ((n - sin(lat0)) * (1 + sin_chi0));

	w2 = c * w1;
	chi0 = asin((w2 - 1) / (w2 + 1));
	Lambda0 = lon0;
}


ToObliqueStereographic::ToObliqueStereographic(const std::map<ProjParam, double> &_params):
	ObliqueStereographic(_params)
{
}


CoordXY ToObliqueStereographic::execute(const CoordLL &input) const
{
	double lat = torad(input.lat);
	double lon = torad(input.lon - pm);

	double Sa = (1 + sin(lat)) / (1 - sin(lat));
	double Sb = (1 - e * sin(lat)) / (1 + e * sin(lat));
	double w = c * pow(Sa * pow(Sb, e), n);
	double Lambda = n * (normalizeLonRad(lon - Lambda0, 0)) + Lambda0;
	double chi = asin((w - 1) / (w + 1));

	double B = (1 + sin(chi) * sin(chi0) + cos(chi) * cos(chi0) * cos(Lambda - Lambda0));
	double E = FE + 2 * R * k0 * cos(chi) * sin(Lambda - Lambda0) / B;
	double N = FN + 2 * R * k0 *
		(sin(chi) * cos(chi0) - cos(chi) * sin(chi0) * cos(Lambda - Lambda0)) / B;

	return CoordXY(E, N);
}


FromObliqueStereographic::FromObliqueStereographic(const std::map<ProjParam, double> &_params):
	ObliqueStereographic(_params)
{
}


CoordLL FromObliqueStereographic::execute(const CoordXY &input) const
{
	const double EPSILON = 1e-9;

	double E = input.x;
	double N = input.y;

	double g = 2 * R * k0 * tan(PI / 4 - chi0 / 2);
	double h = 4 * R * k0 * tan(chi0) + g;
	double i = atan((E - FE) / (h + (N - FN)));
	double j = atan((E - FE) / (g - (N - FN))) - i;

	double chi = chi0 + 2 * atan(((N - FN) - (E - FE) * tan(j / 2)) / (2 * R * k0));
	double Lambda = j + 2 * i + Lambda0;

	double lon = (Lambda - Lambda0) / n + Lambda0;
	double psi = 0.5 * log((1 + sin(chi)) / (c * (1 - sin(chi)))) / n;

	double lat1 = 2 * atan(exp(psi)) - PI / 2;
	double psi1 = log((tan(lat1 / 2 + PI / 4)) *
		pow((1 - e * sin(lat1))/(1 + e * sin(lat1)), e / 2));
	double lat2 = lat1 - (psi1 - psi) * cos(lat1) * (1 - e2 * pow2(sin(lat1))) / (1 - e2);

	while(fabs(lat2 - lat1) > EPSILON)
	{
		lat1 = lat2;
		psi1 = log((tan(lat1 / 2 + PI / 4)) * pow((1 - e * sin(lat1))/(1 + e * sin(lat1)), e / 2));
		lat2 = lat1 - (psi1 - psi) * cos(lat1) * (1 - e2 * pow2(sin(lat1))) / (1 - e2);
	}
	lat1 = lat2;

	return CoordLL(todeg(lat1), todeg(lon) + pm);
}


LambertConformalConic_1SP::LambertConformalConic_1SP(const std::map<ProjParam, double> &_params):
	params(_params)
{
	e2   = params.find(pparE2)->second;
	a    = params.find(pparA)->second;
	pm   = params.find(pparPM)->second;
	lat0 = torad(params.find(pparLat0)->second);
	lon0 = torad(params.find(pparLon0)->second);
	k0   = params.find(pparK)->second;
	FE   = params.find(pparFE)->second;
	FN   = params.find(pparFN)->second;

	if(lat0 == 0)
		lat0 = 1e-12;

	e = sqrt(e2);

	t0 = tan(PI / 4 - lat0 / 2) / pow((1 - e * sin(lat0)) / (1 + e * sin(lat0)), e / 2);
	m0 = cos(lat0) / sqrt(1 - e2 * pow2(sin(lat0)));
	n = sin(lat0);
	F = m0 / (n * pow(t0, n));
	r0 = a * F * pow(t0, n) * k0;
}


ToLambertConformalConic_1SP::ToLambertConformalConic_1SP(const std::map<ProjParam, double> &_params):
	LambertConformalConic_1SP(_params)
{
}


CoordXY ToLambertConformalConic_1SP::execute(const CoordLL &input) const
{
	double lat = torad(input.lat);
	double lon = torad(input.lon - pm);
	double sinphi = sin(lat);

	//double t  = tan(PI / 4 - lat / 2) / pow((1 - e * sin(lat)) / (1 + e * sin(lat )), e / 2);
	double t  = tan(PI / 4 - lat / 2) / pow((1 - e * sinphi) / (1 + e * sinphi), e / 2);
	//double m  = cos(lat) / sqrt(1 - e2 * pow2(sin(lat)));
	//double m  = cos(lat) / sqrt(1 - e2 * pow2(sinphi));
	double theta = n * (normalizeLonRad(lon - lon0, 0));
	double r = a * F * pow(t, n) * k0;

	double E = FE + r * sin(theta);
	double N = FN + r0 - r * cos(theta);

	return CoordXY(E, N);
}


FromLambertConformalConic_1SP::FromLambertConformalConic_1SP(const std::map<ProjParam, double> &_params):
	LambertConformalConic_1SP(_params)
{
}


CoordLL FromLambertConformalConic_1SP::execute(const CoordXY &input) const
{
	const double EPSILON = 1e-9;

	double E = input.x;
	double N = input.y;

	double theta_ = atan((E - FE) / (r0 - (N - FN)));
	double r_ = (n / fabs(n)) * sqrt(pow2(E - FE) + pow2(r0 - (N - FN)));
	double t_ = pow(r_ / (a * k0 * F), 1 / n);

	double lat1 = PI / 2 - 2 * atan(t_);
	double lat2 = 0;
	do
	{
		lat2 = lat1;
		lat1 = PI / 2 - 2 * atan(t_ * pow((1 - e * sin(lat2)) / (1 + e * sin(lat2)), e / 2));
	}
	while(fabs(lat2 - lat1) > EPSILON);
	double lon = theta_ / n + lon0;

	return CoordLL(todeg(lat1), todeg(lon) + pm);
}


Polyconic::Polyconic(const std::map<ProjParam, double> &_params):
	params(_params)
{
	e2   = params.find(pparE2)->second;
	a    = params.find(pparA)->second;
	pm   = params.find(pparPM)->second;
	lat0 = torad(params.find(pparLat0)->second);
	lon0 = torad(params.find(pparLon0)->second);
	FE   = params.find(pparFE)->second;
	FN   = params.find(pparFN)->second;

	M0 = a * (
		(1 - e2 / 4 -  3 * pow2(e2) /  64 -  5 * pow3(e2) /  256) * lat0 -
		(3 * e2 / 8 +  3 * pow2(e2) /  32 + 45 * pow3(e2) / 1024) * sin(2 * lat0) +
		(             15 * pow2(e2) / 256 + 45 * pow3(e2) / 1024) * sin(4 * lat0) -
		(                                   35 * pow3(e2) / 3072) * sin(6 * lat0));
}

ToPolyconic::ToPolyconic(const std::map<ProjParam, double> &_params):
	Polyconic(_params)
{
}


CoordXY ToPolyconic::execute(const CoordLL &input) const
{
	double lat = torad(input.lat);
	double lon = torad(input.lon - pm);

	double x, y;

	if(lat == 0)
	{
		x = a * (normalizeLonRad(lon - lon0, 0));
		y = -M0;
	}
	else
	{
		double M = a * (
			(1 - e2 / 4 -  3 * pow2(e2) /  64 -  5 * pow3(e2) /  256) * lat -
			(3 * e2 / 8 +  3 * pow2(e2) /  32 + 45 * pow3(e2) / 1024) * sin(2 * lat) +
			(             15 * pow2(e2) / 256 + 45 * pow3(e2) / 1024) * sin(4 * lat) -
			(                                   35 * pow3(e2) / 3072) * sin(6 * lat));
		double N = a / sqrt(1 - e2 * pow2(sin(lat)));

		double E = (normalizeLonRad(lon - lon0, 0)) * sin(lat);

		x = N * cot(lat) * sin(E);
		y = M - M0 + N * cot(lat) * (1 - cos(E));
	}

	return CoordXY(x + FE, y + FN);
}


FromPolyconic::FromPolyconic(const std::map<ProjParam, double> &_params):
	Polyconic(_params)
{
}


CoordLL FromPolyconic::execute(const CoordXY &input) const
{
	const double EPSILON = 1e-7;

	double x = input.x - FE;
	double y = input.y - FN;
	double lon, lat1;

	if(y == -M0)
	{
		lat1 = 0;
		lon = x / a + lon0;
	}
	else
	{
		double A = (M0 + y) / a;
		double B = pow2(x) / pow2(a) + pow2(A);

		lat1 = A;
		double lat2;

		double C;

		do
		{
			lat2 = lat1;

			C = sqrt(1 - e2 * pow2(sin(lat1))) * tan(lat1);
			double M1 = a * (
				(1 - e2 / 4 -  3 * pow2(e2) /  64 -  5 * pow3(e2) /  256) * lat1 -
				(3 * e2 / 8 +  3 * pow2(e2) /  32 + 45 * pow3(e2) / 1024) * sin(2 * lat1) +
				(             15 * pow2(e2) / 256 + 45 * pow3(e2) / 1024) * sin(4 * lat1) -
				(                                   35 * pow3(e2) / 3072) * sin(6 * lat1));
			double M1_ =
				1 -      e2 / 4 -  3 * pow2(e2) /  64 -  5 * pow3(e2) /  256 -
				2 * (3 * e2 / 8 +  3 * pow2(e2) /  32 + 45 * pow3(e2) / 1024) * cos(2 * lat1) +
				4 * (             15 * pow2(e2) / 256 + 45 * pow3(e2) / 1024) * cos(4 * lat1) -
			 	6 * (                                   35 * pow3(e2) / 3072) * cos(6 * lat1);
			double Ma = M1 / a;

			lat1 = lat1 - (A * (C * Ma + 1) - Ma - 0.5 * (pow2(Ma) + B) * C) /
				(e2 * sin(2*lat1) * (pow2(Ma) + B - 2 * A * Ma) / 4 * C + (A - Ma) * (C * M1_ - 2 / sin(2 * lat1)) - M1_);
		}
		while(fabs(lat2 - lat1) > EPSILON);

		lon = asin(x * C / a) / sin(lat1) + lon0;   // NB: the C parameter is the last one calculated
	}

	return CoordLL(todeg(lat1), todeg(lon) + pm);
}


TransverseMercator::TransverseMercator(const std::map<ProjParam, double> &_params):
	params(_params)
{
	e2   = params.find(pparE2)->second;
	a    = params.find(pparA)->second;
	pm   = params.find(pparPM)->second;
	lat0 = torad(params.find(pparLat0)->second);
	lon0 = torad(params.find(pparLon0)->second);
	k0   = params.find(pparK)->second;
	FE   = params.find(pparFE)->second;
	FN   = params.find(pparFN)->second;

	e_2 = e2 / (1 - e2);
	M0 = a * (
		(1 - e2 / 4 -  3 * pow2(e2) /  64 -  5 * pow3(e2) /  256) * lat0 -
		(3 * e2 / 8 +  3 * pow2(e2) /  32 + 45 * pow3(e2) / 1024) * sin(2 * lat0) +
		(             15 * pow2(e2) / 256 + 45 * pow3(e2) / 1024) * sin(4 * lat0) -
		(                                   35 * pow3(e2) / 3072) * sin(6 * lat0));
	e1 = (1 - sqrt(1 - e2)) / (1 + sqrt(1 - e2));
}


ToTransverseMercator::ToTransverseMercator(const std::map<ProjParam, double> &_params):
	TransverseMercator(_params)
{
}


CoordXY ToTransverseMercator::execute(const CoordLL &input) const
{
	double lat = torad(input.lat);
	double lon = torad(input.lon - pm);
	lon = normalizeLonRad(lon, lon0);

	double T = pow2(tan(lat));
	double C = e2 * pow2(cos(lat)) / (1 - e2);
	double A = (lon - lon0) * cos(lat);
	double nu = a / sqrt(1 - e2 * pow2(sin(lat)));
	double M = a * (
		(1 - e2 / 4 -  3 * pow2(e2) /  64 -  5 * pow3(e2) /  256) * lat -
		(3 * e2 / 8 +  3 * pow2(e2) /  32 + 45 * pow3(e2) / 1024) * sin(2 * lat) +
		(             15 * pow2(e2) / 256 + 45 * pow3(e2) / 1024) * sin(4 * lat) -
		(                                   35 * pow3(e2) / 3072) * sin(6 * lat));

	double E = FE + k0 * nu * (
		A + 
		(1 -      T +                C           ) * pow3(A) /   6 + 
		(5 - 18 * T + pow2(T) + 72 * C - 58 * e_2) * pow5(A) / 120);
	double N = FN + k0 * (M - M0 + nu * tan(lat) * (
		pow2(A) / 2 +
		(5 -       T +             9 * C + 4 * pow2(C)) * pow4(A) / 24 +
		(61 - 58 * T + pow2(T) + 600 * C - 330 * e_2  ) * pow6(A) / 720));

	return CoordXY(E, N);
}


FromTransverseMercator::FromTransverseMercator(const std::map<ProjParam, double> &_params):
	TransverseMercator(_params)
{
}


CoordLL FromTransverseMercator::execute(const CoordXY &input) const
{
	const double EPSILON = 1e-9;

	double E = input.x;
	double N = input.y;

	double M1 = M0 + (N - FN) / k0;
	double mu1 = M1 / (a * (1 - e2 / 4 - 3 * pow2(e2) / 64 - 5 * pow3(e2) / 256));

	double lat1 = mu1 + 
		(3 * e1 / 2 -  27 * pow3(e1) / 32                      ) * sin(2 * mu1) +
		(              21 * pow2(e1) / 16 - 55 * pow4(e1) /  32) * sin(4 * mu1) +
		(             151 * pow3(e1) / 96                      ) * sin(6 * mu1) +
		(                                 1097 * pow4(e1) / 512) * sin(8 * mu1);

	double nu1 = a / sqrt(1 - e2 * pow2(sin(lat1)));
	double rho1 = a * (1 - e2) / pow(1 - e2 * pow2(sin(lat1)), 1.5);

	double T1 = pow2(tan(lat1));
	double C1 = e_2 * pow2(cos(lat1));
	double D = (E - FE) / (nu1 * k0);

	double lat = lat1 - (nu1 * tan(lat1) / rho1) * (
		pow2(D) / 2 - 
		( 5 +  3 * T1 +  10 * C1 -                 4 * pow2(C1) -   9 * e_2) * pow4(D) /  24 +
		(61 + 90 * T1 + 298 * C1 + 45 * pow2(T1) - 3 * pow2(C1) - 252 * e_2) * pow6(D) / 720);
	double lon = lon0 + (
		D - 
		(1 +     C1 +  2 * T1                                         ) * pow3(D) / 6 + 
		(5 - 2 * C1 + 28 * T1 - 3 * pow2(C1) + 8 * e_2 + 24 * pow2(T1)) * pow5(D) / 120
		) / cos(lat1);

	lon = normalizeLonRad(lon, 0);
	return CoordLL(todeg(lat), todeg(lon) + pm);
}


PreussischePolyeder::PreussischePolyeder(const std::map<ProjParam, double> &_params):
	params(_params)
{
	e2   = params.find(pparE2)->second;
	a    = params.find(pparA)->second;
	pm   = params.find(pparPM)->second;
	lat0 = torad(params.find(pparLat0)->second);
	lon0 = torad(params.find(pparLon0)->second);
	k0   = params.find(pparK)->second;
	FE   = params.find(pparFE)->second;
	FN   = params.find(pparFN)->second;
}


ToPreussischePolyeder::ToPreussischePolyeder(const std::map<ProjParam, double> &_params):
	PreussischePolyeder(_params)
{
}


CoordXY ToPreussischePolyeder::execute(const CoordLL &input) const
{
	double lat = torad(input.lat);
	double lon = torad(input.lon - pm);
	lon = normalizeLonRad(lon, lon0);

	double E = FE + (lon - lon0) * a * cos(lat);
	double N = FN + (lat - lat0) * a;

	return CoordXY(E, N);
}


FromPreussischePolyeder::FromPreussischePolyeder(const std::map<ProjParam, double> &_params):
	PreussischePolyeder(_params)
{
}


CoordLL FromPreussischePolyeder::execute(const CoordXY &input) const
{
	double E = input.x;
	double N = input.y;

	double lat = (N - FN) / a + lat0;
	double lon = (E - FE) / (a * cos(lat)) + lon0;

	lon = normalizeLonRad(lon, 0);
	return CoordLL(todeg(lat), todeg(lon) + pm);
}


CoordLL DatumTrans::execute(const CoordLL &input) const
{
	double lat1 = torad(input.lat);
	double lambda1 = torad(input.lon + pm1);

	double N = ell1.a() / sqrt(1 - ell1.e2() * pow2(sin(lat1)));
	double h = 0;
	CoordXYZ xyz1, xyz2;
	
	xyz1.x = (N + h) * cos(lat1) * cos(lambda1);
	xyz1.y = (N + h) * cos(lat1) * sin(lambda1);
	xyz1.z = (N * (1 - ell1.e2()) + h) * sin(lat1);

	//xyz2.x = xyz1.x + tx + scale * xyz1.x -    rz * xyz1.y +    ry * xyz1.z;
	//xyz2.y = xyz1.y + ty + rz    * xyz1.x + scale * xyz1.y -    rx * xyz1.z;
	//xyz2.z = xyz1.z + tz - ry    * xyz1.x +    rx * xyz1.y + scale * xyz1.z;
	double M = 1.0 + scale * 10e-6;
	xyz2.x = M * (      xyz1.x - rz * xyz1.y + ry * xyz1.z) + tx;
	xyz2.y = M * ( rz * xyz1.x +      xyz1.y - rx * xyz1.z) + ty;
	xyz2.z = M * (-ry * xyz1.x + rx * xyz1.y +      xyz1.z) + tz;

	double r = sqrt(pow2(xyz2.x) + pow2(xyz2.y));
	double theta = atan((xyz2.z * ell2.a()) / (r * ell2.b()));
	
	double lat2 = atan((xyz2.z + ell2.epsilon2() * ell2.b() *
		pow3(sin(theta))) / (r - ell2.e2() * ell2.a() * pow3(cos(theta))));
	double lambda2 = atan2(xyz2.y, xyz2.x);
	CoordLL ll(todeg(lat2), todeg(lambda2) - pm2);

	//ll.lon = normalizeLonRad(ll.lon, input.lon);
	ll.lon = normalizeLonDeg(ll.lon, input.lon + pm1 - pm2);
	
	return ll;
}
