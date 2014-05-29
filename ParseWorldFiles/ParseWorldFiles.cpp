// ParseWorldFiles.cpp
// FS, 25-05-2012
// Based on ParseOziMap.cpp

#include "ParseWorldFiles.h"

//#define showVal(val) std::cout << #val " = " << (val) << std::endl


int main(int argc, char *argv[])
{
	//const std::string OZIMAP_DIR = "E:\\Frank\\private\\Maps\\army_map_service_canada_1m\\";

	std::string sDir, sOutputFile;

	FSUtils::CmdApp app(PROGNAME, PROGVERSION);

	FSUtils::CmdArg
		argDir       (FSUtils::CmdArgType::String, &sDir,        "d", "dir",    true),
		argOutputFile(FSUtils::CmdArgType::String, &sOutputFile, "o", "output", true);

	// Projection: can be alias (should be defined in sheet index) or WKT

	argDir.desc         = "world file directory";
	argDir.param        = "DIR";
	argOutputFile.desc  = "output file";
	argOutputFile.param = "OUTPUT";

	app.bindArg(argDir);
	app.bindArg(argOutputFile);


	try
	{
		//uArgList.parseArgs(argc, argv, false);
		app.parseArgs(argc, argv, false);
	}
	catch(FSUtils::CmdApp::RequiredArgumentMissing &exc)
	{
		app.syntax("A required argument is missing: " + exc.name);
		return -1;
	}
	catch(FSUtils::CmdApp::UnknownArgument &exc)
	{
		app.syntax("An unknown option is encountered: " + exc.name);
		return -1;
	}
	catch(FSUtils::CmdApp::UnsupportedArgumentType &)
	{
		app.syntax("An unsupported argument type is encountered");
		return -1;
	}

	if(app.stopExecution())
	{
		// Stop execution. Potential reasons: help / version information requested.
		return 0;
	}


	// Loop through all map files
	WIN32_FIND_DATAA uFindFileData;
	HANDLE hResult;
	BOOL bResult;

	if(sDir[sDir.length() - 1] == '\\')
		sDir = sDir.substr(0, sDir.length() - 1);

	std::string sPattern = sDir + "\\*.??w";
	std::cout << "Pattern: " << sPattern << std::endl;
	std::string sWorldFile;
	std::ofstream fOut;
	std::ostringstream ssOut;

	hResult = FindFirstFileA(sPattern.c_str(), &uFindFileData);
	if(hResult != INVALID_HANDLE_VALUE)
	{
		fOut.open(sOutputFile.c_str());

		fOut << "ProjectionAlias=" << g_DATUM_WGS84 << "|GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]" << std::endl;
		fOut << "ProjectionAlias=GoogleMaps|PROJCS[\"Mercator_WGS84\",GEOGCS[\"WGS84\",DATUM[\"WGS84\",SPHEROID[\"WGS84\",6378137,298.257223563],TOWGS84[0,0,0,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Mercator\"],PARAMETER[\"central_meridian\",0],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]" << std::endl;
		fOut << std::endl;

		do
		{
			sWorldFile = sDir + "\\" + uFindFileData.cFileName;
			std::cout << sWorldFile << std::endl;
			parseWorldFile(sDir, sWorldFile, fOut, ssOut);

			bResult = FindNextFileA(hResult, &uFindFileData);
		} while (bResult);

		bResult = FindClose(hResult);
		fOut << std::endl << std::endl;
		fOut.write(ssOut.str().c_str(), ssOut.str().length());
		fOut.close();
	}
	else
	{
		std::cout << "Can't find files" << std::endl;
	}


	return 0;
}


void parseWorldFile(const std::string &sDir, const std::string &sWorldFile, std::ostream &strHeader, std::ostream &strData)
{
	std::ifstream fIn(sWorldFile.c_str());
	if(!fIn)
		return;

	// Determine the extension of the world file, and find the accompanying image file
	std::string::size_type extPos = sWorldFile.rfind('.');
	std::string ext = StringLib<char>::toLower(sWorldFile.substr(extPos + 1));
	std::string imageFile;
	std::string mimeType;
	if(ext == "tfw")
	{
		imageFile = sWorldFile.substr(0, extPos) + ".tif"; // or tiff?
		mimeType = MIME_TIFF;
	}
	else if(ext == "jgw")
	{
		imageFile = sWorldFile.substr(0, extPos) + ".jpg"; // or jpeg?
		mimeType = MIME_JPEG;
	}
	else if(ext == "pgw")
	{
		imageFile = sWorldFile.substr(0, extPos) + ".png";
		mimeType = MIME_PNG;
	}
	else
	{
		// Unrecognized world file extension
		return;
	}

	// Read file size information
	int w, h, spp, bps;
	try
	{
		RasterImage::GetImageData(imageFile, w, h, spp, bps);
	}
	catch(const ImageBossException &exc)
	{
		std::cout << imageFile << ": " << exc.get_Message() << std::endl;
		return;
	}

	// Create new entry
	strData << "[SheetFile]" << std::endl;
	strData << "FileName=" << imageFile << std::endl;

	static std::map<std::string, std::string> s_mapCoordAlias;

	// Read transformation matrix parameters
	double a, b, c, d, e, f;
	fIn >> a;
	fIn >> b;
	fIn >> c;
	fIn >> d;
	fIn >> e;
	fIn >> f;

	ImageMatrix iMatrix(a, b, c, d, e, f);

	// Determine and write registered coordinates
	CoordXY ul = iMatrix.execute(PixelCoord(-0.5, -0.5));
	CoordXY ur = iMatrix.execute(PixelCoord(w - 0.5, -0.5));
	CoordXY ll = iMatrix.execute(PixelCoord(-0.5, h - 0.5));
	CoordXY lr = iMatrix.execute(PixelCoord(w - 0.5, h - 0.5));

	strData << "Anchor=0|0|" << ul.x << "|" << ul.y << std::endl;
	strData << "Anchor=" << w << "|0|" << ur.x << "|" << ur.y << std::endl;
	strData << "Anchor=0|" << h << "|" << ll.x << "|" << ll.y << std::endl;
	strData << "Anchor=" << w << "|" << h << "|" << lr.x << "|" << lr.y << std::endl;

	// Create WKT, and alias name
	char sWKT[2000];
	// TODO: used registered datum!
	sprintf_s(sWKT, 2000,
		"GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]");

	char sAlias[100];
	sprintf_s(sAlias, 100, "%s%d", g_ALIAS_PREFIX.c_str(), 0);

	if(s_mapCoordAlias.size() == 0)
	{
		s_mapCoordAlias.insert(std::pair<std::string, std::string>(sAlias, sWKT));
		strHeader << "ProjectionAlias=" << sAlias << "|" << sWKT << std::endl;
	}

	strData << "ProjectionAlias=" << sAlias << std::endl;
	strData << "RegisteredCoordAlias=" << sAlias << std::endl;

	strData << std::endl;

	return;
}
