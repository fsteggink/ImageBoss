// ParseOziMap.cpp
// FS, 08-09-2007

#include "ParseOziMap.h"

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

	argDir.desc         = "ozi map file directory";
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

	std::string sPattern = sDir + "\\*.map";
	std::string sOziMapFile;
	std::ofstream fOut;
	std::ostringstream ssOut;

	hResult = FindFirstFileA(sPattern.c_str(), &uFindFileData);
	if(hResult != INVALID_HANDLE_VALUE)
	{
		fOut.open(sOutputFile.c_str());

		fOut << "ProjectionAlias=" << g_DATUM_WGS84 << "|GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]" << std::endl;
		fOut << "ProjectionAlias=GoogleMaps|PROJCS[\"Mercator_WGS84\",GEOGCS[\"WGS84\",DATUM[\"WGS84\",SPHEROID[\"WGS84\",6378137,298.257223563],TOWGS84[0,0,0,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Mercator\"],PARAMETER[\"central_meridian\",0],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]" << std::endl;
		fOut << "ProjectionAlias=BonneNL|PROJCS[\"Bonne\",GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128],TOWGS84[565.04,49.91,465.84,4.0772e-6,-1.9848e-6,1.7439e-6,-9.0587e-6]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Bonne\"],PARAMETER[\"latitude_of_origin\",51.5],PARAMETER[\"central_meridian\",4.883883008],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",12],PARAMETER[\"false_northing\",-60],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]" << std::endl;
		fOut << "ProjectionAlias=RD|PROJCS[\"Amersfoort / RD New\",GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128],TOWGS84[565.04,49.91,465.84,4.0772e-6,-1.9848e-6,1.7439e-6,-9.0587e-6]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.01745329251994328]],PROJECTION[\"Oblique_Stereographic\"],PARAMETER[\"latitude_of_origin\",52.15616055555555],PARAMETER[\"central_meridian\",5.38763888888889],PARAMETER[\"scale_factor\",0.9999079],PARAMETER[\"false_easting\",155000],PARAMETER[\"false_northing\",463000],UNIT[\"metre\",1]]" << std::endl;
		fOut << std::endl;


		do
		{
			sOziMapFile = sDir + "\\" + uFindFileData.cFileName;
			std::cout << sOziMapFile << std::endl;
			parseMapFile(sDir, sOziMapFile, fOut, ssOut);

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


void parseMapFile(const std::string &sDir, const std::string &sOziMapFile, std::ostream &strHeader, std::ostream &strData)
{
	std::ifstream fIn(sOziMapFile.c_str());
	if(!fIn)
		return;


	strData << "[SheetFile]" << std::endl;

	int iLineCount = 0;
	std::string sLine, sTemp;
	std::vector<std::string> vecWords;
	
	std::string sDatumName, sProjName, sOZIProjName;
	std::map<std::string, double> mapProjParams, mapDatumParams;
	double adToWGS84[7] = {0};
	double lonSum = 0, latSum = 0;
	int coordCount = 0;
	bool bRegIsLL = false;
	std::string sProjLL;

	static std::map<std::string, std::string> s_mapCoordAlias;

	// Insert standard aliases
	s_mapCoordAlias.insert(std::pair<std::string, std::string>("WGS84", "GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]"));
	s_mapCoordAlias.insert(std::pair<std::string, std::string>("GoogleMaps", "PROJCS[\"Mercator_WGS84\",GEOGCS[\"WGS84\",DATUM[\"WGS84\",SPHEROID[\"WGS84\",6378137,298.257223563],TOWGS84[0,0,0,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Mercator\"],PARAMETER[\"central_meridian\",0],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]"));
	s_mapCoordAlias.insert(std::pair<std::string, std::string>("BonneNL", "PROJCS[\"Bonne\",GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128],TOWGS84[565.04,49.91,465.84,4.0772e-6,-1.9848e-6,1.7439e-6,-9.0587e-6]],PRIMEM[\"Greenwich\",0],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"Bonne\"],PARAMETER[\"latitude_of_origin\",51.5],PARAMETER[\"central_meridian\",4.883883008],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",12],PARAMETER[\"false_northing\",-60],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]"));
	s_mapCoordAlias.insert(std::pair<std::string, std::string>("RD", "PROJCS[\"Amersfoort / RD New\",GEOGCS[\"Amersfoort\",DATUM[\"Amersfoort\",SPHEROID[\"Bessel 1841\",6377397.155,299.1528128],TOWGS84[565.04,49.91,465.84,4.0772e-6,-1.9848e-6,1.7439e-6,-9.0587e-6]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.01745329251994328]],PROJECTION[\"Oblique_Stereographic\"],PARAMETER[\"latitude_of_origin\",52.15616055555555],PARAMETER[\"central_meridian\",5.38763888888889],PARAMETER[\"scale_factor\",0.9999079],PARAMETER[\"false_easting\",155000],PARAMETER[\"false_northing\",463000],UNIT[\"metre\",1]]"));

	while(std::getline(fIn, sLine))
	{
		iLineCount++;

		switch(iLineCount)
		{
		case 3:
			// Image file name
			if(sLine.find_first_of("\\/") != std::string::npos)
				strData << "FileName=" << sLine << std::endl;
			else
				strData << "FileName=" << sDir << "\\" << sLine << std::endl;
			break;

		case 5:
			// Registered coordinate projection
			// param 1: datum
			vecWords = SL::split(sLine, ",");
			sDatumName = SL::trim(vecWords[0], WHITESPACE);
			
			if(sDatumName == g_OZIDAT_WGS84)
			{
				mapDatumParams.insert(std::pair<std::string, double>(g_PARAM_SMA, 6378137));
				mapDatumParams.insert(std::pair<std::string, double>(g_PARAM_INV_FL, 298.257223563));
			}
			if(sDatumName == g_OZIDAT_WGS72)
			{
				mapDatumParams.insert(std::pair<std::string, double>(g_PARAM_SMA, 6378135));
				mapDatumParams.insert(std::pair<std::string, double>(g_PARAM_INV_FL, 298.26));
			}
			else if(sDatumName == g_OZIDAT_PULKOVO1 || sDatumName == g_OZIDAT_PULKOVO2)
			{
				mapDatumParams.insert(std::pair<std::string, double>(g_PARAM_SMA, 6378245));
				mapDatumParams.insert(std::pair<std::string, double>(g_PARAM_INV_FL, 298.3));
				adToWGS84[0] = 28;
				adToWGS84[1] = -130;
				adToWGS84[2] = -95;
			}
			else if(sDatumName == g_OZIDAT_POTSDAM_DHDN)
			{
				// WGS84 parameters inserted
				mapDatumParams.insert(std::pair<std::string, double>(g_PARAM_SMA, 6378137));
				mapDatumParams.insert(std::pair<std::string, double>(g_PARAM_INV_FL, 298.257223563));
			}
			else if(sDatumName == g_OZIDAT_RD)
			{
				// Override OziExplorer TM-definition later on
			}
			
			
			sTemp = SL::trim(vecWords[1], WHITESPACE);
			if(sTemp == g_OZIDAT_WGS84	)
				sTemp = g_DATUM_WGS84;

			//strData << "ProjectionAlias=" << sTemp << std::endl;
			//strData << "RegisteredCoordAlias=" << sTemp << std::endl;
			sProjLL = sTemp;
			break;

		case 9:
			// Map projection
			vecWords = SL::split(sLine, ",");
			sOZIProjName = SL::trim(vecWords[1], WHITESPACE);

			if(sOZIProjName == g_OZIPROJ_UTM ||
				sOZIProjName == g_OZIPROJ_TM)
			{
				sProjName = g_PROJ_TM;
			}
			else if(sOZIProjName == g_OZIPROJ_LCC)
			{
				sProjName = g_PROJ_LCC_1SP;
			}
			else if(sOZIProjName == g_OZIPROJ_BONNE)
			{
				sProjName = g_PROJ_BONNE;
			}


			//sProjName = "Mercator_1SP";
			//sProjName = "Bonne";
			//sProjName = "Oblique_Stereographic";

			break;

		case 40:
			// Map projection parameters
			//param 2: lat0
			//param 3: lon0
			//param 4: k
			//param 5: FE
			//param 6: FN
			//param 7: lat1
			//param 8: lat2
			//param 9: height
			if(sOZIProjName == g_OZIPROJ_UTM)
			{
				double lonAvg = lonSum / coordCount;
				int iZone = static_cast<int>((lonAvg + 186.0) / 6.0);
				double lon0 = iZone * 6 - 183;

				int FN = (latSum < 0) ? 10000000 : 0;

				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_LAT0, 0));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_LON0, lon0));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_K, 0.9996));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_FE, 500000));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_FN, FN));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_LAT1, 0));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_LAT2, 0));
				//mapProjParams.insert(std::pair<std::string, double>("height", 0));
			}
			else if(sOZIProjName == g_OZIPROJ_TM ||
				sOZIProjName == g_OZIPROJ_LCC ||
				sOZIProjName == g_OZIPROJ_BONNE)
			{
				vecWords = SL::split(sLine, ",");
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_LAT0, atof(vecWords[1])));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_LON0, atof(vecWords[2])));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_K, atof(vecWords[3])));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_FE, atof(vecWords[4])));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_FN, atof(vecWords[5])));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_LAT1, atof(vecWords[6])));
				mapProjParams.insert(std::pair<std::string, double>(g_PARAM_LAT2, atof(vecWords[7])));
				//mapProjParams.insert(std::pair<std::string, double>("height", atof(vecWords[8])));

				// Below changes (k, LCC_1SP) are not necessary when LCC_2SP has been implemented
				// LCC_2SP doesn't need a scale factor
				if(mapProjParams[g_PARAM_K] == 0)
					mapProjParams[g_PARAM_K] = 1;

				if(sProjName == g_PROJ_LCC_1SP)
					mapProjParams[g_PARAM_LAT0] = (mapProjParams[g_PARAM_LAT1] + mapProjParams[g_PARAM_LAT2]) / 2.0;
			}
			else // Lat-Lon projection
			{
				//std::cout << "Test" << std::endl;
			}

			break;
		}

		if(iLineCount >= 10 && iLineCount <= 39)
		{
			vecWords = SL::split(sLine, ",");

			if(SL::trim(vecWords[2], WHITESPACE).length() > 0 && SL::trim(vecWords[4], WHITESPACE) == "in")
			{
				if(SL::trim(vecWords[6], WHITESPACE).size() > 0)
				{
					double dLat, dLon;
					dLat = atof(SL::trim(vecWords[6], WHITESPACE)) + atof(SL::trim(vecWords[7], WHITESPACE)) / 60;
					dLon = atof(SL::trim(vecWords[9], WHITESPACE)) + atof(SL::trim(vecWords[10], WHITESPACE)) / 60;
					if(SL::trim(vecWords[8], WHITESPACE)=="S")
						dLat = -dLat;
					if(SL::trim(vecWords[11], WHITESPACE)=="W")
						dLon = -dLon;
					
					strData << "Anchor=" << SL::trim(vecWords[2], WHITESPACE) <<
						"|" << SL::trim(vecWords[3], WHITESPACE) <<
						"|" << dLon << "|" << dLat << std::endl;

					// Store average longitude for UTM zone (not stored by OZI), and average latitude
					// for northern / southern hemisphere
					lonSum += dLon;
					latSum += dLat;
					bRegIsLL = true;
				}
				else
				{
					strData << "Anchor=" << SL::trim(vecWords[2], WHITESPACE) <<
						"|" << SL::trim(vecWords[3], WHITESPACE) <<
						"|" << SL::trim(vecWords[14], WHITESPACE) <<
						"|" << SL::trim(vecWords[15], WHITESPACE) << std::endl;
				}
				coordCount++;
			}
		}
	}


	// Create WKT, and alias name
	//"PROJCS[\"<name>\",GEOGCS[\"<name>\",DATUM[\"<name>\",SPHEROID[\"<name>\",<a>,<1/f>],TOWGS84[<tx,ty,tz,d,rx,ry,rz>]],PRIMEM[\"Greenwich\",0,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"<proj>\"],PARAMETER[\"latitude_of_origin\",<lat0>],PARAMETER[\"central_meridian\",<lon0>],PARAMETER[\"scale_factor\",<k>],PARAMETER[\"false_easting\",<FE>],PARAMETER[\"false_northing\",<FN>],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]";
	//"GEOGCS[\"<name>\",DATUM[\"<name>\",SPHEROID[\"<name>\",<a>,<1/f>],TOWGS84[<tx,ty,tz,d,rx,ry,rz>]],PRIMEM[\"Greenwich\",0,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]]";
	char sWKT[2000];
	if(sDatumName == g_OZIDAT_RD)
	{
		// Override OziExplorer's TM definition
		strncpy_s(sWKT, s_mapCoordAlias["RD"].c_str(), 2000);
	}
	else if(sProjName == g_PROJ_TM ||
		sProjName == g_PROJ_MERC_1SP ||
		sProjName == g_PROJ_BONNE ||
		sProjName == g_PROJ_OBLSTER ||
		sProjName == g_PROJ_LCC_1SP)
	{
		sprintf_s(sWKT, 2000,
			"PROJCS[\"%s\",GEOGCS[\"%s\",DATUM[\"%s\",SPHEROID[\"%s\",%.10f,%.10f],TOWGS84[%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f]],PRIMEM[\"Greenwich\",0,],UNIT[\"DMSH\",0.0174532925199433],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST]],PROJECTION[\"%s\"],PARAMETER[\"latitude_of_origin\",%.10f],PARAMETER[\"central_meridian\",%.10f],PARAMETER[\"scale_factor\",%.10f],PARAMETER[\"false_easting\",%.10f],PARAMETER[\"false_northing\",%.10f],UNIT[\"metre\",1],AXIS[\"E\",EAST],AXIS[\"N\",NORTH]]",
			"X",
			"X",
			sDatumName.c_str(),
			"X",
			mapDatumParams[g_PARAM_SMA],
			mapDatumParams[g_PARAM_INV_FL],
			adToWGS84[0],
			adToWGS84[1],
			adToWGS84[2],
			adToWGS84[3],
			adToWGS84[4],
			adToWGS84[5],
			adToWGS84[6],
			sProjName.c_str(),
			mapProjParams[g_PARAM_LAT0],
			mapProjParams[g_PARAM_LON0],
			mapProjParams[g_PARAM_K],
			mapProjParams[g_PARAM_FE],
			mapProjParams[g_PARAM_FN]);
	}
	else
	{
		// TODO: used registered datum!
		sprintf_s(sWKT, 2000,
			"GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]]");
	}

	// Check if the projection can already be found
	char sAlias[100] = {0};
	for(std::map<std::string, std::string>::iterator iterAlias = s_mapCoordAlias.begin(); iterAlias != s_mapCoordAlias.end(); ++iterAlias)
	{
		if(iterAlias->second == sWKT)
		{
			strncpy_s(sAlias, iterAlias->first.c_str(), 100);
		}
	}

	if(sAlias[0] == 0)
	{
		// Insert new projection alias
		sprintf_s(sAlias, 100, "%s%d", g_ALIAS_PREFIX.c_str(), s_mapCoordAlias.size());
		s_mapCoordAlias.insert(std::pair<std::string, std::string>(sAlias, sWKT));

		strHeader << "ProjectionAlias=" << sAlias << "|" << sWKT << std::endl;
	}

	strData << "ProjectionAlias=" << sAlias << std::endl;

	if(bRegIsLL)
	{
		strData << "RegisteredCoordAlias=" << sTemp << std::endl;
	}
	else
	{
		strData << "RegisteredCoordAlias=" << sAlias << std::endl;
	}


	strData << std::endl;

	return;
}
