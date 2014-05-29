// RasterImage_JPEG.h
// FS, 10-03-2007

#ifndef RASTERIMAGE_JPEG_H
#define RASTERIMAGE_JPEG_H

#ifndef DLLAPI_RASTERIMAGE_JPEG
	#ifdef DLL_INTERNAL_COMPILE
		//#define DLLAPI_RASTERIMAGE_JPEG __declspec(dllexport)
	#else
		//#define DLLAPI_RASTERIMAGE_JPEG __declspec(dllimport)
	#endif
#endif

#include "../../Inc/ImageBoss.h"
#include "../../Inc/RasterLayer_extern.h"
#include <fstream>

#undef FAR
#define XMD_H

extern "C"
{
	#include <cstdio>
	#include <jpeglib.h>
	#include <jerror.h>
}


// Declaration of the initialization functions for the source and destination
// managers for compressed JPEG data
void imageboss_jpeg_stdio_dest (j_compress_ptr cinfo, std::ostream *outstream);
void imageboss_jpeg_stdio_src (j_decompress_ptr cinfo, std::istream *instream);



////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////

const Interface g_ceSupportedInterfaces = \
static_cast<Interface>(Interface::RasterLoad | Interface::RasterSave | Interface::RasterInfo | Interface::RasterLineLoad | Interface::RasterLineSave);
//static_cast<Interface>(Interface::RasterLoad | Interface::RasterSave | Interface::RasterInfo | Interface::RasterLineSave);
//static_cast<Interface>(Interface::RasterLoad | Interface::RasterSave | Interface::RasterInfo);

const std::string g_csJPEG_QUALITY = "JPEG_QUALITY";
const int MAX_JPEG_SIZE = 65500;


////////////////////////////////////////////////////////////
// Exported function declarations
////////////////////////////////////////////////////////////

bool DLLAPI_RASTERIMAGE_JPEG SupportsInterface(Interface eIF);
Interface DLLAPI_RASTERIMAGE_JPEG SupportedInterfaces();
boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_JPEG CreatePlugin(Interface eIF);


////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////

inline int atoi(const std::string &value)
{
	return atoi(value.c_str());
}


////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////

class LoadPlugin_JPEG: public RasterLoadPlugin
{
public:
	FileMode CanLoad(const MimeType &mimeType) const;
	boost::shared_ptr<IRasterLoadByLinePlugin> GetLoadByLinePlugin() const;
};


class SavePlugin_JPEG: public RasterSavePlugin
{
public:
	FileMode CanSave(const MimeType &mimeType) const;
	boost::shared_ptr<IRasterSaveByLinePlugin> GetSaveByLinePlugin() const;
};


class InfoPlugin_JPEG: public IRasterInfoPlugin
{
public:
	bool CanInfo(const MimeType &mimeType) const;
	void Info(const std::string &fileName, int &width, int &height, int &samplesPerPixel,
		int &bitsPerSample) const;
};


class LoadByLinePlugin_JPEG: public IRasterLoadByLinePlugin
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
};


class SaveByLinePlugin_JPEG: public IRasterSaveByLinePlugin
{
	mutable std::ofstream m_fOut;

public:
	FileMode CanSave(const MimeType &mimeType) const;
	IContext *Open(const std::string &fileName, int width, int height, int spp) const;
	IContext *Open(std::ostream &output, int width, int height, int spp) const;
	void WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanline) const;
	void Close(IContext *&context) const;
};


class LoadByLineContext: public IContext
{
public:
	LoadByLineContext(std::istream &strIn): input(strIn), isFile(false), startedDecompress(false)
	{
	}

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	std::istream &input;
	bool isFile;
	bool startedDecompress;
};


class SaveByLineContext: public IContext
{
public:
	SaveByLineContext(std::ostream &strOut): output(strOut), isFile(false)
	{
	}

	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;
	std::ostream &output;
	bool isFile;
};


////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////

//extern boost::shared_ptr<LoadPlugin_JPEG> g_LoadPlugin;
//extern boost::shared_ptr<SavePlugin_JPEG> g_SavePlugin;
//extern boost::shared_ptr<InfoPlugin_JPEG> g_InfoPlugin;
//extern boost::shared_ptr<LoadByLinePlugin_JPEG> g_LoadByLinePlugin;
//extern boost::shared_ptr<SaveByLinePlugin_JPEG> g_SaveByLinePlugin;


#endif // RASTERIMAGE_JPEG_H
