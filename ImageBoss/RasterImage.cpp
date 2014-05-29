// RasterImage.cpp
// FS, 11-03-2007

#define DLLAPI_IMAGEBOSS __declspec(dllexport)

//#include "ImageBoss_internal.h"
#include "RasterLayer_intern.h"


RasterImage::MimeMap RasterImage::ms_mapMimeTypes;


RasterImage::RasterImage()
{
}


RasterImage::~RasterImage()
{
	m_vecLayers.clear();
}


void RasterImage::Init()
{
	// TODO: have them added by the plugin
	ms_mapMimeTypes["bmp"] = MIME_BMP;
	ms_mapMimeTypes["jpg"] = MIME_JPEG;
	ms_mapMimeTypes["jpeg"] = MIME_JPEG;
	ms_mapMimeTypes["gif"] = MIME_GIF;
	ms_mapMimeTypes["png"] = MIME_PNG;
	ms_mapMimeTypes["ecw"] = MIME_ECW;
	ms_mapMimeTypes["tif"] = MIME_TIFF;
	ms_mapMimeTypes["tiff"] = MIME_TIFF;
}


void RasterImage::Load(const std::string &fileName)
{
	Load(fileName, Params());

	return;
}


void RasterImage::Load(const std::string &fileName, const Params &params)
{
	std::string::size_type iPos = fileName.rfind(".");
	std::string sExt = (iPos != std::string::npos) ? fileName.substr(iPos + 1) : "";
	MimeMap::const_iterator iterMime = ms_mapMimeTypes.find(sExt);

	if(iterMime == ms_mapMimeTypes.end())
		throw ImageBossException(EXC_MIMETYPE_NOT_FOUND);

	// Find the plugin which can load the image, and load it
	// TODO: what to do when the image can't be loaded?
	std::vector<boost::shared_ptr<IImagePlugin>> &vecPlugins = g_mapPlugins[Interface::RasterLoad];
	for(std::vector<boost::shared_ptr<IImagePlugin>>::const_iterator iterPlugin = vecPlugins.begin(); \
		iterPlugin != vecPlugins.end(); ++iterPlugin)
	{
		boost::shared_ptr<IRasterLoadPlugin> ifPlugin =
			boost::dynamic_pointer_cast<IRasterLoadPlugin, IImagePlugin>(*iterPlugin);
		if(ifPlugin->CanLoad(iterMime->second))
		{
			ifPlugin->Load(*this, fileName, params);
			break;
		}
	}

	return;
}


void RasterImage::Load(std::istream &input, const MimeType &mimeType)
{
	Load(input, mimeType, Params());

	return;
}


void RasterImage::Load(std::istream &input, const MimeType &mimeType, const Params &params)
{
	// Find the plugin which can load the image, and load it
	// TODO: what to do when the image can't be loaded?
	std::vector<boost::shared_ptr<IImagePlugin>> &vecPlugins = g_mapPlugins[Interface::RasterLoad];
	for(std::vector<boost::shared_ptr<IImagePlugin>>::const_iterator iterPlugin = vecPlugins.begin(); \
		iterPlugin != vecPlugins.end(); ++iterPlugin)
	{
		boost::shared_ptr<IRasterLoadPlugin> ifPlugin =
			boost::dynamic_pointer_cast<IRasterLoadPlugin, IImagePlugin>(*iterPlugin);
		if(ifPlugin->CanLoad(mimeType))
		{
			ifPlugin->Load(*this, input, params);
			break;
		}
	}

	return;
}


void RasterImage::Save(const std::string &fileName) const
{
	Save(fileName, Params());

	return;
}


void RasterImage::Save(const std::string &fileName, const Params &params) const
{
	std::string::size_type iPos = fileName.rfind(".");
	std::string sExt = (iPos != std::string::npos) ? fileName.substr(iPos + 1) : "";
	MimeMap::const_iterator iterMime = ms_mapMimeTypes.find(sExt);

	if(iterMime == ms_mapMimeTypes.end())
		throw ImageBossException(EXC_MIMETYPE_NOT_FOUND);

	// Find the plugin which can save the image, and save it
	// TODO: what to do when the image can't be saved?
	bool bImageSaved = false;
	std::vector<boost::shared_ptr<IImagePlugin>> &vecPlugins = g_mapPlugins[Interface::RasterSave];
	for(std::vector<boost::shared_ptr<IImagePlugin>>::const_iterator iterPlugin = vecPlugins.begin(); \
		iterPlugin != vecPlugins.end(); ++iterPlugin)
	{
		boost::shared_ptr<IRasterSavePlugin> ifPlugin =
			boost::dynamic_pointer_cast<IRasterSavePlugin, IImagePlugin>(*iterPlugin);
		if(ifPlugin->CanSave(iterMime->second))
		{
			ifPlugin->Save(*this, fileName, params);
			bImageSaved = true;
			break;
		}
	}

	if(!bImageSaved)
	{
		std::cout << "Can't save image of mime type " << iterMime->second << std::endl;
	}

	return;
}


void RasterImage::Save(std::ostream &output, const MimeType &mimeType) const
{
	Save(output, mimeType, Params());

	return;
}


