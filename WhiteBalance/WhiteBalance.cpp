// WhiteBalance.cpp
// FS, 20-07-2013

#include "WhiteBalance.h"


int main(int argc, char *argv[])
{
	std::string inputFile, outputFile;
	unsigned int filterSize = FILTER_SIZE;
	double threshold = WB_THRESHOLD;

	FSUtils::CmdApp app(PROGNAME, PROGVERSION);

	FSUtils::CmdArg
		argInputFile (FSUtils::CmdArgType::String, &inputFile,  "i", "input",       true),
		argOutputFile(FSUtils::CmdArgType::String, &outputFile, "o", "output",      true),
		argFilterSize(FSUtils::CmdArgType::Int,    &filterSize, "f", "filter_size", false),
		argThreshold (FSUtils::CmdArgType::Double, &threshold,  "t", "threshold",   false);
	// Projection: can be alias (should be defined in sheet index) or WKT

	argInputFile.desc   = "input file";
	argInputFile.param  = "INPUT";
	argOutputFile.desc  = "output file";
	argOutputFile.param = "OUTPUT";
	argFilterSize.desc  = "filter size (default: 200)";
	argFilterSize.param = "FILTER";
	argThreshold.desc   = "threshold (default, 0.5%)";
	argThreshold.param  = "THRES";

	app.bindArg(argInputFile);
	app.bindArg(argOutputFile);
	app.bindArg(argFilterSize);
	app.bindArg(argThreshold);


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

	// Check argument values
	if(threshold < 0 || threshold >= 0.5)
	{
		app.syntax("Invalid threshold value, must between 0 and 0.5");
		return -1;
	}


	if(app.stopExecution())
	{
		// Stop execution. Potential reasons: help / version information requested.
		return 0;
	}

	std::cout << "Input: " << inputFile << std::endl;
	std::cout << "Output: " << outputFile << std::endl;
	std::cout << "Filter size: " << filterSize << std::endl;

	try
	{
		applyWhiteBalance(inputFile, outputFile, filterSize, threshold);
	}
	catch(const char *exc)
	{
		std::cerr << "Exception: " << exc << std::endl;
	}
	catch(...)
	{
		std::cerr << "Unknown exception" << std::endl;
	}

	return 0;
}


void applyWhiteBalance(const std::string &inputFile, const std::string &outputFile, unsigned int filterSize, double threshold)
{
	boost::shared_ptr<RasterImage> img(new RasterImage());
	img->Load(inputFile);
	boost::shared_ptr<RasterLayer<byte>> lyr = boost::dynamic_pointer_cast<RasterLayer<byte>, IRasterLayer>(img->Layers()[0]);

	RasterLayer<byte>::Tile tile(std::min(filterSize, lyr->get_Width()), std::min(filterSize, lyr->get_Height()), 3);

	unsigned int thresCount = static_cast<unsigned int>(lyr->get_Width() * lyr->get_Height() * threshold);

	Histogram hist = getHistogram(*lyr);
	RGB global_black, global_white;
	getThresholdValues(global_black, global_white, hist, thresCount);

	for(unsigned int y = 0; y < lyr->get_Height(); y += filterSize)
	{
		for(unsigned int x = 0; x < lyr->get_Width(); x += filterSize)
		{
	//for(unsigned int y = 0; y < 200; y += filterSize)
	//{
	//	for(unsigned int x = 0; x < 200; x += filterSize)
	//	{
			unsigned int thresCount = static_cast<unsigned int>(tile.get_Width() * tile.get_Height() * threshold);

			//std::cout << "Process block " << x << ", " << y << std::endl;
			lyr->getTile(tile, PixelCoordI(x, y));
			hist = getHistogram(tile);
			//showHistogram(hist);
			//std::cout << tile.get_Width() * tile.get_Height() << std::endl;
			//std::cout << "Threshold: " << thresCount << std::endl;

			RGB black, white;
			getThresholdValues(black, white, hist, thresCount);
			//std::cout << "Black: " << black.r << " " << black.g << " " << black.b << std::endl;
			//std::cout << "White: " << white.r << " " << white.g << " " << white.b << std::endl;

			// Correctoin for global values
			black.r = std::max(black.r, global_black.r);
			black.g = std::max(black.g, global_black.g);
			black.b = std::max(black.b, global_black.b);
			white.r = std::min(white.r, global_white.r);
			white.g = std::min(white.g, global_white.g);
			white.b = std::min(white.b, global_white.b);

			double rf = 255.0 / (white.r - black.r);
			double gf = 255.0 / (white.g - black.g);
			double bf = 255.0 / (white.b - black.b);

			for(unsigned int j = 0; j < tile.get_Height(); ++j)
			{
				for(unsigned int i = 0; i < tile.get_Width(); ++i)
				{
					RasterLayer<byte>::Pixel px(3);
					tile.getPixel(px.begin(), i, j);
					px[0] = Bounds((int)(px[0] * rf - black.r), 0, 255);
					px[1] = Bounds((int)(px[1] * rf - black.g), 0, 255);
					px[2] = Bounds((int)(px[2] * rf - black.b), 0, 255);
					tile.setPixel(px.begin(), i, j);
				}
			}

			lyr->setTile(tile, PixelCoordI(x, y));
		}
	}

	img->Save(outputFile);

	return;
}


Histogram getHistogram(const RasterLayer<byte>::Tile &tile)
{
	Histogram hist(768);

	for(unsigned int y = 0; y < tile.get_Height(); ++y)
	{
		for(unsigned int x = 0; x < tile.get_Width(); x++)
		{
			const byte *data = tile.data(x, y);
			++hist[data[0]].r;
			++hist[data[1]].g;
			++hist[data[2]].b;
		}
	}

	return hist;
}


void showHistogram(const Histogram &hist)
{
	unsigned int r, g, b;
	r = g = b = 0;
	for(unsigned int i = 0; i < 256; ++i)
	{
		std::cout << i << " " << hist[i].r << " " << hist[i].g << " " << hist[i].b << std::endl;
		r += hist[i].r;
		g += hist[i].g;
		b += hist[i].b;
	}
	//std::cout << "sum " << r << " " << g << " " << b << std::endl;

	return;
}


void getThresholdValues(RGB &black, RGB &white, const Histogram &hist, unsigned int threshold)
{
	black.r = black.g = black.b = -1;
	white.r = white.g = white.b = 256;

	RGB blackCount;
	RGB whiteCount;

	for(unsigned int i = 0; i < 256; ++i)
	{
		if(blackCount.r <= threshold)
		{
			blackCount.r += hist[i].r;
			++black.r;
		}

		if(blackCount.g <= threshold)
		{
			blackCount.g += hist[i].g;
			++black.g;
		}

		if(blackCount.b <= threshold)
		{
			blackCount.b += hist[i].b;
			++black.b;
		}

		if(whiteCount.r <= threshold)
		{
			whiteCount.r += hist[255 - i].r;
			--white.r;
		}

		if(whiteCount.g <= threshold)
		{
			whiteCount.g += hist[255 - i].g;
			--white.g;
		}

		if(whiteCount.b <= threshold)
		{
			whiteCount.b += hist[255 - i].b;
			--white.b;
		}
	}
}
