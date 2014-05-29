// SplitTiles
// FS, 17-07-2007

#include "SplitTiles.h"

#define showVal(val) std::cout << #val " = " << (val) << std::endl
#define show(val) #val ": " << (val)


int main(int argc, char *argv[])
{
	// Params needed:
	// Input file
	// Output mask
	// ul_x, ul_y
	// tile_width, tile_height (px)
	// offset_width, offset_height (px)
	
	// Example: SplitTiles -ilatvia_lv10.jpg -olatvia_tiles\%06d_%06d_10.jpg -u568,304 -t256,256
	// Example: SplitTiles -ieurope_lv08.jpg -oeurope_tiles\%06d_%06d_08.jpg -u119,74 -t256,256

	std::string inputFile, /*outputMask,*/ outputDir, outputExt, sULCorner, sTileSize, sOffset = "0,0";
	PixelCoordI ulCorner, tileSize, offset;

	FSUtils::CmdApp app(PROGNAME, PROGVERSION);

	FSUtils::CmdArg
		argInputFile (FSUtils::CmdArgType::String, &inputFile,  "i", "input",       true),
		//argOutputMask(FSUtils::CmdArgType::String, &outputMask, "o", "output-mask", true),
		argOutputDir (FSUtils::CmdArgType::String, &outputDir,  "d", "output-directory", true),
		argOutputExt (FSUtils::CmdArgType::String, &outputExt,  "e", "output-extension", true),
		argULCorner  (FSUtils::CmdArgType::String, &sULCorner,  "u", "upper-left",  true),
		argTileSize  (FSUtils::CmdArgType::String, &sTileSize,  "t", "tile-size",   true),
		argOffset    (FSUtils::CmdArgType::String, &sOffset,         "offset",      false);

	argInputFile.desc   = "input file";
	argInputFile.param  = "INPUT";
	//argOutputMask.desc  = "output parameter";
	//argOutputMask.param = "OUTPUT";
	argOutputDir.desc   = "output directory";
	argOutputDir.param  = "DIR";
	argOutputExt.desc   = "output extension";
	argOutputExt.param  = "EXT";
	argULCorner.desc    = "upper left corner (x,y)";
	argULCorner.param   = "UPPERLEFT";
	argTileSize.desc    = "tile size (w,h)";
	argTileSize.param   = "TILESIZE";
	argOffset.desc      = "offset (x,y; default: 0,0)";
	argOffset.param     = "OFFSET";

	app.bindArg(argInputFile);
	//app.bindArg(argOutputMask);
	app.bindArg(argOutputDir);
	app.bindArg(argOutputExt);
	app.bindArg(argULCorner);
	app.bindArg(argTileSize);
	app.bindArg(argOffset);


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

	if(outputDir[outputDir.size() - 1] != '\\')
	{
		outputDir += '\\';
	}


	int posULCorner = static_cast<int>(sULCorner.find(","));
	int posTileSize = static_cast<int>(sTileSize.find(","));
	int posOffset = static_cast<int>(sOffset.find(","));
	ulCorner = PixelCoordI(atoi(sULCorner.substr(0, posULCorner)), atoi(sULCorner.substr(posULCorner + 1)));
	tileSize = PixelCoordI(atoi(sTileSize.substr(0, posTileSize)), atoi(sTileSize.substr(posTileSize + 1)));
	offset = PixelCoordI(atoi(sOffset.substr(0, posOffset)), atoi(sOffset.substr(posOffset + 1)));

	showVal(ulCorner);
	showVal(tileSize);
	showVal(offset);


	//RasterImage img;
	//img.Load(inputFile);
	//RasterLayer<byte> *lyr = dynamic_cast<RasterLayer<byte> *>(img.Layers()[0]);
	//boost::shared_ptr<RasterLayer<byte>> lyr = img.Layers()[0];
	//boost::shared_ptr<RasterLayer<byte>> lyr = boost::dynamic_pointer_cast<RasterLayer<byte>>(img.Layers()[0]);

	RasterImageReader imgReader;
	IContext *oCtxt = 0;
	try
	{
		oCtxt = imgReader.Open(inputFile);
	}
	catch(...)
	{
		// Ignore
	}

	if(!oCtxt)
	{
		std::cerr << "Can't open input image" << std::endl;
		return -1;
	}

	//int tx = lyr->get_Width() / tileSize.x;
	//int ty = lyr->get_Height() / tileSize.y;
	int tx = imgReader.get_Width() / tileSize.x;
	int ty = imgReader.get_Height() / tileSize.y;
	showVal(tx);
	showVal(ty);
	char fileNameBuf[1024] = {0};
	//showVal(outputMask);
	showVal(outputDir);
	showVal(outputExt);

	// Create all directories
	try
	{
		// Test parent directory
		size_t pos = outputDir.rfind('\\', outputDir.length() - 2);
		showVal(pos);
		if(pos != std::string::npos)
		{
			std::string parentDir = outputDir.substr(0, pos);
			CheckAndCreateDir(parentDir);
		}

		CheckAndCreateDir(outputDir);
	}
	catch(...)
	{
		std::cerr << "Invalid directory" << std::endl;
		return -1;
	}

	for(int dx = 0; dx < tx; ++dx)
	{
		std::string currDir(outputDir + s((int)(ulCorner.x + dx)));
		CheckAndCreateDir(currDir);
	}

	for(int dy = 0; dy < ty; ++dy)
	{
		// Read 256 lines
		RasterImage imgResult;
		boost::shared_ptr<RasterLayer<byte>> lyrIn(new RasterLayer<byte>(imgReader.get_Width(), tileSize.y, 3));
		
		imgResult.Layers().push_back(lyrIn);
		RasterLayer<byte>::Scanline sl(lyrIn->get_ScanlineWidth());

		for(int y = 0; y < tileSize.y; ++y)
		{
			try
			{
				lyrIn->getScanline(sl.begin(), y);
				imgReader.ReadScanline(oCtxt, sl);
				lyrIn->setScanline(sl.begin(), y);
			}
			catch(...)
			{
				// Ignore, just continue
			}
		}

		// Save tiles
		for(int dx = 0; dx < tx; ++dx)
		{
			RasterImage imgResult;
			//RasterLayer<byte> *lyrOut = new RasterLayer<byte>(tileSize.x, tileSize.y, 3);
			boost::shared_ptr<RasterLayer<byte>> lyrOut(new RasterLayer<byte>(tileSize.x, tileSize.y, 3));

			imgResult.Layers().push_back(lyrOut);

			//RasterLayer<byte>::Tile tile(tileSize.x, tileSize.y, 3);
			//lyr->getTile(tile, PixelCoord(dx * tileSize.x, dy * tileSize.y));
			//lyrIn->getTile(*lyrOut, PixelCoordI(dx * tileSize.x, dy * tileSize.y));
			lyrIn->getTile(*lyrOut, PixelCoordI(dx * tileSize.x, 0));

			_snprintf(fileNameBuf, 1024, "%s%d\\%d.%s", outputDir.c_str(), (int)(ulCorner.x + dx), (int)(ulCorner.y + dy), outputExt.c_str());
			showVal(fileNameBuf);
			imgResult.Save(fileNameBuf);
		}
	}

	imgReader.Close(oCtxt);


	return 0;
}

void CheckAndCreateDir(std::string dir)
{
	DWORD dwAttrib = GetFileAttributesA(dir.c_str());

	BOOL dirExists = (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));

	if(!dirExists)
	{
		BOOL result = CreateDirectoryA(dir.c_str(), NULL);
	}

	return;
}
