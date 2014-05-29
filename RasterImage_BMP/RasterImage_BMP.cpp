// RasterImage_BMP.cpp
// FS, 10-03-2007

#define DLLAPI_RASTERIMAGE_BMP __declspec(dllexport)

#include "RasterImage_BMP.h"

boost::shared_ptr<LoadPlugin_BMP> g_LoadPlugin;
boost::shared_ptr<SavePlugin_BMP> g_SavePlugin;
boost::shared_ptr<InfoPlugin_BMP> g_InfoPlugin;


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


bool DLLAPI_RASTERIMAGE_BMP SupportsInterface(Interface eIF)
{
	return (g_ceSupportedInterfaces & eIF) != 0;
}


Interface DLLAPI_RASTERIMAGE_BMP SupportedInterfaces()
{
	return g_ceSupportedInterfaces;
}


boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_BMP CreatePlugin(Interface eIF)
{
	switch(eIF)
	{
	case Interface::RasterLoad:
		g_LoadPlugin = boost::shared_ptr<LoadPlugin_BMP>(new LoadPlugin_BMP());
		return g_LoadPlugin;
		break;

	case Interface::RasterSave:
		g_SavePlugin = boost::shared_ptr<SavePlugin_BMP>(new SavePlugin_BMP());
		return g_SavePlugin;
		break;

	case Interface::RasterInfo:
		g_InfoPlugin = boost::shared_ptr<InfoPlugin_BMP>(new InfoPlugin_BMP());
		return g_InfoPlugin;
		break;
	}

	return boost::shared_ptr<IImagePlugin>();
}


LoadPlugin_BMP::LoadPlugin_BMP()
{
}


LoadPlugin_BMP::~LoadPlugin_BMP()
{
}


FileMode LoadPlugin_BMP::CanLoad(const MimeType &mimeType) const
{
	if(mimeType == MIME_BMP)
		return FileMode::All;
	else
		return FileMode::None;
}


void LoadPlugin_BMP::Load(RasterImage &image, const std::string &fileName) const
{
	std::ifstream input(fileName.c_str(), std::ios::binary);
	if(!input)
		throw ImageBossException(EXC_CANT_LOAD_FILE);

	Load(image, input);
	input.close();

	return;
}


void LoadPlugin_BMP::Load(RasterImage &image, const std::string &fileName,
						  const Params &params) const
{
	Load(image, fileName);
	return;
}


void LoadPlugin_BMP::Load(RasterImage &image, std::istream &input) const
{
	if(!input)
		throw ImageBossException(EXC_INVALID_INPUTSTREAM);

	BMPHeader hdr;
	input.read(reinterpret_cast<char *>(&hdr), sizeof(BMPHeader));

	if(hdr.biBitCount != 8 && hdr.biBitCount != 24)
		throw ImageBossException("Input file is not an 8 or 24 color bitmap");

	boost::shared_ptr<RasterLayer<byte>> layer(
		new RasterLayer<byte>(hdr.biWidth, hdr.biHeight, hdr.biBitCount / 8));


	// Read palette
	if(hdr.biBitCount == 8)
	{
		int iPalCount = (hdr.bfOffBits - sizeof(BMPHeader)) / 4;
		RasterLayer<byte>::Palette palette(iPalCount * 3);
		RasterLayer<byte>::Scanline slPalette(iPalCount * 4);
		input.read(reinterpret_cast<char *>(const_cast<byte *>(&slPalette[0])), iPalCount * 4);

		for(int x = 0; x < iPalCount; ++x)
		{
			for(int k = 0; k < 3; ++k)
			{
				palette[x * 3 + k] = slPalette[x * 4 + (2 - k)];
			}
		}
		layer->setPalette(palette);
	}


	// Read image data
	input.seekg(hdr.bfOffBits, std::ios::beg);
	int slw = hdr.biSizeImage / hdr.biHeight;
	RasterLayer<byte>::Scanline sl(slw);
	for(int y = 0; y < hdr.biHeight; ++y)
	{
		input.read(reinterpret_cast<char *>(const_cast<byte *>(&sl[0])), slw);

		if(!layer->hasPalette())
		{
			// Swap red and blue
			for(int x = 0; x < hdr.biWidth; ++x)
			{
				//std::swap(sl[x * 3], sl[x * 3 + 2]);
				swapIntValues(sl[x * 3], sl[x * 3 + 2]);
			}
		}

		layer->setScanline(sl.begin(), hdr.biHeight - y - 1);
	}

	image.Layers().push_back(layer);

	return;
}


void LoadPlugin_BMP::Load(RasterImage &image, std::istream &input, const Params &params) const
{
	Load(image, input);
	return;
}


SavePlugin_BMP::SavePlugin_BMP()
{
}


SavePlugin_BMP::~SavePlugin_BMP()
{
}


FileMode SavePlugin_BMP::CanSave(const MimeType &mimeType) const
{
	if(mimeType == MIME_BMP)
		return FileMode::All;
	else
		return FileMode::None;
}


