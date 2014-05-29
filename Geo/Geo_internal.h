// Geo_internal.h
// FS, 14-07-2007

#ifndef GEO_INTERNAL_H
#define GEO_INTERNAL_H

#include <windows.h>
#include "../../Inc/Geo_wkt.h"


////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////

const double PI = 3.141592653589793;
const double DEGRAD = PI / 180.0;

const std::string g_UNIT_DMSH_ = "UNIT[\"DMSH\",0.0174532925199433]";
const std::string g_PRIMEM_GREENWICH_ = "PRIMEM[\"Greenwich\",0]";

const std::string g_NAME_WGS84_ = "WGS84";
const std::string g_SPHEROID_WGS84_ = "SPHEROID[\"" + g_NAME_WGS84_ + "\",6378137,298.257223563]";
const std::string g_DATUM_WGS84_ = "DATUM[\"" + g_NAME_WGS84_ + "\"," + g_SPHEROID_WGS84_ +
	",TOWGS84[0,0,0,0,0,0,0]]";
const std::string g_GEOGCS_WGS84_ = "GEOGCS[\"" + g_NAME_WGS84_ + "\"," + g_DATUM_WGS84_ + "," +
	g_PRIMEM_GREENWICH_ + "," + g_UNIT_DMSH_ + ",AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]]";
const std::string g_MERCATOR_WGS84_ = "PROJCS[\"Mercator_WGS84\"," + g_GEOGCS_WGS84_ +
	"PROJECTION[\"" + g_PROJ_MERC + "\"],PARAMETER[\"" + g_PARAM_LON0 + "\",0],PARAMETER[\"" + g_PARAM_FE + "\",0],PARAMETER[\"" + g_PARAM_FN + "\",0],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";

const std::string g_PARAM_MT = "PARAM_MT";
const std::string g_PARAMETER = "PARAMETER";
const std::string g_CONCAT_MT = "CONCAT_MT";
const std::string g_INVERSE_MT = "INVERSE_MT";
const std::string g_PASSTHROUGH_MT = "PASSTHROUGH_MT";

const std::string g_PROJCS = "PROJCS";
const std::string g_PROJECTION = "PROJECTION";
const std::string g_GEOGCS = "GEOGCS";
const std::string g_DATUM = "DATUM";
const std::string g_SPHEROID = "SPHEROID";
const std::string g_PRIMEM = "PRIMEM";
const std::string g_UNIT = "UNIT";
const std::string g_GEOCCS = "GEOCCS";
const std::string g_AUTHORITY = "AUTHORITY";
const std::string g_VERT_CS = "VERT_CS";
const std::string g_VERT_DATUM = "VERT_DATUM";
const std::string g_COMPD_CS = "COMPD_CS";
const std::string g_AXIS = "AXIS";
const std::string g_TOWGS84 = "TOWGS84";
const std::string g_FITTED_CS = "FITTED_CS";
const std::string g_LOCAL_CS = "LOCAL_CS";
const std::string g_LOCAL_DATUM =" LOCAL_DATUM";

const std::string g_AXES[] = { "OTHER", "NORTH", "SOUTH", "EAST", "WEST", "UP", "DOWN" };
const int g_NUM_AXES = 7;

const std::string g_DIGITS = "0123456789.-";
const std::string g_IDENTIFIER_CHARS =
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";
const std::string g_IDENTIFIER_STARTCHARS =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_";


////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////

inline double torad(double deg)
{
	return deg * DEGRAD;
}


inline double todeg(double rad)
{
	return rad / DEGRAD;
}


inline double pow2(double value)
{
	return value * value;
}


inline double pow3(double value)
{
	return value * value * value;
}


inline double pow4(double value)
{
	return pow2(pow2(value));
}


inline double pow5(double value)
{
	return pow2(value) * pow3(value);
}


inline double pow6(double value)
{
	return pow3(pow2(value));
}


inline double normalizeLonRad(double lon, double baseLon)
{
	while(lon < baseLon - PI)
		lon += 2 * PI;

	while(lon > baseLon + PI)
		lon -= 2 * PI;

	return lon;
}


inline double normalizeLonDeg(double lon, double baseLon)
{
	while(lon < baseLon - 180.0)
		lon += 360.0;

	while(lon > baseLon + 180.0)
		lon -= 360.0;

	return lon;
}


/*inline double normalizeLon(double lon)
{
	return normalizeLon(lon, 0);
}*/


// Function declarations
void initDLL();

std::string::size_type findMatchingBracket(std::string::size_type pos, const std::string &text);

// In the following functions, the returned pos parameter will always be _after_ the returned value
std::string parseName(std::string::size_type &pos, const std::string &wkt);
double parseDouble(std::string::size_type &pos, const std::string &wkt);
int parseInt(std::string::size_type &pos, const std::string &wkt);
std::string parseIdentifier(std::string::size_type &pos, const std::string &wkt);



#endif // GEO_INTERNAL_H