void RasterImage::Save(std::ostream &output, const MimeType &mimeType, const Params &params) const
{
	// Find the plugin which can save the image, and save it
	// TODO: what to do when the image can't be saved?
	std::vector<boost::shared_ptr<IImagePlugin>> &vecPlugins = g_mapPlugins[Interface::RasterSave];
	for(std::vector<boost::shared_ptr<IImagePlugin>>::const_iterator iterPlugin = vecPlugins.begin(); \
		iterPlugin != vecPlugins.end(); ++iterPlugin)
	{
		boost::shared_ptr<IRasterSavePlugin> ifPlugin =
			boost::dynamic_pointer_cast<IRasterSavePlugin, IImagePlugin>(*iterPlugin);
		if(ifPlugin->CanSave(mimeType))
		{
			ifPlugin->Save(*this, output, params);
			break;
		}
	}

	return;
}


void RasterImage::GetImageData(const std::string &fileName, int &width, int &height,
							   int &samplesPerPixel, int &bitsPerSample)
{
	width = height = samplesPerPixel = bitsPerSample = -1;

	std::string::size_type iPos = fileName.rfind(".");
	std::string sExt = (iPos != std::string::npos) ? fileName.substr(iPos + 1) : "";
	MimeMap::const_iterator iterMime = ms_mapMimeTypes.find(sExt);

	if(iterMime == ms_mapMimeTypes.end())
		throw ImageBossException(EXC_MIMETYPE_NOT_FOUND);

	std::vector<boost::shared_ptr<IImagePlugin>> &vecPlugins = g_mapPlugins[Interface::RasterInfo];
	for(std::vector<boost::shared_ptr<IImagePlugin>>::const_iterator iterPlugin = vecPlugins.begin(); \
		iterPlugin != vecPlugins.end(); ++iterPlugin)
	{
		boost::shared_ptr<IRasterInfoPlugin> ifPlugin =
			boost::dynamic_pointer_cast<IRasterInfoPlugin, IImagePlugin>(*iterPlugin);
		if(ifPlugin->CanInfo(iterMime->second))
		{
			ifPlugin->Info(fileName, width, height, samplesPerPixel, bitsPerSample);
			break;
		}
	}

	return;
}


RasterLayerList &RasterImage::Layers()
{
	return m_vecLayers;
}


const RasterLayerList &RasterImage::Layers() const
{
	return m_vecLayers;
}


void RasterImage::Invoke(const std::string &methodName)
{
	std::vector<boost::shared_ptr<IImagePlugin>> &vecPlugins = g_mapPlugins[Interface::Color];
	for(std::vector<boost::shared_ptr<IImagePlugin>>::const_iterator iterPlugin = vecPlugins.begin(); \
		iterPlugin != vecPlugins.end(); ++iterPlugin)
	{
		boost::shared_ptr<IColorManipulationPlugin> ifPlugin =
			boost::dynamic_pointer_cast<IColorManipulationPlugin, IImagePlugin>(*iterPlugin);
		try
		{
			ifPlugin->Invoke(*m_vecLayers[0], methodName, Params());
			break;
		}
		catch(...)
		{
			// catch all
		}
	}

	return;
}


IContext *RasterImageReader::Open(const std::string &fileName)
{
	std::string::size_type iPos = fileName.rfind(".");
	std::string sExt = (iPos != std::string::npos) ? fileName.substr(iPos + 1) : "";
	RasterImage::MimeMap::const_iterator iterMime = RasterImage::ms_mapMimeTypes.find(sExt);

	if(iterMime == RasterImage::ms_mapMimeTypes.end())
		throw ImageBossException(EXC_MIMETYPE_NOT_FOUND);

	// Find the plugin which can load the image, and load it
	// TODO: what to do when the image can't be loaded?
	std::vector<boost::shared_ptr<IImagePlugin>> &vecPlugins = g_mapPlugins[Interface::RasterLineLoad];
	IContext *oContext = 0;

	for(std::vector<boost::shared_ptr<IImagePlugin>>::const_iterator iterPlugin = vecPlugins.begin(); \
		iterPlugin != vecPlugins.end(); ++iterPlugin)
	{
		boost::shared_ptr<IRasterLoadByLinePlugin> ifPlugin =
			boost::dynamic_pointer_cast<IRasterLoadByLinePlugin, IImagePlugin>(*iterPlugin);

		if(!ifPlugin)
			continue;

		if(ifPlugin->CanLoad(iterMime->second))
		{
			m_ifPlugin = ifPlugin;
			oContext = m_ifPlugin->Open(fileName);
			break;
		}
	}

	if(!oContext)
	{
		std::cout << "Can't load image of mime type " << iterMime->second << std::endl;
	}

	m_width = m_ifPlugin->GetWidth(oContext);
	m_height = m_ifPlugin->GetHeight(oContext);
	m_spp = m_ifPlugin->GetSamplesPerPixel(oContext);

	return oContext;
}


