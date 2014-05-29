// RasterImage_PNG.h
// FS, 25-01-2009

#ifndef RASTERIMAGE_PNG_H
#define RASTERIMAGE_PNG_H

#ifndef DLLAPI_RASTERIMAGE_PNG
	#ifdef DLL_INTERNAL_COMPILE
		#define DLLAPI_RASTERIMAGE_PNG __declspec(dllexport)
	#else
		#define DLLAPI_RASTERIMAGE_PNG __declspec(dllimport)
	#endif
#endif

#include "../../Inc/ImageBoss.h"
#include "../../Inc/RasterLayer_extern.h"
#include <fstream>

#define ZLIB_WINAPI
#include <zlib.h>
#include <zconf.h>


////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////

const Interface g_ceSupportedInterfaces = \
static_cast<Interface>(Interface::RasterLoad | Interface::RasterSave | Interface::RasterInfo | Interface::RasterLineLoad | Interface::RasterLineSave);
//static_cast<Interface>(Interface::RasterLoad | Interface::RasterSave | Interface::RasterInfo | Interface::RasterLineSave);

const char PNG_SIGNATURE[] = { '\x89', 'P', 'N', 'G', '\x0D', '\x0A', '\x1A', '\x0a' };
const int MAX_PNG_SIZE = 0x7FFFFFFF;
const int DEFLATE_BUFFER = 0x8000;

// d:\Dev\Win\VisualCPP_Frank.zip\VisualCPP_Frank\Graphics\Wallabie\ bevat oude PNG code


////////////////////////////////////////////////////////////
// Exported function declarations
////////////////////////////////////////////////////////////

bool DLLAPI_RASTERIMAGE_PNG SupportsInterface(Interface eIF);
Interface DLLAPI_RASTERIMAGE_PNG SupportedInterfaces();
boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_PNG CreatePlugin(Interface eIF);


////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////

inline void swapShort(unsigned short &value)
{
	value = (value & 0xFF00) >> 8 | (value & 0xFF) << 8;
}

inline void swapInt(unsigned int &value)
{
	value =
		(value & 0xFF000000) >> 24 |
		(value & 0xFF0000) >> 8 |
		(value & 0xFF00) << 8 |
		(value & 0xFF) << 24;
}

inline void swapLong(unsigned __int64 &value)
{
	value =
		(value & 0xFF00000000000000) >> 56 |
		(value & 0xFF000000000000) >> 40 |
		(value & 0xFF0000000000) >> 24 |
		(value & 0xFF00000000) >> 8 |
		(value & 0xFF000000) << 8 |
		(value & 0xFF0000) << 24 |
		(value & 0xFF00) << 40 |
		(value & 0xFF) << 56;
}

inline unsigned short swapShortValue(unsigned short value)
{
	return (value & 0xFF00) >> 8 | (value & 0xFF) << 8;
}

inline unsigned int swapIntValue(unsigned int value)
{
	return
		(value & 0xFF000000) >> 24 |
		(value & 0xFF0000) >> 8 |
		(value & 0xFF00) << 8 |
		(value & 0xFF) << 24;
}

inline unsigned __int64 swapLongValue(unsigned __int64 value)
{
	return
		(value & 0xFF00000000000000) >> 56 |
		(value & 0xFF000000000000) >> 40 |
		(value & 0xFF0000000000) >> 24 |
		(value & 0xFF00000000) >> 8 |
		(value & 0xFF000000) << 8 |
		(value & 0xFF0000) << 24 |
		(value & 0xFF00) << 40 |
		(value & 0xFF) << 56;
}

inline int paethPredictor(byte a, byte b, byte c)
{
	int p, pa, pb, pc;
	
	p = a + b - c;
	pa = abs(p - a);
	pb = abs(p - b);
	pc = abs(p - c);

	if(pa <= pb && pa <= pc) 
		return a;
	else if(pb <= pc) 
		return b;
	else 
		return c;
}

inline byte filterNone(int sign, byte argA, byte argB, byte argC, byte argD)
{
	return argA;
}

