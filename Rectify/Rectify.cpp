// Rectify.cpp
// FS, 20-04-2007

#include "Rectify.h"

// Global variables
int g_polygoncontains = 0;
std::map<std::string, boost::shared_ptr<CoordinateSystem>> g_mapCS;
std::map<boost::shared_ptr<const Sheet>, boost::shared_ptr<CoordinateCache<CoordXY, PixelCoord>>> g_coordCaches_img;
std::map<boost::shared_ptr<const Sheet>, boost::shared_ptr<CoordinateCache<CoordXY, CoordXY>>> g_coordCaches_map;

ContextData g_uContext;
RasterLayer<byte>::Scanline g_sl_bg;

boost::mutex g_mutex;


int main(int argc, char *argv[])
{
	std::string sheetIndexFile, outputFile, sBBox, sResolution, sCoordSys, sBGColor;
	Box<CoordXY> box;
	double resx, resy;

	int bgcolor = 0xFFFFFF;
	std::string sFilter = "TENT";

	const std::string csVALID_FILTERS = "BOX|TENT|LANCZOS";

	//boost::shared_ptr<ProjectedCS> pcs(new ProjectedCS());
	//pcs->parseWKT("PROJCS[\"Lambert Conformal Conic Canada\",GEOGCS[\"WGS 84\",DATUM[\"WGS 84\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],TOWGS84[0,0,0,0,0,0,0],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"DMSH\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9108\"]],AXIS[\"Lat\",NORTH],AXIS[\"Long\",EAST],AUTHORITY[\"EPSG\",\"4326\"]],PROJECTION[\"Lambert_Conformal_Conic\"],PARAMETER[\"latitude_of_origin\",62],PARAMETER[\"central_meridian\",-90],PARAMETER[\"scale_factor\",0.999],PARAMETER[\"false_easting\",0],PARAMETER[\"false_northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"E\",EAST],AXIS[\"N\",NORTH],AUTHORITY[\"FS\",\"0\"]]");

	// Canada, call of rectify: rectify -b-3000000,-2500000,3200000,2600000 -r10000 -itest.dat -otest.jpg

	// Number of processors:
	/*
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );

	int numCPU = sysinfo.dwNumberOfProcessors;
	*/


	boost::shared_ptr<CoordinateSystem> cs(g_csWGS84);

	FSUtils::CmdApp app(PROGNAME, PROGVERSION);

	FSUtils::CmdArg
		argBBox      (FSUtils::CmdArgType::String, &sBBox,          "b", "box",         true),
		argResolution(FSUtils::CmdArgType::String, &sResolution,    "r", "resolution",  true),
		argSheetIndex(FSUtils::CmdArgType::String, &sheetIndexFile, "i", "sheet-index", true),
		argOutputFile(FSUtils::CmdArgType::String, &outputFile,     "o", "output",      true),
		argCoordSys  (FSUtils::CmdArgType::String, &sCoordSys,      "c", "coordsys",    false),
		argBGColor   (FSUtils::CmdArgType::String, &sBGColor,       "x", "bgcolor",     false),
		argFilter    (FSUtils::CmdArgType::String, &sFilter,         "", "filter",      false);
	
	// Projection: can be alias (should be defined in sheet index) or WKT

	argBBox.desc        = "bounding box (x0,y0,x1,y1)";
	argBBox.param       = "BOX";
	argResolution.desc  = "resolution (resx[,resy])";
	argResolution.param = "RES";
	argSheetIndex.desc  = "sheet index file";
	argSheetIndex.param = "INDEX";
	argOutputFile.desc  = "output file";
	argOutputFile.param = "OUTPUT";
	argCoordSys.desc    = "coordinate system (alias or wkt) =WGS84";
	argCoordSys.param   = "COORDSYS";
	argBGColor.desc     = "background color (hex RRGGBB) =0xFFFFFF";
	argBGColor.param    = "BGCOLOR";
	argFilter.desc      = "filter (" + csVALID_FILTERS + ") =" + sFilter;
	argFilter.param     = "FILTER";

	app.bindArg(argBBox);
	app.bindArg(argResolution);
	app.bindArg(argSheetIndex);
	app.bindArg(argOutputFile);
	app.bindArg(argCoordSys);
	app.bindArg(argBGColor);
	app.bindArg(argFilter);


	try
	{
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


	// Parse bounding box and resolution
	std::vector<std::string> words = StringLib<char>::split(sBBox, ",");
	if(words.size() < 4)
	{
		app.syntax("Not enough values in bounding box");
		return -1;
	}

	for(int i = 0; i < 4; ++i)
	{
		words[i] = StringLib<char>::trim(words[i], WHITESPACE);
	}
	box = Box<CoordXY>(
		CoordXY(atof(words[0]), atof(words[1])),
		CoordXY(atof(words[2]), atof(words[3])));

	words = StringLib<char>::split(sResolution, ",");
	if(words.size() == 0)
	{
		app.syntax("Not enough values in resolution");
		return -1;
	}

	words[0] = StringLib<char>::trim(words[0], WHITESPACE);
	resx = resy = atof(words[0]);

	if(words.size() > 1)
	{
		words[1] = StringLib<char>::trim(words[1], WHITESPACE);
		resy = atof(words[1]);
	}

	if(box.upper.x < box.lower.x)
	{
		std::cerr << "Maxx should be greater than minx" << std::endl;
		exit(1);
	}
	else if(box.upper.y < box.lower.y)
	{
		std::cerr << "Maxy should be greater than miny" << std::endl;
		exit(1);
	}

	// Parse color
	if(sBGColor.length() > 0)
	{
		if(sBGColor[0] == '#')
			sBGColor = sBGColor.substr(1);
		if(sBGColor.compare(0, 2, "0x") != 0 && sBGColor.compare(0, 2, "0X") != 0)
			sBGColor = "0x" + sBGColor;
		bgcolor = strtol(sBGColor.c_str(), 0, 16);
	}

	// Parse filter
	sFilter = StringLib<char>::toUpper(sFilter);
	if(("|" + csVALID_FILTERS + "|").find("|" + sFilter + "|") == std::string::npos)
	{
		std::cerr << "Filter " << sFilter << " is not recognized" << std::endl;
		exit(1);
	}


	showVal(sheetIndexFile);
	showVal(outputFile);
	showVal(box);
	showVal(resx);
	showVal(resy);
	showVal(sCoordSys);
	showVal(bgcolor);
	showVal(sFilter);


	// Read sheet index file
	SheetIndex &sheetIndex = g_uContext.sheetIndex;
	std::ifstream fIn(sheetIndexFile.c_str());
	if(!fIn)
	{
		std::cerr << "Sheet index " << sheetIndexFile << " could not be opened" << std::endl;
		exit(1);
	}
	sheetIndex.readGeneral(fIn);

	// Alias can't be read, because it is in sheetfile, but CS's are defined in sheetfile as well!
	showVal(g_mapCS.size());
	if(sCoordSys != "")
	{
		if(g_mapCS.find(sCoordSys) != g_mapCS.end())
			cs = g_mapCS[sCoordSys];
		else
			cs = CoordinateSystem::CreateCS(sCoordSys);
	}
	showVal(cs->getName());


	try
	{
		std::cout << "Parse index file" << std::endl;
		//std::cout << "File pos: " << fIn.tellg() << std::endl;
		sheetIndex.read(fIn, cs, box);
	}
	catch(char *exc)
	{
		std::cout << "An exception has occurred: " << exc << std::endl;
		exit(1);
	}
	catch(...)
	{
		std::cout << "An unknown exception has occurred while reading index file" << std::endl;
		exit(1);
	}

	std::cout << "Number of sheets: " << sheetIndex.size() << std::endl;
	if(sheetIndex.size() == 0)
	{
		std::cerr << "Can't read any sheets" << std::endl;
		exit(1);
	}
	std::cout << "All sheets read" << std::endl;
	fIn.close();
	
	

	LanczosFilter<2> uFilter_Lanczos2;
	//LanczosFilter<3> uFilter;
	//GaussianFilter<2> uFilter;   // broken
	TentFilter uFilter_Tent;
	BoxFilter uFilter_Box;

	g_uContext.box = box;
	g_uContext.resx = resx;
	g_uContext.resy = resy;

	if(sFilter.compare("BOX") == 0)
		g_uContext.oFilter = &uFilter_Box;
	else if(sFilter.compare("TENT") == 0)
		g_uContext.oFilter = &uFilter_Tent;
	else if(sFilter.compare("LANCZOS") == 0)
		g_uContext.oFilter = &uFilter_Lanczos2;

	g_uContext.width  = static_cast<int>(0.5 + g_uContext.box.width()  / fabs(g_uContext.resx));
	g_uContext.height = static_cast<int>(0.5 + g_uContext.box.height() / fabs(g_uContext.resy));


	// Prepare background for scanline
	RasterLayer<byte>::Pixel px(3);
	px[0] = (bgcolor & 0xFF0000) >> 16;
	px[1] = (bgcolor & 0xFF00) >> 8;
	px[2] = bgcolor & 0xFF;
		
	g_sl_bg = RasterLayer<byte>::Scanline(3 * g_uContext.width);
	showVal(g_sl_bg.size());
	for(int x = 0; x < g_uContext.width; ++x)
	{
		memcpy(&g_sl_bg[x * 3], &px[0], 3);
	}


	try
	{
		CreateImage(outputFile, cs);
	}
	catch(const char *exc)
	{
		showVal(exc);
	}
	catch(const std::string &exc)
	{
		showVal(exc);
	}

	return 0;
}


void CreateImage(const std::string &outputFile, const boost::shared_ptr<CoordinateSystem> &cs)
{
	std::cout << "Bitmap size: " << g_uContext.width << "," << g_uContext.height << std::endl;

	// Remove superfluous sheets from the index
	SheetIndex &sheetIndex = const_cast<SheetIndex &>(g_uContext.sheetIndex);
	sheetIndex.matchBox(g_uContext.box, cs);
	std::cout << "Sheet indexes left: " << sheetIndex.size() << std::endl;


	std::string ext = SL::toLower(outputFile.substr(outputFile.length() - 4));
	if(ext.compare(".ecw") == 0 || ext.compare(".jp2") == 0)
	{
		// Use ECW to create image
		//CreateECW(outputFile.c_str(), g_uContext.width, g_uContext.height, LoadAndReadLine);
		std::cout << "ECW output not supported!" << std::endl;
	}
	else
	{
		// Save image by scanline
		CreateImage_scanline(outputFile, cs);
	}

	return;
}


void CreateImage_scanline(const std::string &outputFile, const boost::shared_ptr<CoordinateSystem> &cs)
{
	RasterImageWriter imgResult;
	IContext *oCtxt = imgResult.Open(outputFile, g_uContext.width, g_uContext.height, 3);

	if(oCtxt)
	{
		RasterLayer<byte>::Scanline sl(imgResult.get_ScanlineWidth());
		RasterLayer<byte>::Scanline sl_bg(imgResult.get_ScanlineWidth());
		

		for(int y = 0; y < g_uContext.height; ++y)
		{
			try
			{
				LoadAndReadLine(sl, y, cs);
				imgResult.WriteScanline(oCtxt, sl);
			}
			catch(...)
			{
				// Ignore, just continue
			}
		}
		imgResult.Close(oCtxt);
	}
	else
	{
		// If file can't be created above, then it will probably also fail here.
		// This is unless no plugin can be found to save files by scanline.
		RasterImage imgResult = CreateBitmap(cs);
		std::cout << "Save image result" << std::endl;
		showVal(outputFile);
		imgResult.Save(outputFile);
	}

	return;
}


RasterImage CreateBitmap(const boost::shared_ptr<CoordinateSystem> &cs)
{
	RasterLayer<byte>::Pixel px(3);
	for(int k = 0; k < 3; ++k)
	{
		px[k] = g_sl_bg[k];
	}
	
	// Create bitmap
	RasterImage imgResult;
	boost::shared_ptr<RasterLayer<byte>> lyrOut(new RasterLayer<byte>(g_uContext.width, g_uContext.height, 3, px));
	
	imgResult.Layers().push_back(lyrOut);
	RasterLayer<byte>::Scanline sl(lyrOut->get_ScanlineWidth());

	for(int y = 0; y < g_uContext.height; ++y)
	{
		try
		{
			//lyrOut->getScanline(sl.begin(), y);
			LoadAndReadLine(sl, y, cs);
			lyrOut->setScanline(sl.begin(), y);
		}
		catch(...)
		{
			// Ignore, just continue
		}
	}

	return imgResult;
}

void LoadAndReadLine(RasterLayer<byte>::Scanline &sl, int y, const boost::shared_ptr<CoordinateSystem> &cs)
{
	//char state = 'A';
	try
	{
		// Clear scanline
		memcpy(&sl[0], &g_sl_bg[0], g_sl_bg.size());
		//state='B';

		static SheetIndex activeSheets;
		if(y == 0)
			activeSheets.getSheets().reserve(g_uContext.sheetIndex.size());   // Reserve space
		//state='C';

		static SheetIndex &sheetIndex = g_uContext.sheetIndex;
		//state='D';

		static double e = (g_uContext.resx >= 0) ? g_uContext.box.lower.x : g_uContext.box.upper.x;
		static double f = (g_uContext.resy >= 0) ? g_uContext.box.upper.y : g_uContext.box.lower.y;
		static ImageMatrix im(g_uContext.resx, 0, 0, -g_uContext.resy, e, f);
		//state='E';

		// Check which sheets should be loaded or unloaded
		PixelCoord imgC(0.5, y + 0.5);
		double ry = g_uContext.box.upper.y - (imgC.y + EPSILON) * g_uContext.resy;
		if(g_uContext.resy < 0)
			ry = g_uContext.box.upper.y + (imgC.y + EPSILON) * g_uContext.resy;
		//state='F';

		activeSheets.loadSheets(sheetIndex, g_uContext.box, y, ry, cs);
		//state='G';
		activeSheets.unloadSheets(g_uContext.box, y, ry, cs);
		//state='H';

		ReadLine(sl, 0, g_uContext.width, y, activeSheets, im, cs);
		//state='I';

		/*
		// Multithreaded approach
		// Notes:
		// * RasterLayer::getPixel_filtered contains some static variables. They must be made normal (stack) variables, but then the program becomes slower.
		// * Pass pointers to ReadLine (needs to be adjusted as well), otherwise no image is created. Boost::bind takes care that all variables are passed by value.
		// * When using threads, the resulting image is not 100% identical. When calling ReadLine directly, the result is identical to the unsplit version.
		// Because of the first point, there is not really a performance benefit.
		int cw = g_uContext.width;
		int numCores = 6;
		boost::thread_group threads;

		for(int n = 0; n < numCores; n++)
		{
			//ReadLine(&sl, n * cw / numCores, (n + 1) * cw / numCores, y, &activeSheets, &im, &cs);
			threads.create_thread(boost::bind(ReadLine, &sl, n * cw / numCores, (n + 1) * cw / numCores, y, &activeSheets, &im, &cs));
		}

		threads.join_all();
		*/
	}
	catch(char *e)
	{
		std::cerr << "Exception in LoadAndReadLine, msg: " << e << ", y: " << y << std::endl;
		throw;
	}
	catch(...)
	{
		std::cerr << "Exception in LoadAndReadLine, y: " << y << std::endl; // << ", state: " << state << std::endl;
		throw;
	}
	//std::cout << "Line " << y << " has been read" << std::endl;

	return;
}

void ReadLine(RasterLayer<byte>::Scanline &sl, int start, int end, int y, const SheetIndex &activeSheets, const ImageMatrix &im, const boost::shared_ptr<CoordinateSystem> &cs)
{
	// Always reset active sheet
	ConstSheetIterator activeSheet = activeSheets.end();
	std::map<boost::shared_ptr<const Sheet>, boost::shared_ptr<CoordinateCache<CoordXY, PixelCoord>>>::const_iterator iterCC_img;
	std::map<boost::shared_ptr<const Sheet>, boost::shared_ptr<CoordinateCache<CoordXY, CoordXY>>>::const_iterator iterCC_map;
	float decX, decY;

	for(int x = start; x < end; ++x)
	{
		PixelCoord pc;
		CoordXY r;
		//char state = 'a';

		try
		{
			PixelCoord imgC(x + 0.5, y + 0.5);
			//state = 'b';
			r = im.execute(imgC);
			//state = 'c';

			//if(x == 0)
			//	std::cout << "y: " << y << ", y-coord: " << r.y << std::endl;

			// Test if coordinate still falls within the active sheet
			if(activeSheet != activeSheets.end())
			{
				// Does it fall in the sheet?
				iterCC_map = g_coordCaches_map.find(*activeSheet);
				CoordXY r_map = iterCC_map->second->execute(r);

				if(!(*activeSheet)->contains(r_map))
					activeSheet = activeSheets.end();
			}
			//state = 'd';

			if(activeSheet == activeSheets.end())
			{
				activeSheet = getActiveSheet(activeSheets, r, cs);
				//state = 'g';
				
				if(activeSheet == activeSheets.end())
				{
					continue;
				}
				//std::cout << "New active sheet found: " << activeSheet->get()->getName() << std::endl;
				//state = 'h';

				// Bepaal de coordinate caches
				iterCC_img = g_coordCaches_img.find(*activeSheet);
				iterCC_map = g_coordCaches_map.find(*activeSheet);
				//state = 'i';

				// Bepaal de decimation
				CoordXY r1 = im.execute(imgC + PixelCoord(1, 1));
				//state = 'j';
				PixelCoord p = iterCC_img->second->execute(r);
				//state = 'k';
				PixelCoord p1 = iterCC_img->second->execute(r1);
				//state = 'l';

				decX = static_cast<float>(fabs(p1.x - p.x));
				//state = 'm';
				decY = static_cast<float>(fabs(p1.y - p.y));
				//state = 'n';
			}
			//state = 'e';

			// Calculate coordinates to get pixel
			pc = iterCC_img->second->execute(r);
			//state = 'f';
		}
		catch(...)
		{
			//std::cout << "Exception in ReadLine, x: " << x << ", y: " << y << std::endl;
			//std::cout << "State: " << state << std::endl;
			throw;
		}

		//std::cout << (*activeSheet)->getName() << ": " << pc << std::endl;

		// Get pixel and set pixel
		try
		{
			//boost::mutex::scoped_lock lock(g_mutex);
			(*activeSheet)->getPixel(sl.begin() + x * 3, pc, decX, decY, *g_uContext.oFilter);
		}
		catch(ImageBossException &exc)
		{
			std::cerr << "Exception: " << exc.get_Message() << ", r: " << r << std::endl;
		}
		catch(...)
		{
			std::cerr << "An unknown error occurred while reading line " << y << std::endl;
			throw;
		}
	} // for x

	return;
}

ConstSheetIterator getActiveSheet(
	const SheetIndex &activeSheets,
	const CoordXY &r,
	const boost::shared_ptr<CoordinateSystem> &cs)
{
	// Find new active sheet
	ConstSheetIterator activeSheet = activeSheets.findActiveSheet(r, cs);

	if(activeSheet == activeSheets.end())
	{
		return activeSheet;
	}

	// Laad het kaartblad
	RasterLayer<byte>::Pixel px(3);
	memcpy(&px[0], &g_sl_bg[0], 3);
	(*activeSheet)->load(px);

	if(!(*activeSheet)->isOpen())   // Check if file is indeed open
	{
		return activeSheets.end();
	}

	return activeSheet;
}

// cs1: to
// cs2: from
boost::shared_ptr<XYTrans<CoordXY, CoordXY>> CreateTransformation(
	const boost::shared_ptr<CoordinateSystem> &cs1, const boost::shared_ptr<CoordinateSystem> &cs2)
{
	// Create transformation chains for transformations between coordinate systems
	boost::shared_ptr<XYTrans<CoordXY, CoordXY>> trans;

	boost::shared_ptr<GeographicCS> geo1 = boost::dynamic_pointer_cast<GeographicCS>(cs1);
	boost::shared_ptr<GeographicCS> geo2 = boost::dynamic_pointer_cast<GeographicCS>(cs2);
	boost::shared_ptr<ProjectedCS> proj1 = boost::dynamic_pointer_cast<ProjectedCS>(cs1);
	boost::shared_ptr<ProjectedCS> proj2 = boost::dynamic_pointer_cast<ProjectedCS>(cs2);

	if(geo1 != 0 && geo2 != 0)
	{
		trans = boost::shared_ptr<XYTrans<CoordXY, CoordXY>>(new TransChain<CoordXY, CoordXY, CoordLL>(
			boost::shared_ptr<XYTrans<CoordXY, CoordLL>>(new XY2LL()),
			boost::shared_ptr<XYTrans<CoordLL, CoordXY>>(new TransChain<CoordLL, CoordXY, CoordLL>(
				geo2->getTransformation(geo1),
				boost::shared_ptr<XYTrans<CoordLL, CoordXY>>(new LL2XY())))
			));
	}
	else if(geo1 != 0 && proj2 != 0)
	{
		trans = boost::shared_ptr<XYTrans<CoordXY, CoordXY>>(new TransChain<CoordXY, CoordXY, CoordLL>(
			proj2->getTransformation(geo1),
			boost::shared_ptr<XYTrans<CoordLL, CoordXY>>(new LL2XY())));
	}
	else if(proj1 != 0 && geo2 != 0)
	{
		trans = boost::shared_ptr<XYTrans<CoordXY, CoordXY>>(new TransChain<CoordXY, CoordXY, CoordLL>(
			boost::shared_ptr<XYTrans<CoordXY, CoordLL>>(new XY2LL()),
			geo2->getTransformation(proj1)));
	}
	else if(proj1 != 0 && proj2 != 0)
	{
		trans = proj2->getTransformation(proj1);
	}
	else
	{
		throw "Unrecognized projection(s)";
	}


	return trans;
}

CoordXY normalizeLon(const CoordXY &coord, double normLon)
{
	CoordXY result(coord);

	while(result.x > normLon + 180)
	{
		result.x -= 360;
	}

	while(result.x < normLon - 180)
	{
		result.x += 360;
	}

	return result;
}