void SavePlugin_BMP::Save(const RasterImage &image, const std::string &fileName) const
{
	std::ofstream output(fileName.c_str(), std::ios::binary);
	if(!output)
		throw ImageBossException(EXC_CANT_SAVE_FILE);

	Save(image, output);
	output.close();

	return;
}


void SavePlugin_BMP::Save(const RasterImage &image, const std::string &fileName,
						  const Params &params) const
{
	Save(image, fileName);
	return;
}


void SavePlugin_BMP::Save(const RasterImage &image, std::ostream &output) const
{
	if(!output)
		throw ImageBossException(EXC_INVALID_OUTPUTSTREAM);
	else if(image.Layers().size() == 0)
		throw ImageBossException(EXC_NO_DATA);

	boost::shared_ptr<IRasterLayer> ifLayer = image.Layers()[0];

	if(ifLayer->get_BitsPerSample() != 8 && ifLayer->get_SamplesPerPixel() != 3)
		throw ImageBossException(EXC_CANT_SAVE_FILE);

	RasterLayer<byte> &layer =
		*boost::dynamic_pointer_cast<RasterLayer<byte>, IRasterLayer>(ifLayer);

	int width = layer.get_Width();
	int height = layer.get_Height();
	int spp = layer.get_SamplesPerPixel();
	int slw = (width * spp + sizeof(int) - 1) & 0xFFFFFFFC;

	RasterLayer<byte>::Palette palette;
	if(layer.hasPalette())
		layer.getPalette(palette);
	int iPalCount = static_cast<int>(palette.size() / 3);

	BMPHeader hdr;
	memset(&hdr, 0, sizeof(BMPHeader));
	hdr.bfType1 = 'B';
	hdr.bfType2 = 'M';
	hdr.bfSize = height * slw + sizeof(BMPHeader) + iPalCount * 4;
	//hdr.bfReserved1;
	//hdr.bfReserved2;
	hdr.bfOffBits = sizeof(BMPHeader) + iPalCount * 4;
	hdr.biSize = 40;
	hdr.biWidth = width;
	hdr.biHeight = height;
	hdr.biPlanes = 1;
	hdr.biBitCount = spp * 8;
	//hdr.biCompression;
	hdr.biSizeImage = height * slw;
	hdr.biXPelsPerMeter = 2835;
	hdr.biYPelsPerMeter = 2835;
	hdr.biClrUsed = iPalCount;
	hdr.biClrImportant = iPalCount;

	output.write(reinterpret_cast<char *>(const_cast<BMPHeader *>(&hdr)), sizeof(BMPHeader));


	// Save palette
	if(layer.hasPalette())
	{
		RasterLayer<byte>::Palette palette;
		layer.getPalette(palette);
		RasterLayer<byte>::Scanline slPalette(4 * iPalCount);
		for(int x = 0; x < iPalCount; ++x)
		{
			for(int k = 0; k < 3; ++k)
			{
				slPalette[x * 4 + (2 - k)] = palette[x * 3 + k];
			}
		}

		output.write(reinterpret_cast<char *>(const_cast<byte *>(&slPalette[0])), 4 * iPalCount);		
	}


	RasterLayer<byte>::Scanline sl(slw);
	memset(&sl[0], 0, slw);
	for(int y = 0; y < height; ++y)
	{
		layer.getScanline(sl.begin(), hdr.biHeight - y - 1);

		if(!layer.hasPalette())
		{
			// Swap red and blue
			for(int x = 0; x < hdr.biWidth; ++x)
			{
				//std::swap(sl[x * 3], sl[x * 3 + 2]);
				swapIntValues(sl[x * 3], sl[x * 3 + 2]);
			}
		}

		output.write(reinterpret_cast<char *>(const_cast<byte *>(&sl[0])), slw);
	}

	return;
}


void SavePlugin_BMP::Save(const RasterImage &image, std::ostream &output,
						  const Params &params) const
{
	Save(image, output);

	return;
}


InfoPlugin_BMP::InfoPlugin_BMP()
{
}


InfoPlugin_BMP::~InfoPlugin_BMP()
{
}


bool InfoPlugin_BMP::CanInfo(const MimeType &mimeType) const
{
	return (mimeType == MIME_BMP);
}


void InfoPlugin_BMP::Info(const std::string &fileName, int &width, int &height,
						  int &samplesPerPixel, int &bitsPerSample) const
{
	width = height = samplesPerPixel = bitsPerSample = -1;

	std::ifstream input(fileName.c_str(), std::ios::binary);
	if(!input)
		throw ImageBossException(EXC_CANT_LOAD_FILE);


	// Read bitmap header
	BMPHeader hdr;
	input.read(reinterpret_cast<char *>(&hdr), sizeof(BMPHeader));

	width = hdr.biWidth;
	height = hdr.biHeight;
	
	if(hdr.biBitCount >= 8)
	{
		samplesPerPixel = hdr.biBitCount / 8;
		bitsPerSample = 8;
	}
	else
	{
		samplesPerPixel = 1;
		bitsPerSample = hdr.biBitCount;
	}


	input.close();

	return;
}
