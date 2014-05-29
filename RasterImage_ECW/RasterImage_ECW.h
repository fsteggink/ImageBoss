// RasterImage_ECW.h
// FS, 03-05-2008

#ifndef RASTERIMAGE_ECW_H
#define RASTERIMAGE_ECW_H

#ifndef DLLAPI_RASTERIMAGE_ECW
	#ifdef DLL_INTERNAL_COMPILE
		#define DLLAPI_RASTERIMAGE_ECW __declspec(dllexport)
	#else
		#define DLLAPI_RASTERIMAGE_ECW __declspec(dllimport)
	#endif
#endif

#include <fstream>
#include "../../Inc/ImageBoss.h"
#include "../../Inc/RasterLayer_extern.h"
#include "../ECWHandler/ECWHandler.h"


////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////

const Interface g_ceSupportedInterfaces = 
	//static_cast<Interface>(Interface::RasterSave | Interface::RasterLineSave);
	static_cast<Interface>(Interface::RasterSave);


////////////////////////////////////////////////////////////
// Exported function declarations
////////////////////////////////////////////////////////////

bool DLLAPI_RASTERIMAGE_ECW SupportsInterface(Interface eIF);
Interface DLLAPI_RASTERIMAGE_ECW SupportedInterfaces();
boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_ECW CreatePlugin(Interface eIF);


////////////////////////////////////////////////////////////
// Struct definitions
////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////

//class LoadPlugin_ECW: public IRasterLoadPlugin
//{
//public:
//	LoadPlugin_ECW();
//	~LoadPlugin_ECW();
//
//	FileMode CanLoad(const MimeType &mimeType) const;
//	void Load(RasterImage &image, const std::string &fileName) const;
//	void Load(RasterImage &image, const std::string &fileName, const Params &params) const;
//	void Load(RasterImage &image, std::istream &input) const;
//	void Load(RasterImage &image, std::istream &input, const Params &params) const;
//};


class SavePlugin_ECW: public IRasterSavePlugin
{
	static boost::shared_ptr<IRasterLayer> ms_layer;

	static void ReadLineWrapper(RasterLayer<byte>::Scanline &sl, int y);

public:
	SavePlugin_ECW();
	~SavePlugin_ECW();

	FileMode CanSave(const MimeType &mimeType) const;
	void Save(const RasterImage &image, const std::string &fileName) const;
	void Save(const RasterImage &image, const std::string &fileName, const Params &params) const;
	void Save(const RasterImage &image, std::ostream &output) const;
	void Save(const RasterImage &image, std::ostream &output, const Params &params) const;
};


//class InfoPlugin_ECW: public IRasterInfoPlugin
//{
//public:
//	InfoPlugin_ECW();
//	~InfoPlugin_ECW();
//
//	bool CanInfo(const MimeType &mimeType) const;
//	void Info(const std::string &fileName, int &width, int &height, int &samplesPerPixel,
//		int &bitsPerSample) const;
//};


//class LoadByLinePlugin_ECW: public IRasterLoadByLinePlugin
//{
//	std::ifstream m_fIn;
//
//public:
//	LoadByLinePlugin_ECW();
//	~LoadByLinePlugin_ECW();
//
//	FileMode CanLoad(const MimeType &mimeType);
//	IContext *Open(const std::string &fileName);
//	IContext *Open(std::istream &input);
//	void ReadScanline(IContext *context, RasterLayer<byte>::Scanline &scanline);
//	void Close(IContext *&context);
//
//	int GetWidth(IContext *context) const;
//	int GetHeight(IContext *context) const;
//	int GetSamplesPerPixel(IContext *context) const;
//};


//class SaveByLinePlugin_ECW: public IRasterSaveByLinePlugin
//{
//	std::ofstream m_fOut;
//
//public:
//	SaveByLinePlugin_ECW();
//	~SaveByLinePlugin_ECW();
//
//	FileMode CanSave(const MimeType &mimeType);
//	IContext *Open(const std::string &fileName, int width, int height, int spp);
//	IContext *Open(std::ostream &output, int width, int height, int spp);
//	void WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanline);
//	void Close(IContext *&context);
//};


//class LoadByLineContext: public IContext
//{
//public:
//	LoadByLineContext(std::istream &strIn): input(strIn), isFile(false)
//	{
//	}
//
//	jpeg_decompress_struct cinfo;
//	jpeg_error_mgr jerr;
//	std::istream &input;
//	bool isFile;
//};


//class SaveByLineContext: public IContext
//{
//public:
//	SaveByLineContext(std::ostream &strOut): output(strOut), isFile(false)
//	{
//	}
//
//	//jpeg_compress_struct cinfo;
//	//jpeg_error_mgr jerr;
//	std::ostream &output;
//	bool isFile;
//};


void initDic(int initCodeSize, std::vector<std::string> &gifDic);


////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////

//extern boost::shared_ptr<LoadPlugin_ECW> g_LoadPlugin;
extern boost::shared_ptr<SavePlugin_ECW> g_SavePlugin;
//extern boost::shared_ptr<InfoPlugin_ECW> g_InfoPlugin;
//extern boost::shared_ptr<LoadByLinePlugin_ECW> g_LoadByLinePlugin;
//extern boost::shared_ptr<SaveByLinePlugin_ECW> g_SaveByLinePlugin;


#endif // RASTERIMAGE_ECW_H