IContext *RasterImageReader::Open(std::istream &input, const MimeType &mimeType)
{
	// Find the plugin which can load the image, and load it
	// TODO: what to do when the image can't be loaded?
	std::vector<boost::shared_ptr<IImagePlugin>> &vecPlugins = g_mapPlugins[Interface::RasterLineLoad];
	IContext *oContext = 0;

	for(std::vector<boost::shared_ptr<IImagePlugin>>::const_iterator iterPlugin = vecPlugins.begin(); \
		iterPlugin != vecPlugins.end(); ++iterPlugin)
	{
		boost::shared_ptr<IRasterLoadByLinePlugin> ifPlugin =
			boost::dynamic_pointer_cast<IRasterLoadByLinePlugin, IImagePlugin>(*iterPlugin);

		if(!ifPlugin)
			continue;

		if(ifPlugin->CanLoad(mimeType))
		{
			m_ifPlugin = ifPlugin;
			oContext = m_ifPlugin->Open(input);
			break;
		}
	}

	m_width = m_ifPlugin->GetWidth(oContext);
	m_height = m_ifPlugin->GetHeight(oContext);
	m_spp = m_ifPlugin->GetSamplesPerPixel(oContext);

	return oContext;
}


void RasterImageReader::ReadScanline(IContext *context, RasterLayer<byte>::Scanline &scanline)
{
	if(m_ifPlugin.get())
		m_ifPlugin->ReadScanline(context, scanline);

	return;
}


void RasterImageReader::Close(IContext *context)
{
	if(m_ifPlugin.get())
		m_ifPlugin->Close(context);

	return;
}


int RasterImageReader::get_Width() const
{
	return m_width;
}


int RasterImageReader::get_Height() const
{
	return m_height;
}


int RasterImageReader::get_SamplesPerPixel() const
{
	return m_spp;
}


int RasterImageReader::get_ScanlineWidth() const
{
	return m_width * m_spp;
}


RasterImageWriter::RasterImageWriter(): m_width(0), m_height(0), m_spp(0)
{
}


IContext *RasterImageWriter::Open(const std::string &fileName, int width, int height, int spp)
{
	std::string::size_type iPos = fileName.rfind(".");
	std::string sExt = (iPos != std::string::npos) ? fileName.substr(iPos + 1) : "";
	RasterImage::MimeMap::const_iterator iterMime = RasterImage::ms_mapMimeTypes.find(sExt);

	if(iterMime == RasterImage::ms_mapMimeTypes.end())
		throw ImageBossException(EXC_MIMETYPE_NOT_FOUND);

	// Find the plugin which can save the image, and save it
	// TODO: what to do when the image can't be saved?
	std::vector<boost::shared_ptr<IImagePlugin>> &vecPlugins = g_mapPlugins[Interface::RasterLineSave];
	IContext *oContext = 0;

	for(std::vector<boost::shared_ptr<IImagePlugin>>::const_iterator iterPlugin = vecPlugins.begin(); \
		iterPlugin != vecPlugins.end(); ++iterPlugin)
	{
		boost::shared_ptr<IRasterSaveByLinePlugin> ifPlugin =
			boost::dynamic_pointer_cast<IRasterSaveByLinePlugin, IImagePlugin>(*iterPlugin);

		if(!ifPlugin)
			continue;

		if(ifPlugin->CanSave(iterMime->second))
		{
			m_ifPlugin = ifPlugin;
			oContext = m_ifPlugin->Open(fileName, width, height, spp);
			break;
		}
	}

	if(!oContext)
	{
		std::cout << "Can't open (save by scanline) image of mime type " << iterMime->second << std::endl;
	}

	m_width = width;
	m_height = height;
	m_spp = spp;

	return oContext;
}


IContext *RasterImageWriter::Open(std::ostream &output, const MimeType &mimeType, int width, int height, int spp)
{
	// Find the plugin which can save the image, and save it
	// TODO: what to do when the image can't be saved?
	std::vector<boost::shared_ptr<IImagePlugin>> &vecPlugins = g_mapPlugins[Interface::RasterLineSave];
	IContext *oContext = 0;

	for(std::vector<boost::shared_ptr<IImagePlugin>>::const_iterator iterPlugin = vecPlugins.begin(); \
		iterPlugin != vecPlugins.end(); ++iterPlugin)
	{
		boost::shared_ptr<IRasterSaveByLinePlugin> ifPlugin =
			boost::dynamic_pointer_cast<IRasterSaveByLinePlugin, IImagePlugin>(*iterPlugin);

		if(!ifPlugin)
			continue;

		if(ifPlugin->CanSave(mimeType))
		{
			m_ifPlugin = ifPlugin;
			oContext = m_ifPlugin->Open(output, width, height, spp);
			break;
		}
	}

	m_width = width;
	m_height = height;
	m_spp = spp;

	return oContext;
}


void RasterImageWriter::WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanline)
{
	if(m_ifPlugin.get())
		m_ifPlugin->WriteScanline(context, scanline);

	return;
}


void RasterImageWriter::Close(IContext *context)
{
	if(m_ifPlugin.get())
		m_ifPlugin->Close(context);

	return;
}


int RasterImageWriter::get_ScanlineWidth() const
{
	return m_width * m_spp;
}
