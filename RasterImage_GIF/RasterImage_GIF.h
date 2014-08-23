// RasterImage_GIF.h
// FS, 29-06-2007

#ifndef RASTERIMAGE_GIF_H
#define RASTERIMAGE_GIF_H

#ifndef DLLAPI_RASTERIMAGE_GIF
	#ifdef DLL_INTERNAL_COMPILE
		#define DLLAPI_RASTERIMAGE_GIF __declspec(dllexport)
	#else
		#define DLLAPI_RASTERIMAGE_GIF __declspec(dllimport)
	#endif
#endif

#include "../../Inc/ImageBoss.h"
#include "../../Inc/RasterLayer.h"
#include <fstream>


////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////

const Interface g_ceSupportedInterfaces = 
	//static_cast<Interface>(Interface::RasterLoad | Interface::RasterSave | Interface::RasterInfo);
	static_cast<Interface>(Interface::RasterLoad | Interface::RasterInfo);


////////////////////////////////////////////////////////////
// Exported function declarations
////////////////////////////////////////////////////////////

bool DLLAPI_RASTERIMAGE_GIF SupportsInterface(Interface eIF);
Interface DLLAPI_RASTERIMAGE_GIF SupportedInterfaces();
boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_GIF CreatePlugin(Interface eIF);


////////////////////////////////////////////////////////////
// Struct definitions
////////////////////////////////////////////////////////////

#pragma pack(push, 1)
struct GIFHeader
{
	char Signature[3];
	char Version[3];
};

struct GIFLogicalScreenDescriptor
{
	unsigned short Width;
	unsigned short Height;
	char PackedFields;
	char BackgroundColorIndex;
	char PixelAspectRatio;
};

/*
PackedFields = Global Color Table Flag       1 Bit
               Color Resolution              3 Bits
               Sort Flag                     1 Bit
               Size of Global Color Table    3 Bits
AspectRatio = (PixelAspectRatio + 15) / 64
*/

struct GIFImageDescriptor
{
	char Separator;   // Always 2C
	unsigned short LeftPosition;
	unsigned short TopPosition;
	unsigned short Width;
	unsigned short Height;
	char PackedFields;
};

/*
PackedFields = Local Color Table Flag        1 Bit
               Interlace Flag                1 Bit
               Sort Flag                     1 Bit
               Reserved                      2 Bits
               Size of Local Color Table     3 Bits
*/

struct GIFGraphicalControlExtension
{
	char ExtensionIntroducer;   // Always 21
	char Label;                 // Always F9
	char BlockSize;             // Always 04
	char PackedFields;
	unsigned short DelayTime;
	char TransparentControlIndex;
	char BlockTerminator;       // Always 00
};

struct GIFApplicationExtension
{
	char ExtensionIntroducer;      // Always 21
	char Label;                    // Always FF
	char BlockSize;                // Always 0B
	char ApplicationIdentifier[8];
	char AuthenticationCode[3];
	// Application data: depends on application
	// char BlockTerminator
};

#pragma pack(pop)


////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////

class LoadPlugin_GIF: public RasterLoadPlugin
{
	void ReadExtensions(std::istream &input) const;

public:
	FileMode CanLoad(const MimeType &mimeType) const;
	void Load(RasterImage &image, std::istream &input, const Params &params) const;
};


//class SavePlugin_GIF: public RasterSavePlugin
//{
//public:
//	FileMode CanSave(const MimeType &mimeType) const;
//	void Save(const RasterImage &image, std::ostream &output, const Params &params) const;
//};


class InfoPlugin_GIF: public IRasterInfoPlugin
{
public:
	bool CanInfo(const MimeType &mimeType) const;
	void Info(const std::string &fileName, int &width, int &height, int &samplesPerPixel,
		int &bitsPerSample) const;
};


void initDic(int initCodeSize, std::vector<std::string> &gifDic);


////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////

extern boost::shared_ptr<LoadPlugin_GIF> g_LoadPlugin;
//extern boost::shared_ptr<SavePlugin_GIF> g_SavePlugin;
extern boost::shared_ptr<InfoPlugin_GIF> g_InfoPlugin;


#endif // RASTERIMAGE_GIF_H