inline byte filterSub(int sign, byte argA, byte argB, byte argC, byte argD)
{
	return byte((int(argA) + sign * int(argB)) & 0xFF);
}

inline byte filterUp(int sign, byte argA, byte argB, byte argC, byte argD)
{
	return byte((int(argA) + sign * int(argC)) & 0xFF);
}

inline byte filterAverage(int sign, byte argA, byte argB, byte argC, byte argD)
{
	return byte((int(argA) + sign * (int(argB) + int(argC)) / 2) & 0xFF);
}

inline byte filterPaeth(int sign, byte argA, byte argB, byte argC, byte argD)
{
	return byte((int(argA) + sign * paethPredictor(argB, argC, argD)) & 0xFF);
}


////////////////////////////////////////////////////////////
// Struct definitions
////////////////////////////////////////////////////////////

#pragma pack(push, 1)
struct IHDR_Data
{
	unsigned int width;
	unsigned int height;
	unsigned char bitDepth;
	unsigned char colorType;
	unsigned char compression;
	unsigned char filter;
	unsigned char interlace;
};

#pragma pack(pop)


////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////

struct PNGChunk;

class LoadPlugin_PNG: public RasterLoadPlugin
{
public:
	FileMode CanLoad(const MimeType &mimeType) const;
	boost::shared_ptr<IRasterLoadByLinePlugin> GetLoadByLinePlugin() const;
};


class SavePlugin_PNG: public RasterSavePlugin
{
public:
	FileMode CanSave(const MimeType &mimeType) const;
	boost::shared_ptr<IRasterSaveByLinePlugin> GetSaveByLinePlugin() const;
};


class InfoPlugin_PNG: public IRasterInfoPlugin
{
public:
	bool CanInfo(const MimeType &mimeType) const;
	void Info(const std::string &fileName, int &width, int &height, int &samplesPerPixel,
		int &bitsPerSample) const;
};


class LoadByLinePlugin_PNG: public IRasterLoadByLinePlugin
{
	mutable std::ifstream m_fIn;

public:
	FileMode CanLoad(const MimeType &mimeType) const;
	IContext *Open(const std::string &fileName) const;
	IContext *Open(std::istream &input) const;
	void ReadScanline(IContext *context, RasterLayer<byte>::Scanline &scanline) const;
	void Close(IContext *&context) const;

	int GetWidth(IContext *context) const;
	int GetHeight(IContext *context) const;
	int GetSamplesPerPixel(IContext *context) const;
	bool HasPalette(IContext *context) const;
	const RasterLayer<byte>::Palette &GetPalette(IContext *context) const;

private:
	void readPalette(RasterLayer<byte>::Palette &palette, const PNGChunk &chunk) const;
};


class SaveByLineContext;

class SaveByLinePlugin_PNG: public IRasterSaveByLinePlugin
{
	mutable std::ofstream m_fOut;

public:
	FileMode CanSave(const MimeType &mimeType) const;
	IContext *Open(const std::string &fileName, int width, int height, int spp) const;
	IContext *Open(std::ostream &output, int width, int height, int spp) const;
	void WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanline) const;
	void Close(IContext *&context) const;

private:
	void applyFilter(RasterLayer<byte>::Scanline &scanline, const RasterLayer<byte>::Scanline &scanlineOrig,
		const RasterLayer<byte>::Scanline &scanlinePrev, unsigned int slw, unsigned int spp, unsigned int bps) const;
	void deflateAndWriteData(int flush, SaveByLineContext *oContext) const;
	int sumArray(const std::vector<byte> &array) const;
	double entropyArray(const std::vector<byte> &array) const;
};


class LoadByLineContext: public IContext
{
public:
	LoadByLineContext(std::istream &strIn): input(strIn), isFile(false)
	{
	}

	std::istream &input;
	bool isFile;
	IHDR_Data header;
	z_stream strm;
	RasterLayer<byte>::Scanline scanlinePrev;
	RasterLayer<byte>::Palette palette;

	// temporary
	boost::shared_array<byte> data;
	int y;
};


