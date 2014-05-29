// RasterImage_GIF.cpp
// FS, 29-06-2007

#define DLLAPI_RASTERIMAGE_GIF __declspec(dllexport)

#include "RasterImage_GIF.h"

boost::shared_ptr<LoadPlugin_GIF> g_LoadPlugin;
//boost::shared_ptr<SavePlugin_GIF> g_SavePlugin;
boost::shared_ptr<InfoPlugin_GIF> g_InfoPlugin;


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


bool DLLAPI_RASTERIMAGE_GIF SupportsInterface(Interface eIF)
{
	return (g_ceSupportedInterfaces & eIF) != 0;
}


Interface DLLAPI_RASTERIMAGE_GIF SupportedInterfaces()
{
	return g_ceSupportedInterfaces;
}


boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_GIF CreatePlugin(Interface eIF)
{
	switch(eIF)
	{
	case Interface::RasterLoad:
		g_LoadPlugin = boost::shared_ptr<LoadPlugin_GIF>(new LoadPlugin_GIF());
		return g_LoadPlugin;
		break;

	//case Interface::RasterSave:
	//	g_SavePlugin = boost::shared_ptr<SavePlugin_GIF>(new SavePlugin_GIF());
	//	return g_SavePlugin;
	//	break;

	case Interface::RasterInfo:
		g_InfoPlugin = boost::shared_ptr<InfoPlugin_GIF>(new InfoPlugin_GIF());
		return g_InfoPlugin;
		break;
	}

	return boost::shared_ptr<IImagePlugin>();
}


FileMode LoadPlugin_GIF::CanLoad(const MimeType &mimeType) const
{
	if(mimeType == MIME_GIF)
		return FileMode::All;
	else
		return FileMode::None;
}


void LoadPlugin_GIF::Load(RasterImage &image, std::istream &input, const Params &params) const
{
	if(!input)
		throw ImageBossException("GIF: " + EXC_INVALID_INPUTSTREAM);

	RasterLayer<byte> *layer = 0;
	char *buf = 0;

	try
	{
		// Read file header
		GIFHeader hdr;
		input.read(reinterpret_cast<char *>(&hdr), sizeof(GIFHeader));

		GIFLogicalScreenDescriptor lsd;
		input.read(reinterpret_cast<char *>(&lsd), sizeof(GIFLogicalScreenDescriptor));

		int width = lsd.Width;
		int height = lsd.Height;

		boost::shared_ptr<RasterLayer<byte>> layer(new RasterLayer<byte>(width, height, 1));

		bool bGlobalColorTableFlag = ((lsd.PackedFields & 0x80) != 0);
		int iColorResolution = 1 << (((lsd.PackedFields & 0x70) >> 4) + 1);
		bool bSortFlag = ((lsd.PackedFields & 0x08) != 0);
		int iGlobalColorTableSize = 1 << ((lsd.PackedFields & 0x07) + 1);

		// Read palette
		if(bGlobalColorTableFlag)
		{
			int iPalCount = iGlobalColorTableSize;
			RasterLayer<byte>::Palette palette(iPalCount * 3);
			input.read(reinterpret_cast<char *>(const_cast<byte *>(&palette[0])), iPalCount * 3);
			layer->setPalette(palette);
		}


		// Read extensions
		ReadExtensions(input);


		// Read image data
		GIFImageDescriptor id;
		input.read(reinterpret_cast<char *>(&id), sizeof(GIFImageDescriptor));

		if(id.Separator != 0x2C)
			throw ImageBossException("GIF: Unexpected byte at image descriptor");

		bool bLocalColorTableFlag = ((id.PackedFields & 0x80) != 0);
		bool bInterlaceFlag = ((id.PackedFields & 0x40) != 0);
		bool bLocalSortFlag = ((id.PackedFields & 0x20) != 0);
		int iLocalColorTableSize = 1 << ((id.PackedFields & 0x07) + 1);

		if(bInterlaceFlag)
			throw ImageBossException("GIF: GIF interlacing not supported");

		// Skip local palette
		if(bLocalColorTableFlag)
		{
			input.seekg(iLocalColorTableSize, std::ios::cur);
		}


		/*** *** Begin LZW Decompression *** ***/

		// Build initial GIF directory
		std::vector<std::string> gifDic;
		int initCodeSize = (int)input.get();
		int codeSize = initCodeSize + 1;
		int clearCode = (1 << initCodeSize);
		int endOfInfoCode = clearCode + 1;
		const int maxCodeValue = 4095;
		const char trailer = 0x3B;

		gifDic.reserve(maxCodeValue);
		initDic(initCodeSize, gifDic);

		// Create mask of codeSize set bytes
		int mask = 0;
		for(int i = 0; i < codeSize; ++i)
		{
			mask = mask << 1 | 1;
		}

		int x = 0, y = 0;
		RasterLayer<byte>::Scanline sl(width * 3);   // Need extra space
		bool exit = false;


		int bitPos = 16;
		int iBitBufSize;
		buf = new char[257];   // maximum size = 255 + 2 (for remains)

		unsigned char blockSize = input.get();
		while(blockSize != 0 && !exit)
		{
			// Read new buffer
			input.read(&buf[2], blockSize);
			iBitBufSize = (blockSize + 2) * 8;

			while(bitPos + codeSize < iBitBufSize)
			{
				int iValue = (*reinterpret_cast<int*>(&buf[bitPos / 8]) >> bitPos % 8) & mask;
				bitPos += codeSize;

				if(iValue == clearCode)
				{
					// ClearCode will be encountered automatically when the dictionary grows too
					// large
					initDic(initCodeSize, gifDic);
					codeSize = initCodeSize + 1;

					// Reset mask
					mask = 0;
					for(int i = 0; i < codeSize; ++i)
					{
						mask = mask << 1 | 1;
					}
				}
				else if(iValue == endOfInfoCode)
				{
					exit = true;
					break;
				}
				else if(iValue < clearCode)
				{
					// New character
					unsigned char prev = iValue & 0xFF;
					sl[x] = prev;
					x++;

					gifDic.back() += prev;

					// Add new value to dictionary
					gifDic.push_back(std::string(1, prev));
				}
				else if(iValue < static_cast<int>(gifDic.size()))
				{
					// Existing string needed
					std::vector<std::string>::iterator entry = gifDic.begin() + iValue;
					gifDic.back() += (*entry)[0];

					memcpy(&sl[x], &(*entry)[0], entry->length());
					x += static_cast<int>(entry->length());
					
					// Add new value to dictionary
					gifDic.push_back(*entry);
				}
				else
				{
					// Value too big
					throw ImageBossException("GIF: Invalid GIF code encountered in data");
				}


				// NB: for GIF files of a single color, it is possible that more than one scanline
				// can be set from a single entry.
				while(x >= width)
				{
					// Store scanline
					if(y < height)
						layer->setScanline(sl.begin(), y);
					memcpy(&sl[0], &sl[width], width);
					x -= width;
					y++;
				}

				int iDicSize = static_cast<int>(gifDic.size());
				for(int i = initCodeSize; i < 12; ++i)
				{
					if(iDicSize == (1 << i) + 1)
					{
						codeSize++;
						mask = mask << 1 | 1;
					}
				}
			}

			// Prepare for next round
			buf[0] = buf[blockSize];
			buf[1] = buf[blockSize + 1];
			bitPos -= blockSize * 8;

			blockSize = input.get();
		}

		delete[] buf;
		buf = 0;

		/*** *** End LZW Compression *** ***/


		image.Layers().push_back(layer);
	}
	catch(std::bad_alloc)
	{
		if(layer != 0) delete layer;
		if(buf != 0) delete[] buf;
		throw ImageBossException("GIF: Memory exhausted!");
	}
	catch(...)
	{
		if(layer != 0) delete layer;
		if(buf != 0) delete[] buf;
		throw ImageBossException("GIF: Unknown error");
	}

	return;
}


