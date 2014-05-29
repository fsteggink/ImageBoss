// RasterImage_ECW.cpp
// FS, 03-05-2008

#define DLLAPI_RASTERIMAGE_ECW __declspec(dllexport)

#include "RasterImage_ECW.h"

//boost::shared_ptr<LoadPlugin_ECW> g_LoadPlugin;
boost::shared_ptr<SavePlugin_ECW> g_SavePlugin;
//boost::shared_ptr<InfoPlugin_ECW> g_InfoPlugin;
//boost::shared_ptr<LoadByLinePlugin_ECW> g_LoadByLinePlugin;
//boost::shared_ptr<SaveByLinePlugin_ECW> g_SaveByLinePlugin;

boost::shared_ptr<IRasterLayer> SavePlugin_ECW::ms_layer;


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

    return TRUE;
}


bool DLLAPI_RASTERIMAGE_ECW SupportsInterface(Interface eIF)
{
	return (g_ceSupportedInterfaces & eIF) != 0;
}


Interface DLLAPI_RASTERIMAGE_ECW SupportedInterfaces()
{
	return g_ceSupportedInterfaces;
}


boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_ECW CreatePlugin(Interface eIF)
{
	switch(eIF)
	{
	//case Interface::RasterLoad:
	//	g_LoadPlugin = boost::shared_ptr<LoadPlugin_ECW>(new LoadPlugin_ECW());
	//	return g_LoadPlugin;
	//	break;

	case Interface::RasterSave:
		g_SavePlugin = boost::shared_ptr<SavePlugin_ECW>(new SavePlugin_ECW());
		return g_SavePlugin;
		break;

	//case Interface::RasterInfo:
	//	g_InfoPlugin = boost::shared_ptr<InfoPlugin_ECW>(new InfoPlugin_ECW());
	//	return g_InfoPlugin;
	//	break;

	//case Interface::RasterLineLoad:
	//	g_LoadByLinePlugin = boost::shared_ptr<LoadByLinePlugin_ECW>(new LoadByLinePlugin_ECW());
	//	return g_LoadByLinePlugin;
	//	break;

	//case Interface::RasterLineSave:
	//	g_SaveByLinePlugin = boost::shared_ptr<SaveByLinePlugin_ECW>(new SaveByLinePlugin_ECW());
	//	return g_SaveByLinePlugin;
	//	break;
	}

	return boost::shared_ptr<IImagePlugin>();
}


SavePlugin_ECW::SavePlugin_ECW()
{
}


SavePlugin_ECW::~SavePlugin_ECW()
{
}


FileMode SavePlugin_ECW::CanSave(const MimeType &mimeType) const
{
	if(mimeType == MIME_ECW)
		return FileMode::All;
	else
		return FileMode::None;
}


void SavePlugin_ECW::Save(const RasterImage &image, const std::string &fileName) const
{
	Save(image, fileName, Params());

	return;
}


void SavePlugin_ECW::Save(const RasterImage &image, const std::string &fileName, const Params &params) const
{
	ms_layer = image.Layers()[0];
	CreateECW(fileName.c_str(), ms_layer->get_Width(), ms_layer->get_Height(), SavePlugin_ECW::ReadLineWrapper);

	return;
}


void SavePlugin_ECW::Save(const RasterImage &image, std::ostream &output) const
{
	Save(image, output, Params());

	return;
}


void SavePlugin_ECW::Save(const RasterImage &image, std::ostream &output, const Params &params) const
{
	// Save as file, then write contents of file to output stream

	return;
}


void SavePlugin_ECW::ReadLineWrapper(RasterLayer<byte>::Scanline &sl, int y)
{
	RasterLayer<byte> &layer =
		*boost::dynamic_pointer_cast<RasterLayer<byte>, IRasterLayer>(ms_layer);
	
	layer.getScanline(sl.begin(), y);
	
	return;
}


//SaveByLinePlugin_ECW::SaveByLinePlugin_ECW()
//{
//}
//
//
//SaveByLinePlugin_ECW::~SaveByLinePlugin_ECW()
//{
//}
//
//
//FileMode SaveByLinePlugin_ECW::CanSave(const MimeType &mimeType)
//{
//	if(mimeType == MIME_ECW)
//		return FileMode::All;
//	else
//		return FileMode::None;
//}
//
//
//IContext *SaveByLinePlugin_ECW::Open(const std::string &fileName, int width, int height, int spp)
//{
//}
//
//
//IContext *SaveByLinePlugin_ECW::Open(std::ostream &output, int width, int height, int spp)
//{
//}
//
//
//void SaveByLinePlugin_ECW::WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanline)
//{
//}
//
//
//void SaveByLinePlugin_ECW::Close(IContext *&context)
//{
//}
