// ParseWKTTest.cpp
// FS, 13-07-2007

#include "ParseWKTTest.h"

#define showVal(val) std::cout << #val " = " << (val) << std::endl
#define show(val) #val ": " << (val)

void showLL(const CoordLL &ll)
{
	std::cout << int(ll.lat) << "° " << (ll.lat - int(ll.lat)) * 60 << "', " << int(ll.lon) << "° " << (ll.lon - int(ll.lon)) * 60;
	//std::cout << ll.lat * 3600 << ", " << ll.lon * 3600;
}


int main(int argc, char *argv[])
{
	std::cout.precision(10);

	//const char WKT_CRS[] = "PROJCS[\"Amersfoort / RD New\",GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128],TOWGS84[565.04,49.91,465.84,-1.9848e-6,1.7439e-6,-9.0587e-6,4.0772e-6]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.01745329251994328]],PROJECTION[\"Oblique_Stereographic\"],PARAMETER[\"latitude_of_origin\",52.15616055555555],PARAMETER[\"central_meridian\",5.38763888888889],PARAMETER[\"scale_factor\",0.9999079],PARAMETER[\"false_easting\",155000],PARAMETER[\"false_northing\",463000],UNIT[\"metre\",1]]";
	const char WKT_CRS[] = "PROJCS[\"Amersfoort / RD New\",GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128],TOWGS84[565.04,49.91,465.84,-1.9848e-6,1.7439e-6,-9.0587e-6,4.0772e-6]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.01745329251994328]],PROJECTION[\"Oblique_Stereographic\"],PARAMETER[\"latitude_of_origin\",52.15616055555555],PARAMETER[\"central_meridian\",5.38763888888889],PARAMETER[\"scale_factor\",0.9999079],PARAMETER[\"false_easting\",155000],PARAMETER[\"false_northing\",463000],UNIT[\"metre\",1]]";

	//const char WKT_CRS[] = "GEOGCS[\"OSGB 1936\",DATUM[\"OSGB_1936\",SPHEROID[\"Airy 1830\",6377563.396,299.3249646,AUTHORITY[\"EPSG\",\"7001\"]],TOWGS84[375,-111,431,0,0,0,0],AUTHORITY[\"EPSG\",\"6277\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4277\"]]";
	//const char WKT_CRS[] = "PROJCS[\"OSGB 1936 / British National Grid\",GEOGCS[\"OSGB 1936\",DATUM[\"OSGB_1936\",SPHEROID[\"Airy 1830\",6377563.396,299.3249646,AUTHORITY[\"EPSG\",\"7001\"]],TOWGS84[375,-111,431,0,0,0,0],AUTHORITY[\"EPSG\",\"6277\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4277\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",49],PARAMETER[\"central_meridian\",-2],PARAMETER[\"scale_factor\",0.999601272],PARAMETER[\"false_easting\",400000],PARAMETER[\"false_northing\",-100000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"E\",EAST],AXIS[\"N\",NORTH],AUTHORITY[\"EPSG\",\"27700\"]],";
	//const char WKT_CRS[] = "COMPD_CS[\"OSGB36 / British National Grid + ODN\",PROJCS[\"OSGB 1936 / British National Grid\",GEOGCS[\"OSGB 1936\",DATUM[\"OSGB_1936\",SPHEROID[\"Airy 1830\",6377563.396,299.3249646,AUTHORITY[\"EPSG\",\"7001\"]],TOWGS84[375,-111,431,0,0,0,0],AUTHORITY[\"EPSG\",\"6277\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4277\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",49],PARAMETER[\"central_meridian\",-2],PARAMETER[\"scale_factor\",0.999601272],PARAMETER[\"false_easting\",400000],PARAMETER[\"false_northing\",-100000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"E\",EAST],AXIS[\"N\",NORTH],AUTHORITY[\"EPSG\",\"27700\"]],VERT_CS[\"Newlyn\",VERT_DATUM[\"Ordnance Datum Newlyn\",2005,AUTHORITY[\"EPSG\",\"5101\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Up\",UP],AUTHORITY[\"EPSG\",\"5701\"]],AUTHORITY[\"EPSG\",\"7405\"]]";



	// Pulkovo
	//const char WKT_CRS[] = "PROJCS[\"Pulkovo UTM zone 32\",GEOGCS[\"Pulkovo 1942\",DATUM[\"Pulkovo 1942\",SPHEROID[\"Krassowsky 1940\",6378245,298.3,AUTHORITY[\"EPSG\",\"7024\"]],TOWGS84[28,-130,-95,0,0,0,0],AUTHORITY[\"EPSG\",\"6284\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4284\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",9],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"E\",EAST],AXIS[\"N\",NORTH],AUTHORITY[\"FS\",\"0\"]]";
	//const char WKT_CRS[] = "PROJCS[\"Pulkovo UTM zone 18\",GEOGCS[\"Pulkovo 1942\",DATUM[\"Pulkovo 1942\",SPHEROID[\"Krassowsky 1940\",6378245,298.3,AUTHORITY[\"EPSG\",\"7024\"]],TOWGS84[28,-130,-95,0,0,0,0],AUTHORITY[\"EPSG\",\"6284\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4284\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-75],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"E\",EAST],AXIS[\"N\",NORTH],AUTHORITY[\"FS\",\"0\"]]";
	//const char WKT_CRS[] = "PROJCS[\"Pulkovo UTM zone 1\",GEOGCS[\"Pulkovo 1942\",DATUM[\"Pulkovo 1942\",SPHEROID[\"Krassowsky 1940\",6378245,298.3,AUTHORITY[\"EPSG\",\"7024\"]],TOWGS84[28,-130,-95,0,0,0,0],AUTHORITY[\"EPSG\",\"6284\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4284\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-177],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"E\",EAST],AXIS[\"N\",NORTH],AUTHORITY[\"FS\",\"0\"]]";
	//const char WKT_CRS[] = "PROJCS[\"Pulkovo UTM zone 60\",GEOGCS[\"Pulkovo 1942\",DATUM[\"Pulkovo 1942\",SPHEROID[\"Krassowsky 1940\",6378245,298.3,AUTHORITY[\"EPSG\",\"7024\"]],TOWGS84[28,-130,-95,0,0,0,0],AUTHORITY[\"EPSG\",\"6284\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4284\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",177],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"E\",EAST],AXIS[\"N\",NORTH],AUTHORITY[\"FS\",\"0\"]]";
	//const char WKT_CRS2[] = "GEOGCS[\"Pulkovo 1942\",DATUM[\"Pulkovo 1942\",SPHEROID[\"Krassowsky 1940\",6378245,298.3,AUTHORITY[\"EPSG\",\"7024\"]],TOWGS84[28,-130,-95,0,0,0,0],AUTHORITY[\"EPSG\",\"6284\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4284\"]]";

	//const char WKT_CRS[] = "PROJCS[\"Lambert Conformal Conic zone 18\",GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Lambert_Conformal_Conic\"],PARAMETER[\"latitude_of_origin\",48],PARAMETER[\"central_meridian\",-75],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"E\",EAST],AXIS[\"N\",NORTH],AUTHORITY[\"FS\",\"0\"]]";
	//const char WKT_CRS[] = "PROJCS[\"Test LCC\",GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378206.400,294.97870,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Lambert_Conformal_Conic\"],PARAMETER[\"latitude_of_origin\",18],PARAMETER[\"central_meridian\",-77],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",250000],PARAMETER[\"false_northing\",150000.00],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"E\",EAST],AXIS[\"N\",NORTH],AUTHORITY[\"FS\",\"0\"]]";
	
	
	//const char WKT_CRS[] = "PROJCS[\"Bonne\",GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128],TOWGS84[565.04,49.91,465.84,-1.9848e-6,1.7439e-6,-9.0587e-6,4.0772e-6]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Bonne\"],PARAMETER[\"latitude_of_origin\",51.5],PARAMETER[\"central_meridian\",4.883883008],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",12],PARAMETER[\"false_northing\",-60],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//const char WKT_CRS[] = "PROJCS[\"Bonne\",GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128],TOWGS84[0,0,0,0,0,0,0]],PRIMEM[\"Amsterdam\",4.883883008],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Bonne\"],PARAMETER[\"latitude_of_origin\",51.5],PARAMETER[\"central_meridian\",0],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//const char WKT_AMERSFOORT[] = "GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128],TOWGS84[0,0,0,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]]";

	//const char WKT_CRS[] = "PROJCS[\"WGS 84 / Geneaknowhow.net\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",51.443053],PARAMETER[\"central_meridian\",4.930056],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",300000],PARAMETER[\"false_northing\",750000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"GKH\",\"1\"]]";
	//const char WKT_CRS[] = "PROJCS[\"Amersfoort / RD New\",GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128,AUTHORITY[\"EPSG\",\"7004\"]],AUTHORITY[\"EPSG\",\"6289\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4289\"]],PROJECTION[\"Oblique_Stereographic\"],PARAMETER[\"latitude_of_origin\",52.15616055555555],PARAMETER[\"central_meridian\",5.38763888888889],PARAMETER[\"scale_factor\",0.9999079],PARAMETER[\"false_easting\",155000],PARAMETER[\"false_northing\",463000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"28992\"]]";
	//const char WKT_CRS[] = "PROJCS[\"Amersfoort / RD New\",GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128,AUTHORITY[\"EPSG\",\"7004\"]],AUTHORITY[\"EPSG\",\"6289\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4289\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],PROJECTION[\"Oblique_Stereographic\"],PARAMETER[\"latitude_of_origin\",52.15616055555555],PARAMETER[\"central_meridian\",5.38763888888889],PARAMETER[\"scale_factor\",0.9999079],PARAMETER[\"false_easting\",155000],PARAMETER[\"false_northing\",463000],AUTHORITY[\"EPSG\",\"28992\"]]";
	//const char WKT_CRS[] = "PROJCS[\"Amersfoort / RD New\",GEOGCS[\"Amersfoort\",DATUM[\"D_Amersfoort\",SPHEROID[\"Bessel_1841\",6377397.155,299.1528128]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Stereographic_North_Pole\"],PARAMETER[\"standard_parallel_1\",52.15616055555555],PARAMETER[\"central_meridian\",5.38763888888889],PARAMETER[\"scale_factor\",0.9999079],PARAMETER[\"false_easting\",155000],PARAMETER[\"false_northing\",463000],UNIT[\"Meter\",1]]";

	//const char WKT_CRS[] = "PROJCS[\"X\",GEOGCS[\"X\",DATUM[\"Pulkovo 1942 (2)\",SPHEROID[\"X\",6378245.0000000000,298.3000000000],TOWGS84[28.0000000000,-130.0000000000,-95.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000]],PRIMEM[\"Greenwich\",0,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0.0000000000],PARAMETER[\"central_meridian\",-177.0000000000],PARAMETER[\"scale_factor\",1.0000000000],PARAMETER[\"false_easting\",500000.0000000000],PARAMETER[\"false_northing\",0.0000000000],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//const char WKT_CRS[] = "PROJCS[\"X\",GEOGCS[\"X\",DATUM[\"WGS 84\",SPHEROID[\"X\",6378137.0000000000,298.2572235630],TOWGS84[0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000,0.0000000000]],PRIMEM[\"Paris\",2.33722916,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Bonne\"],PARAMETER[\"latitude_of_origin\",51.5000000000],PARAMETER[\"central_meridian\",2.5466538410],PARAMETER[\"scale_factor\",1.0000000000],PARAMETER[\"false_easting\",0.0000000000],PARAMETER[\"false_northing\",0.0000000000],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//const char WKT_CRS[] = "GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Ferro\",-17.666667,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]";

	//const char WKT_CRS[] = "GEOGCS[\"DHDN\",DATUM[\"Deutsches_Hauptdreiecksnetz\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128],TOWGS84[582.0,105.0,414.0,5.0421e-6,1.6968e-6,-14.9323e-6,8.3]],PRIMEM[\"Ferro\",-17.666667],UNIT[\"degree\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]]";

	//const char WKT_CRS2[] = "PROJCS[\"Mercator_WGS84\",GEOGCS[\"WGS84\",DATUM[\"WGS84\",SPHEROID[\"WGS84\",6378137,298.257223563],TOWGS84[0,0,0,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]]PROJECTION[\"Mercator\"],PARAMETER[\"central_meridian\",0],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	
	//const char WKT_CRS[] = "GEOGCS[\"Undefined\",DATUM[\"Undefined\",SPHEROID[\"Clarke 1866\",6378206.4,294.9786982139006]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433]]";
	//const char WKT_CRS2[] = "PROJCS[\"Synder Polyconic test\",GEOGCS[\"Undefined\",DATUM[\"Undefined\",SPHEROID[\"Clarke 1866\",6378206.4,294.9786982139006]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433]],PROJECTION[\"Polyconic\"],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],PARAMETER[\"central_meridian\",-96],PARAMETER[\"latitude_of_origin\",30],UNIT[\"metre\",1]]";

	//const char WKT_CRS[] = "GEOGCS[\"Undefined\",DATUM[\"Undefined\",SPHEROID[\"Clarke 1866\",6378206.4,294.9786982139006]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433]]";
	//const char WKT_CRS2[] = "PROJCS[\"World_Polyconic 006D05M\",GEOGCS[\"Undefined\",DATUM[\"Undefined\",SPHEROID[\"Clarke 1866\",6378206.4,294.9786982139006]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433]],PROJECTION[\"Polyconic\"],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],PARAMETER[\"central_meridian\",6.083333],PARAMETER[\"latitude_of_origin\",0],UNIT[\"metre\",1]]";

	//const char WKT_CRS[] = "PROJCS[\"DHDN / Gauss-Kruger zone 3\",GEOGCS[\"DHDN\",DATUM[\"Deutsches_Hauptdreiecksnetz\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128,AUTHORITY[\"EPSG\",\"7004\"]],AUTHORITY[\"EPSG\",\"6314\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4314\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",9],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",3500000],PARAMETER[\"false_northing\",0],AUTHORITY[\"EPSG\",\"31467\"],AXIS[\"Y\",EAST],AXIS[\"X\",NORTH]]";
	//const char WKT_CRS[] = "PROJCS[\"X\",GEOGCS[\"X\",DATUM[\"X\",SPHEROID[\"Bessel\",6377397.155,299.1528128],TOWGS84[584.50,160.69,411.69,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",21],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//const char WKT_CRS2[] = "GEOGCS[\"X\",DATUM[\"X\",SPHEROID[\"Bessel\",6377397.155,299.1528128],TOWGS84[584.50,160.69,411.69,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]]";

	// Roemenie
	//const char WKT_CRS[] = "GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]";
	//const char WKT_CRS[] = "GEOGCS[\"X\",DATUM[\"Clarke 1880\",SPHEROID[\"X\",6378249.2,293.4660213],TOWGS84[-118.46,-163.65,287.58,0,0,0,0]],PRIMEM[\"Greenwich\",0,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]]";
	//const char WKT_CRS[] = "GEOGCS[\"Clarke 1866\",DATUM[\"Test2\",SPHEROID[\"Clarke 1866\",6378206.4,294.9786982139006],TOWGS84[-30,-48,105,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433]]";
	//const char WKT_CRS2[] = "PROJCS[\"Lambert-Cholesky\",GEOGCS[\"X\",DATUM[\"Clarke 1880\",SPHEROID[\"X\",6378249.2,293.4660213],TOWGS84[-118.46,-163.65,287.58,0,0,0,0]],PRIMEM[\"Greenwich\",0,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Lambert_Conformal_Conic_1SP\"],PARAMETER[\"latitude_of_origin\",45.04144888],PARAMETER[\"central_meridian\",24.31249722],PARAMETER[\"scale_factor\",0.99844674],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",504599.11],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//const char WKT_CRS2[] = "PROJCS[\"DanubeZone\",GEOGCS[\"X\",DATUM[\"X\",SPHEROID[\"Bessel\",6377397.155,299.1528128],TOWGS84[1058.57,771.97,-200.27,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Lambert_Conformal_Conic_1SP\"],PARAMETER[\"latitude_of_origin\",45.9],PARAMETER[\"central_meridian\",29],PARAMETER[\"scale_factor\",0.998992911],PARAMETER[\"false_easting\",1500000],PARAMETER[\"false_northing\",600001],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";

	// Frankrijk
	//const char WKT_CRS2[] = "PROJCS[\"NTF (Paris) / Lambert Nord France\",GEOGCS[\"NTF (Paris)\",DATUM[\"Nouvelle_Triangulation_Francaise_Paris\",SPHEROID[\"Clarke 1880 (IGN)\",6378249.2,293.4660212936269,AUTHORITY[\"EPSG\",\"7011\"]],TOWGS84[-168,-60,320,0,0,0,0],AUTHORITY[\"EPSG\",\"6807\"]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4807\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],PROJECTION[\"Lambert_Conformal_Conic_1SP\"],PARAMETER[\"latitude_of_origin\",49.5],PARAMETER[\"central_meridian\",2.33722917],PARAMETER[\"scale_factor\",0.999877341],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",200000],AUTHORITY[\"EPSG\",\"27561\"],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH]]";
	//const char WKT_CRS2[] = "PROJCS[\"NTF (Paris) / Lambert Centre France\",GEOGCS[\"NTF (Paris)\",DATUM[\"Nouvelle_Triangulation_Francaise_Paris\",SPHEROID[\"Clarke 1880 (IGN)\",6378249.2,293.4660212936269,AUTHORITY[\"EPSG\",\"7011\"]],TOWGS84[-168,-60,320,0,0,0,0],AUTHORITY[\"EPSG\",\"6807\"]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4807\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],PROJECTION[\"Lambert_Conformal_Conic_1SP\"],PARAMETER[\"latitude_of_origin\",46.8],PARAMETER[\"central_meridian\",2.33722917],PARAMETER[\"scale_factor\",0.99987742],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",200000],AUTHORITY[\"EPSG\",\"27562\"],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH]]";
	//const char WKT_CRS2[] = "PROJCS[\"NTF (Paris) / Lambert Sud France\",GEOGCS[\"NTF (Paris)\",DATUM[\"Nouvelle_Triangulation_Francaise_Paris\",SPHEROID[\"Clarke 1880 (IGN)\",6378249.2,293.4660212936269,AUTHORITY[\"EPSG\",\"7011\"]],TOWGS84[-168,-60,320,0,0,0,0],AUTHORITY[\"EPSG\",\"6807\"]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4807\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],PROJECTION[\"Lambert_Conformal_Conic_1SP\"],PARAMETER[\"latitude_of_origin\",44.1],PARAMETER[\"central_meridian\",2.33722917],PARAMETER[\"scale_factor\",0.999877499],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",200000],AUTHORITY[\"EPSG\",\"27563\"],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH]]";
	//const char WKT_CRS2[] = "GEOGCS[\"Clarke 1866\",DATUM[\"Test2\",SPHEROID[\"Clarke 1866\",6378206.4,294.9786982139006],TOWGS84[-30,-48,105,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433]]";
	//const char WKT_CRS2[] = "PROJCS[\"NordDeGuerre\",GEOGCS[\"X\",DATUM[\"WGS 84\",SPHEROID[\"DuPlessis\",6376523,308.64],TOWGS84[1383,44,454,0,0,0,0]],PRIMEM[\"Greenwich\",0,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Lambert_Conformal_Conic_1SP\"],PARAMETER[\"latitude_of_origin\",49.50],PARAMETER[\"central_meridian\",7.7372083330],PARAMETER[\"scale_factor\",0.999509081],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",300000],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//const char WKT_CRS2[] = "PROJCS[\"NTF (Paris) / Lambert Nord France\",GEOGCS[\"NTF (Paris)\",DATUM[\"Nouvelle_Triangulation_Francaise_Paris\",SPHEROID[\"Clarke 1880 (IGN)\",6378249.2,293.4660212936269,AUTHORITY[\"EPSG\",\"7011\"]],TOWGS84[-168,-60,320,0,0,0,0],AUTHORITY[\"EPSG\",\"6807\"]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AUTHORITY[\"EPSG\",\"4807\"]],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],PROJECTION[\"Lambert_Conformal_Conic_1SP\"],PARAMETER[\"latitude_of_origin\",49.5],PARAMETER[\"central_meridian\",2.33722917],PARAMETER[\"scale_factor\",0.999877341],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",200000],AUTHORITY[\"EPSG\",\"27561\"],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH]]";

	// Lambert zones o.b.v. andere ellipsoides
	//const char WKT_CRS[] = "PROJCS[\"Lambert II test\",GEOGCS[\"Test\",DATUM[\"Test2\",SPHEROID[\"Clarke 1866\",6378206.4,294.9786982139006],TOWGS84[-168,-60,320,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433]],PROJECTION[\"Lambert_Conformal_Conic_1SP\"],PARAMETER[\"latitude_of_origin\",46.8],PARAMETER[\"central_meridian\",2.33722917],PARAMETER[\"scale_factor\",0.99987742],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",200000],AUTHORITY[\"EPSG\",\"27562\"],AXIS[\"X\",EAST],AXIS[\"Y\",NORTH]]";
	//const char WKT_CRS[] = "PROJCS[\"BonneATF\",GEOGCS[\"ATF\",DATUM[\"ATF\",SPHEROID[\"Clarke 1880 (IGN)\",6378249.2,293.4660212936269],TOWGS84[-168,-60,320,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Bonne\"],PARAMETER[\"latitude_of_origin\",45],PARAMETER[\"central_meridian\",2.33722917],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",300000],PARAMETER[\"scale_factor\",0.99987742],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//const char WKT_CRS[] = "PROJCS[\"BonneNTF\",GEOGCS[\"NTF (Paris)\",DATUM[\"NTF (Paris)\",SPHEROID[\"Clarke 1880 (IGN)\",6378249.2,293.4660212936269],TOWGS84[-168,-60,320,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Bonne\"],PARAMETER[\"latitude_of_origin\",45],PARAMETER[\"central_meridian\",2.33722917],PARAMETER[\"scale_factor\",0.99987742],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",300000],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//const char WKT_CRS[] = "PROJCS[\"BonneATF\",GEOGCS[\"ATF\",DATUM[\"ATF\",SPHEROID[\"DuPlessis\",6376523,308.64],TOWGS84[1383,44,454,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Bonne\"],PARAMETER[\"latitude_of_origin\",45],PARAMETER[\"central_meridian\",2.33722917],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",300000],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";

	// Iberisch schiereiland
	//TOWGS84[-30,-48,105,0,0,0,0]
	//const char WKT_CRS[] = "GEOGCS[\"X\",DATUM[\"Hayford\",SPHEROID[\"Hayford\",6378388,297.0],TOWGS84[0,0,0,0,0,0,0]],PRIMEM[\"Greenwich\",0,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]]";
	//const char WKT_CRS2[] = "PROJCS[\"X\",GEOGCS[\"X\",DATUM[\"Hayford\",SPHEROID[\"Hayford\",6378388,297.0],TOWGS84[0,0,0,0,0,0,0]],PRIMEM[\"Greenwich\",0,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Lambert_Conformal_Conic_1SP\"],PARAMETER[\"latitude_of_origin\",40.0],PARAMETER[\"central_meridian\",-3.687375],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",530000],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//const char WKT_CRS2[] = "PROJCS[\"X\",GEOGCS[\"X\",DATUM[\"Hayford\",SPHEROID[\"Hayford\",6378388,297.0],TOWGS84[-30,-48,105,0,0,0,0]],PRIMEM[\"Greenwich\",0,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Lambert_Conformal_Conic_1SP\"],PARAMETER[\"latitude_of_origin\",40.0],PARAMETER[\"central_meridian\",-3.687375],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",600000],PARAMETER[\"false_northing\",530000],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";

	// Cassini-Soldner
	//const char WKT_CRS[] = "PROJCS[\"Catast_MonteCastelluccio\",GEOGCS[\"GCS_MonteCastelluccio\",DATUM[\"<custom>\",SPHEROID[\"Bessel_1841\",6377397.155,299.1528128]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Cassini\"],PARAMETER[\"False_Easting\",0.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",13.77912777777778],PARAMETER[\"Scale_Factor\",1.0],PARAMETER[\"Latitude_Of_Origin\",37.41457777777778],UNIT[\"Meter\",1.0]]";
	//const char WKT_CRS[] = "PROJCS[\"Trinidad 1903 / Trinidad Grid\",GEOGCS[\"Trinidad 1903\",DATUM[\"Trinidad_1903\",SPHEROID[\"Clarke 1858\",6378293.645208759,294.2606763692654,AUTHORITY[\"EPSG\",\"7007\"]],AUTHORITY[\"EPSG\",\"6302\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4302\"]],UNIT[\"Clarke's link\",0.201166195164,AUTHORITY[\"EPSG\",\"9039\"]],PROJECTION[\"Cassini_Soldner\"],PARAMETER[\"latitude_of_origin\",10.44166666666667],PARAMETER[\"central_meridian\",-61.33333333333334],PARAMETER[\"false_easting\",430000],PARAMETER[\"false_northing\",325000],AUTHORITY[\"EPSG\",\"30200\"],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]";
	//const char WKT_CRS[] = "PROJCS[\"Trinidad 1903 / Trinidad Grid\",GEOGCS[\"Trinidad 1903\",DATUM[\"Trinidad_1903\",SPHEROID[\"Clarke 1858\",6378293.645208759,294.2606763692654,AUTHORITY[\"EPSG\",\"7007\"]],AUTHORITY[\"EPSG\",\"6302\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4302\"]],UNIT[\"Clarke's link\",0.201166195164,AUTHORITY[\"EPSG\",\"9039\"]],PROJECTION[\"Cassini_Soldner\"],PARAMETER[\"latitude_of_origin\",10.44166666666667],PARAMETER[\"central_meridian\",-61.33333333333334],PARAMETER[\"false_easting\",86501.46392052],PARAMETER[\"false_northing\",65379.0134283],AUTHORITY[\"EPSG\",\"30200\"],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]";
	//const char WKT_CRS2[] = "GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]";

	try {

	//std::cout << "test 1" << std::endl;
	boost::shared_ptr<CoordinateSystem> cs = CoordinateSystem::CreateCS(WKT_CRS);
	//boost::shared_ptr<CoordinateSystem> cs2 = CoordinateSystem::CreateCS(WKT_CRS2);
	//boost::shared_ptr<CoordinateSystem> csAmersfoort = CoordinateSystem::CreateCS(WKT_AMERSFOORT);
	//std::cout << "test 2" << std::endl;

	boost::shared_ptr<ProjectedCS> pcs = boost::static_pointer_cast<ProjectedCS>(cs);
	//boost::shared_ptr<GeographicCS> gcs = boost::static_pointer_cast<GeographicCS>(cs2);
	//boost::shared_ptr<ProjectedCS> pcs2 = boost::static_pointer_cast<ProjectedCS>(cs2);
	//boost::shared_ptr<GeographicCS> gcs_Amersfoort = boost::static_pointer_cast<GeographicCS>(csAmersfoort);
	
	if(pcs != 0)
	//if(gcs != 0)
	{
		boost::shared_ptr<XYTrans<CoordLL, CoordXY>> toProj = g_csWGS84->getTransformation(pcs);
		boost::shared_ptr<XYTrans<CoordXY, CoordLL>> fromProj = pcs->getTransformation(g_csWGS84);
		//boost::shared_ptr<XYTrans<CoordLL, CoordLL>> toProj2 = g_csWGS84->getTransformation(gcs);
		//boost::shared_ptr<XYTrans<CoordLL, CoordLL>> fromProj2 = gcs->getTransformation(g_csWGS84);
		//boost::shared_ptr<XYTrans<CoordLL, CoordXY>> toProj = gcs_Amersfoort->getTransformation(pcs);
		//boost::shared_ptr<XYTrans<CoordXY, CoordLL>> fromProj = pcs->getTransformation(gcs_Amersfoort);
		//boost::shared_ptr<XYTrans<CoordLL, CoordXY>> toProj = gcs->getTransformation(pcs);
		//boost::shared_ptr<XYTrans<CoordXY, CoordLL>> fromProj = pcs->getTransformation(gcs);
		//boost::shared_ptr<XYTrans<CoordXY, CoordXY>> toProj = pcs2->getTransformation(pcs);
		//boost::shared_ptr<XYTrans<CoordXY, CoordXY>> fromProj = pcs->getTransformation(pcs2);

		//CoordLL ll0(52.37, 5.5);
		//CoordLL ll0(52 + 28.0/60.0 + 15.73/3600.0, 2 + 3.0/60.0 + 39.52/3600.0);
		//CoordLL ll0(52 + 14.0/60.0 + 47.17/3600.0, 2 + 3.0/60.0 + 2.0/3600.0);
		//CoordLL ll0(45, -75);
		//CoordLL ll0(17.932166666666666666666666666667, -76.943683333333333333333333333333);
		
		//CoordLL ll0(53.555922, 6.476617);   // Rottemerplaat
		//CoordLL ll0(53.180392, 7.227598);   // Nieuweschans
		//CoordLL ll0(49.448040, 6.041752);   // Rumelange
		//CoordLL ll0(51.088800, 2.545609);   // De Panne

		//CoordLL ll0(50.446027, 1.837978);   // Brimeux (ten zuiden van Calais)
		//CoordLL ll0(50.067387, 1.381316);   // Mers-les-Bains (Picardie)
		//CoordLL ll0(49.169359, 6.047802);   // Amanvillers (ten n.w. van Metz)
		//CoordLL ll0(53.571307, 8.120270);   // Wilhelmshaven
		//CoordLL ll0(53.540919, 8.171425);   // Wilhelmshaven 2
		//CoordLL ll0(53.794973, 7.885780);   // Wangerooge

		//CoordLL ll0(66, -174);
		//CoordLL ll0(51.5, 2.54);
		//CoordLL ll0(52.5, 7);
		//CoordLL ll0(52.5, 24.35);
		//CoordLL ll0(40, -75);
		//CoordLL ll0(52, 6);

		if (argc < 3)
		{
			std::cerr << "Geef x,y / lat,lon op" << std::endl;
			return -1;
		}

		//CoordLL ll0(atof(argv[1]), atof(argv[2]));
		CoordXY xy(atof(argv[1]), atof(argv[2]));


		////showVal(ll0);

		////CoordLL wgs840 = fromProj->execute(ll0);
		////std::cout << show(ll0) << " >> " << show(wgs840) << std::endl;
		////CoordXY xy0 = toProj2->execute(wgs840);
		////std::cout << show(wgs840) << " >> " << show(xy0) << std::endl;

		////CoordXY xy0(0.1174565527,1.076040421);

		////CoordLL wgs840_ = fromProj2->execute(xy0);
		////std::cout << show(xy0) << " >> " << show(wgs840_) << std::endl;
		////CoordLL ll0_ = toProj->execute(wgs840_);
		////std::cout << show(wgs840_) << " >> " << show(ll0_) << std::endl;

		//CoordLL ll1 = toProj2->execute(ll0);
		//std::cout << show(ll0) << " >> " << show(ll1) << std::endl;

		//CoordXY xy = toProj->execute(ll1);
		//std::cout << show(ll1) << " >> " << show(xy) << std::endl;

		//xy = CoordXY(1776774.5, 1319657.8);
		//CoordLL ll2 = fromProj->execute(xy);
		CoordXY xy1 = fromProj->execute(xy);
		std::cout << show(xy) << " >> " << show(xy1) << std::endl;
		//std::cout << show(xy) << " >> ";
		//showLL(ll2);
		//std::cout << std::endl;

		//CoordLL ll3 = fromProj2->execute(ll2);
		//std::cout << show(ll2) << " >> " << show(ll3) << std::endl;

		//CoordXY xy2 = toProj->execute(ll2);
		//std::cout << show(ll2) << " >> " << show(xy2) << std::endl;
		CoordXY xy2 = toProj->execute(xy1);
		std::cout << show(xy1) << " >> " << show(xy2) << std::endl;

		//delete toProj;
		//delete fromProj;
	}

	//delete cs;

	//char c;
	//std::cin >> c;

	}
	catch (char *e)
	{
		std::cout << "An error has occurred: " << e << std::endl;
	}

	return 0;
}
