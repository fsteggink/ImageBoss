// WhiteBalance.h
// FS, 20-07-2013

#ifndef WHITE_BALANCE_H
#define WHITE_BALANCE_H

#include <iostream>
#include "../../Inc/ImageGeo.h"
#include "../../Inc/Matrix.h"
#include "../../Inc/StringLib.h"
#include "../../Inc/CmdApp.h"

// Constants
const std::string PROGNAME = "WhiteBalance";
const std::string PROGVERSION = "0.1a";

const unsigned int FILTER_SIZE = 200;
const double WB_THRESHOLD = 0.005;

#undef RGB

struct RGB
{
	unsigned int r;
	unsigned int g;
	unsigned int b;

	RGB(): r(0), g(0), b(0) {}
};

typedef std::vector<RGB> Histogram;

// Functions
void applyWhiteBalance(const std::string &inputFile, const std::string &outputFile, unsigned int filterSize, double threshold);
Histogram getHistogram(const RasterLayer<byte>::Tile &tile);
void showHistogram(const Histogram &hist);
void getThresholdValues(RGB &black, RGB &white, const Histogram &hist, unsigned int threshold);

#endif // WHITE_BALANCE_H