void LoadPlugin_GIF::ReadExtensions(std::istream &input) const
{
	while(input.peek() == 0x21)
	{
		unsigned char n1, n2;
		input.read(reinterpret_cast<char *>(&n1), 1);
		input.read(reinterpret_cast<char *>(&n2), 1);

		input.seekg(-2, std::ios::cur);

		switch(n2)
		{
		case 0xF9:   // Graphical control extension
			// Read the GIFGraphicalControlExtension, but ignore it
			GIFGraphicalControlExtension gce;
			input.read(reinterpret_cast<char *>(&gce), sizeof(GIFGraphicalControlExtension));
			break;

		case 0xFF:   // Application extension
			// Read the GIFApplicationExtension, but ignore it
			GIFApplicationExtension ae;
			input.read(reinterpret_cast<char *>(&ae), sizeof(GIFApplicationExtension));
			while(input.peek() != 0)
			{
				// Advance the read pointer as long as there is more user data
				input.seekg(static_cast<unsigned char>(input.peek()) + 1, std::ios::cur);
			}

			// Skip the block terminator
			input.seekg(1, std::ios::cur);

			break;

		default:
			throw ImageBossException("GIF: Unrecognized GIF extension");
			break;
		}
	}

	return;
}


bool InfoPlugin_GIF::CanInfo(const MimeType &mimeType) const
{
	return (mimeType == MIME_GIF);
}


void InfoPlugin_GIF::Info(const std::string &fileName, int &width, int &height,
						  int &samplesPerPixel, int &bitsPerSample) const
{
	width = height = samplesPerPixel = bitsPerSample = -1;

	std::ifstream input(fileName.c_str(), std::ios::binary);
	if(!input)
		throw ImageBossException(EXC_CANT_LOAD_FILE);


	// Read bitmap header
	GIFHeader hdr;
	input.read(reinterpret_cast<char *>(&hdr), sizeof(GIFHeader));

	GIFLogicalScreenDescriptor lsd;
	input.read(reinterpret_cast<char *>(&lsd), sizeof(GIFLogicalScreenDescriptor));

	width = lsd.Width;
	height = lsd.Height;

	samplesPerPixel = 1;
	bitsPerSample = 8;
	
	input.close();

	return;
}


void initDic(int initCodeSize, std::vector<std::string> &gifDic)
{
	//std::cout << "Init dictionary" << std::endl;
	//gifDic.clear();

	if(gifDic.size() == 0)
	{
		for(int i = 0; i < (1 << initCodeSize); ++i)
		{
			std::string sTemp = std::string(1, (char)i);
			gifDic.push_back(sTemp);
		}
		
		gifDic.push_back("");   // Skip clear code
		gifDic.push_back("");   // Skip end of info code
	}
	else
	{
		gifDic.erase(gifDic.begin() + (1 << initCodeSize) + 2, gifDic.end());
	}
	
	return;
}
