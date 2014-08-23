// RasterImage_BMP.h
// FS, 10-03-2007

#ifndef RASTERIMAGE_BMP_H
#define RASTERIMAGE_BMP_H

#ifndef DLLAPI_RASTERIMAGE_BMP
	#ifdef DLL_INTERNAL_COMPILE
		#define DLLAPI_RASTERIMAGE_BMP __declspec(dllexport)
	#else
		#define DLLAPI_RASTERIMAGE_BMP __declspec(dllimport)
	#endif
#endif

#include "../../Inc/ImageBoss.h"
#include "../../Inc/RasterLayer.h"
#include <fstream>


////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////

const Interface g_ceSupportedInterfaces = 
	static_cast<Interface>(Interface::RasterLoad | Interface::RasterSave | Interface::RasterInfo);


////////////////////////////////////////////////////////////
// Exported function declarations
////////////////////////////////////////////////////////////

bool DLLAPI_RASTERIMAGE_BMP SupportsInterface(Interface eIF);
Interface DLLAPI_RASTERIMAGE_BMP SupportedInterfaces();
boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_BMP CreatePlugin(Interface eIF);


////////////////////////////////////////////////////////////
// Struct definitions
////////////////////////////////////////////////////////////

#pragma pack(push, 1)
struct BMPHeader
{
	char bfType1;
	char bfType2;
	long bfSize;
	short bfReserved1;
	short bfReserved2;
	long bfOffBits;
	long biSize;
	long biWidth;
	long biHeight;
	short biPlanes;
	short biBitCount;
	long biCompression;
	long biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	long biClrUsed;
	long biClrImportant;
};
#pragma pack(pop)


////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////

class LoadPlugin_BMP: public IRasterLoadPlugin
{
public:
	LoadPlugin_BMP();
	~LoadPlugin_BMP();

	FileMode CanLoad(const MimeType &mimeType) const;
	void Load(RasterImage &image, const std::string &fileName) const;
	void Load(RasterImage &image, const std::string &fileName, const Params &params) const;
	void Load(RasterImage &image, std::istream &input) const;
	void Load(RasterImage &image, std::istream &input, const Params &params) const;
};


class SavePlugin_BMP: public IRasterSavePlugin
{
public:
	SavePlugin_BMP();
	~SavePlugin_BMP();

	FileMode CanSave(const MimeType &mimeType) const;
	void Save(const RasterImage &image, const std::string &fileName) const;
	void Save(const RasterImage &image, const std::string &fileName, const Params &params) const;
	void Save(const RasterImage &image, std::ostream &output) const;
	void Save(const RasterImage &image, std::ostream &output, const Params &params) const;
};


class InfoPlugin_BMP: public IRasterInfoPlugin
{
public:
	InfoPlugin_BMP();
	~InfoPlugin_BMP();

	bool CanInfo(const MimeType &mimeType) const;
	void Info(const std::string &fileName, int &width, int &height, int &samplesPerPixel,
		int &bitsPerSample) const;
};


////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////

extern boost::shared_ptr<LoadPlugin_BMP> g_LoadPlugin;
extern boost::shared_ptr<SavePlugin_BMP> g_SavePlugin;
extern boost::shared_ptr<InfoPlugin_BMP> g_InfoPlugin;


#endif // RASTERIMAGE_BMP_H