class SaveByLineContext: public IContext
{
public:
	SaveByLineContext(std::ostream &strOut): output(strOut), isFile(false)
	{
	}

	std::ostream &output;
	bool isFile;

	z_stream strm;
	boost::shared_array<byte> compData;
	RasterLayer<byte>::Scanline scanlinePrev;
	IHDR_Data header;
};

struct PNGColorType
{
	static const unsigned int Grayscale = 0;
	static const unsigned int RGB = 2;
	static const unsigned int Palette = 3;
	static const unsigned int GrayscaleAlpha = 4;
	static const unsigned int RGBAlpha = 6;

	static bool hasAlpha(unsigned int colorType);
	static bool hasColor(unsigned int colorType);
	static bool hasPalette(unsigned int colorType);
	static unsigned int samplesPerPixel(unsigned int colorType);

	static unsigned int colorType(bool hasAlpha, bool hasColor, bool hasPalette);
};

struct PNGChunk
{
	// Critical chunks
	static const unsigned int IHDR = 0x49484452;
	static const unsigned int PLTE = 0x504C5445;
	static const unsigned int IDAT = 0x49444154;
	static const unsigned int IEND = 0x49454E44;

	// Ancillary chunks
	static const unsigned int bKGD = 0x624B4744;
	static const unsigned int cHRM = 0x6348524D;
	static const unsigned int gAMA = 0x67414D41;
	static const unsigned int hIST = 0x68495354;
	static const unsigned int pHYs = 0x70485973;
	static const unsigned int sBIT = 0x73424954;
	static const unsigned int tEXt = 0x74455874;
	static const unsigned int tIME = 0x74494D45;
	static const unsigned int tRNS = 0x74524E53;
	static const unsigned int zTXt = 0x7A545874;

	static bool isAncillary(unsigned int chunk);
	static bool isPrivate(unsigned int chunk);
	static bool isReserved(unsigned int chunk);
	static bool isSafeToCopy(unsigned int chunk);

	unsigned int length;
	unsigned int chunk;
	unsigned int crc;

	boost::shared_array<unsigned char> data;

	PNGChunk();
	static PNGChunk read(std::istream &str);
	void copyData(void *dest) const;
	static void write(std::ostream &str, unsigned int chunkType, const unsigned char *data, unsigned int length);

private:

};

struct PNGCompressionMethod
{
	static const unsigned int Deflate = 0;
};

struct PNGFilterMethod
{
	static const unsigned int Adaptive = 0;
};

struct PNGInterlaceMethod
{
	static const unsigned int None = 0;
	static const unsigned int Adam7 = 1;
};

struct PNGFilterType
{
	static const unsigned int None = 0;
	static const unsigned int Sub = 1;
	static const unsigned int Up = 2;
	static const unsigned int Average = 3;
	static const unsigned int Paeth = 4;
};


////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////

// Adaptive filter function pointer
typedef byte (*filterFunc)(int sign, byte argA, byte argB, byte argC, byte argD);
filterFunc getFilter(const int filterType);
int filterScanline(filterFunc filter, int filterDir, byte *sl, const byte *slOrig, 
	const byte *slPrev, unsigned int width, unsigned int sppa, unsigned int bps);


// CRC functions
void make_crc_table(void);
unsigned int update_crc(unsigned int crc, const unsigned char *buf, unsigned int len);
//unsigned int crc(const unsigned char *buf, unsigned int len);


////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////

//extern boost::shared_ptr<LoadPlugin_PNG> g_LoadPlugin;
//extern boost::shared_ptr<SavePlugin_PNG> g_SavePlugin;
//extern boost::shared_ptr<InfoPlugin_PNG> g_InfoPlugin;
//extern boost::shared_ptr<LoadByLinePlugin_PNG> g_LoadByLinePlugin;
//extern boost::shared_ptr<SaveByLinePlugin_PNG> g_SaveByLinePlugin;

// CRC variables
extern unsigned int crc_table[256];
extern int crc_table_computed;



#endif //RASTERIMAGE_PNG_H
