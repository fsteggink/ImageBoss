// RasterImage_TIFF.cpp
// FS, 25-01-2009

#define DLLAPI_RASTERIMAGE_TIFF __declspec(dllexport)

#include "RasterImage_TIFF.h"

boost::shared_ptr<LoadPlugin_TIFF> g_LoadPlugin;
//boost::shared_ptr<SavePlugin_TIFF> g_SavePlugin;
boost::shared_ptr<InfoPlugin_TIFF> g_InfoPlugin;
//boost::shared_ptr<LoadByLinePlugin_TIFF> g_LoadByLinePlugin;
//boost::shared_ptr<SaveByLinePlugin_TIFF> g_SaveByLinePlugin;


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


bool DLLAPI_RASTERIMAGE_TIFF SupportsInterface(Interface eIF)
{
	return (g_ceSupportedInterfaces & eIF) != 0;
}


Interface DLLAPI_RASTERIMAGE_TIFF SupportedInterfaces()
{
	return g_ceSupportedInterfaces;
}


boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_TIFF CreatePlugin(Interface eIF)
{
	switch(eIF)
	{
	case Interface::RasterLoad:
		g_LoadPlugin = boost::shared_ptr<LoadPlugin_TIFF>(new LoadPlugin_TIFF());
		return g_LoadPlugin;
		break;

	//case Interface::RasterSave:
	//	g_SavePlugin = boost::shared_ptr<SavePlugin_TIFF>(new SavePlugin_TIFF());
	//	return g_SavePlugin;
	//	break;

	case Interface::RasterInfo:
		g_InfoPlugin = boost::shared_ptr<InfoPlugin_TIFF>(new InfoPlugin_TIFF());
		return g_InfoPlugin;
		break;

	//case Interface::RasterLineLoad:
	//	g_LoadByLinePlugin = boost::shared_ptr<LoadByLinePlugin_TIFF>(new LoadByLinePlugin_TIFF());
	//	return g_LoadByLinePlugin;
	//	break;

	//case Interface::RasterLineSave:
	//	g_SaveByLinePlugin = boost::shared_ptr<SaveByLinePlugin_TIFF>(new SaveByLinePlugin_TIFF());
	//	return g_SaveByLinePlugin;
	//	break;
	}

	return boost::shared_ptr<IImagePlugin>();
}


LoadPlugin_TIFF::LoadPlugin_TIFF()
{
}


LoadPlugin_TIFF::~LoadPlugin_TIFF()
{
}


FileMode LoadPlugin_TIFF::CanLoad(const MimeType &mimeType) const
{
	if(mimeType == MIME_TIFF)
		return FileMode::All;
	else
		return FileMode::None;
}


void LoadPlugin_TIFF::Load(RasterImage &image, const std::string &fileName) const
{
	std::ifstream input(fileName.c_str(), std::ios::binary);
	if(!input)
		throw ImageBossException(EXC_CANT_LOAD_FILE);

	Load(image, input);
	input.close();

	return;
}


void LoadPlugin_TIFF::Load(RasterImage &image, const std::string &fileName,
						   const Params &params) const
{
	Load(image, fileName);
	return;
}


void LoadPlugin_TIFF::Load(RasterImage &image, std::istream &input) const
{
	if(!input)
		throw ImageBossException(EXC_INVALID_INPUTSTREAM);

	boost::shared_ptr<RasterLayer<byte>> layer;


	try
	{
		TIFFHeader header;
		bool bigEndian = OpenTiffFile(input, header);

		input.seekg(header.firstIFD, std::ios::beg);
		IFD ifd;
		ifd.readEntries(input, bigEndian);

		int width = ifd.value(TIFFTag::ImageWidth);
		int height = ifd.value(TIFFTag::ImageLength);
		int samplesPerPixel = ifd.value(TIFFTag::SamplesPerPixel);
		int bitsPerSample = ifd.value(TIFFTag::BitsPerSample);
		bool hasPalette = ifd.hasEntry(TIFFTag::ColorMap);

		layer = boost::shared_ptr<RasterLayer<byte>>(
			new RasterLayer<byte>(width, height, samplesPerPixel));

		// Read palette
		if(hasPalette)
		{
			int iPalCount = ifd.count(TIFFTag::ColorMap) / 3;
			RasterLayer<byte>::Palette palette(iPalCount * 3);
			RasterLayer<unsigned short>::Palette palBuf(iPalCount * 3);
			input.seekg(ifd.offset(TIFFTag::ColorMap));
			input.read(reinterpret_cast<char *>(const_cast<unsigned short *>(&palBuf[0])), iPalCount * 3 * sizeof(unsigned short));

			for(int idx = 0; idx < iPalCount; ++idx)
			{
				for(int k = 0; k < 3; ++k)
				{
					if(bigEndian)
						swapShort(palBuf[idx + iPalCount * k]);
					palette[idx * 3 + k] = (palBuf[idx + iPalCount * k] & 0xFF00) >> 8;
				}
			}

			layer->setPalette(palette);
		}


		// Read data
		boost::shared_ptr<IDataReader> dataReader =
			this->CreateDataReader(
				ifd.value(TIFFTag::Compression),
				ifd.value(TIFFTag::Predictor));
		dataReader->ReadData(layer, input, ifd, bigEndian);


		image.Layers().push_back(layer);
	}
	catch(...)
	{
		throw;
	}

	return;
}


void LoadPlugin_TIFF::Load(RasterImage &image, std::istream &input, const Params &params) const
{
	Load(image, input);
	return;
}


boost::shared_ptr<IDataReader> LoadPlugin_TIFF::CreateDataReader(int compression, int predictor) const
{
	switch(compression)
	{
	case TIFFCompression::None:
		return boost::shared_ptr<IDataReader>(new UncompressedDataReader());
		break;

	case TIFFCompression::LZW:
		return boost::shared_ptr<IDataReader>(new LZWDataReader(predictor));
		break;

	//case TIFFCompression.PackBits:
	//	break;

	default:
		throw ImageBossException("TIFF compression not supported");
	}
}


//SavePlugin_TIFF::SavePlugin_TIFF()
//{
//}
//
//
//SavePlugin_TIFF::~SavePlugin_TIFF()
//{
//}
//
//
//FileMode SavePlugin_TIFF::CanSave(const MimeType &mimeType) const
//{
//	if(mimeType == MIME_TIFF)
//		return FileMode::All;
//	else
//		return FileMode::None;
//}
//
//
//void SavePlugin_TIFF::Save(const RasterImage &image, const std::string &fileName) const
//{
//	Save(image, fileName, Params());
//
//	return;
//}
//
//
//void SavePlugin_TIFF::Save(const RasterImage &image, const std::string &fileName,
//						   const Params &params) const
//{
//	std::ofstream output(fileName.c_str(), std::ios::binary);
//	if(!output)
//		throw ImageBossException(EXC_CANT_SAVE_FILE);
//
//	Save(image, output, params);
//	output.close();
//
//	return;
//}
//
//
//void SavePlugin_TIFF::Save(const RasterImage &image, std::ostream &output) const
//{
//	Save(image, output, Params());
//
//	return;
//}
//
//
//void SavePlugin_TIFF::Save(const RasterImage &image, std::ostream &output,
//						   const Params &params) const
//{
//	if(!output)
//		throw ImageBossException(EXC_INVALID_OUTPUTSTREAM);
//
//	boost::shared_ptr<IRasterLayer> ifLayer = image.Layers()[0];
//
//	RasterLayer<byte> &layer =
//		*boost::dynamic_pointer_cast<RasterLayer<byte>, IRasterLayer>(ifLayer);
//
//	int width = layer.get_Width();
//	int height = layer.get_Height();
//	int spp = layer.get_SamplesPerPixel();
//	int slw = width * spp;
//
//	try
//	{
//	}
//	catch(...)
//	{
//		// Ignore
//	}
//
//	return;
//}


InfoPlugin_TIFF::InfoPlugin_TIFF()
{
}


InfoPlugin_TIFF::~InfoPlugin_TIFF()
{
}


bool InfoPlugin_TIFF::CanInfo(const MimeType &mimeType) const
{
	return (mimeType == MIME_TIFF);
}


void InfoPlugin_TIFF::Info(const std::string &fileName, int &width, int &height,
						   int &samplesPerPixel, int &bitsPerSample) const
{
	width = height = samplesPerPixel = bitsPerSample = -1;

	std::ifstream input(fileName.c_str(), std::ios::binary);
	if(!input)
		throw ImageBossException(EXC_CANT_LOAD_FILE);


	// Read file header
	try
	{
		TIFFHeader header;
		bool bigEndian = OpenTiffFile(input, header);

		input.seekg(header.firstIFD, std::ios::beg);
		IFD ifd;
		ifd.readEntries(input, bigEndian);

		//for(std::vector<IFDEntry>::const_iterator iterEntry = ifd.entries().begin();
		//	iterEntry != ifd.entries().end(); ++iterEntry)
		//{
		//	std::cout << "Entry.tag = " << iterEntry->tag << 
		//		", .fieldType = " << iterEntry->fieldType <<
		//		", .count = " << iterEntry->count <<
		//		", .offset = " << iterEntry->offset << std::endl;
		//}

		width = ifd.value(TIFFTag::ImageWidth);
		height = ifd.value(TIFFTag::ImageLength);
		samplesPerPixel = ifd.value(TIFFTag::SamplesPerPixel);
		bitsPerSample = ifd.value(TIFFTag::BitsPerSample);
	}
	catch(const char *exc)
	{
		std::cout << "Exception in file " << fileName << ": " << exc << std::endl;
	}
	catch(...)
	{
		std::cout << "Exception in file " << fileName << std::endl;
	}
	input.close();

	return;
}


//LoadByLinePlugin_TIFF::LoadByLinePlugin_TIFF()
//{
//}
//
//
//LoadByLinePlugin_TIFF::~LoadByLinePlugin_TIFF()
//{
//}
//
//
//FileMode LoadByLinePlugin_TIFF::CanLoad(const MimeType &mimeType)
//{
//	if(mimeType == MIME_TIFF)
//		return FileMode::All;
//	else
//		return FileMode::None;
//}
//
//
//IContext *LoadByLinePlugin_TIFF::Open(const std::string &fileName)
//{
//	m_fIn.open(fileName.c_str(), std::ios::binary);
//	if(!m_fIn)
//		throw ImageBossException(EXC_CANT_LOAD_FILE);
//
//	IContext *context = Open(m_fIn);
//	dynamic_cast<LoadByLineContext *>(context)->isFile = true;
//
//	return context;
//}
//
//
//IContext *LoadByLinePlugin_TIFF::Open(std::istream &input)
//{
//	if(!input)
//		throw ImageBossException(EXC_INVALID_INPUTSTREAM);
//
//	LoadByLineContext *oContext = new LoadByLineContext(input);
//
//	try
//	{
//	}
//	catch(...)
//	{
//		// Ignore
//	}
//
//	return oContext;
//}
//
//
//void LoadByLinePlugin_TIFF::ReadScanline(IContext *context, RasterLayer<byte>::Scanline &scanline)
//{
//	if(!context)
//		return;
//
//	LoadByLineContext *oContext = dynamic_cast<LoadByLineContext *>(context);
//
//	return;
//}
//
//
//void LoadByLinePlugin_TIFF::Close(IContext *&context)
//{
//	if(!context)
//		return;
//
//	LoadByLineContext *oContext = dynamic_cast<LoadByLineContext *>(context);
//
//	delete context;
//	context = 0;
//
//	if(m_fIn)
//	{
//		m_fIn.close();
//	}
//
//	return;
//}
//
//
//int LoadByLinePlugin_TIFF::GetWidth(IContext *context) const
//{
//	//return dynamic_cast<LoadByLineContext *>(context)->cinfo.image_width;
//	return 0;
//}
//
//
//int LoadByLinePlugin_TIFF::GetHeight(IContext *context) const
//{
//	//return dynamic_cast<LoadByLineContext *>(context)->cinfo.image_height;
//	return 0;
//}
//
//
//int LoadByLinePlugin_TIFF::GetSamplesPerPixel(IContext *context) const
//{
//	//return dynamic_cast<LoadByLineContext *>(context)->cinfo.output_components;
//	return 8;
//}
//
//
//SaveByLinePlugin_TIFF::SaveByLinePlugin_TIFF()
//{
//}
//
//
//SaveByLinePlugin_TIFF::~SaveByLinePlugin_TIFF()
//{
//}
//
//
//FileMode SaveByLinePlugin_TIFF::CanSave(const MimeType &mimeType)
//{
//	if(mimeType == MIME_TIFF)
//		return FileMode::All;
//	else
//		return FileMode::None;
//}
//
//
//IContext *SaveByLinePlugin_TIFF::Open(const std::string &fileName, int width, int height, int spp)
//{
//	m_fOut.open(fileName.c_str(), std::ios::binary);
//	if(!m_fOut)
//		throw ImageBossException(EXC_CANT_SAVE_FILE);
//
//	// NOTE: Open(filename) does not work well! Probably because context is copied, and that in
//	// combination with a reference to the stream, might cause problems.
//	IContext *context = Open(m_fOut, width, height, spp);
//	dynamic_cast<SaveByLineContext *>(context)->isFile = true;
//
//	return context;
//}
//
//
//IContext *SaveByLinePlugin_TIFF::Open(std::ostream &output, int width, int height, int spp)
//{
//	if(!output)
//		throw ImageBossException(EXC_INVALID_OUTPUTSTREAM);
//
//	SaveByLineContext *oContext = new SaveByLineContext(output);
//
//	try
//	{
//	}
//	catch(...)
//	{
//		// Ignore
//	}
//
//
//	return oContext;
//}
//
//
//void SaveByLinePlugin_TIFF::WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanline)
//{
//	if(!context)
//		return;
//
//	SaveByLineContext *oContext = dynamic_cast<SaveByLineContext *>(context);
//
//	return;
//}
//
//
//void SaveByLinePlugin_TIFF::Close(IContext *&context)
//{
//	if(!context)
//		return;
//
//	SaveByLineContext *oContext = dynamic_cast<SaveByLineContext *>(context);
//
//	delete context;
//	context = 0;
//
//	if(m_fOut)
//	{
//		m_fOut.close();
//	}
//}


bool OpenTiffFile(std::istream &input, TIFFHeader &header)
{
	bool bigEndian = false;
	input.read(reinterpret_cast<char *>(&header), sizeof(TIFFHeader));

	switch(header.endianness)
	{
	case 0x4949:
		bigEndian = false;
		break;

	case 0x4D4D:
		bigEndian = true;
		break;

	default:
		throw ImageBossException("Invalid endianness specified in file");
	}

	if(bigEndian)
	{
		swapShort(header.check);
		swapInt(header.firstIFD);
	}

	if(header.check != 0x2A)
		throw ImageBossException("Invalid TIFF identifier");

	return bigEndian;
}


void IDataReader::ReadData(boost::shared_ptr<RasterLayer<byte>> &layer, std::istream &input, const IFD &ifd, bool bigEndian)
{
	unsigned int dummy = 0;

	// Read offsets and byte counts
	unsigned int rowsPerStrip = ifd.value(TIFFTag::RowsPerStrip);

	std::vector<unsigned int> stripOffsets, stripByteCounts;
	IFDEntry entryStripOffsets = ifd.entry(TIFFTag::StripOffsets);
	IFDEntry entryStripByteCounts = ifd.entry(TIFFTag::StripByteCounts);

	stripOffsets = readIFDValues(entryStripOffsets, input, bigEndian, dummy);
	stripByteCounts = readIFDValues(entryStripByteCounts, input, bigEndian, dummy);

	// Read data and set it into the layer
	unsigned int maxByteCount = 0;
	for(std::vector<unsigned int>::const_iterator iterByteCounts = stripByteCounts.begin();
		iterByteCounts != stripByteCounts.end(); ++iterByteCounts)
	{
		if(*iterByteCounts > maxByteCount)
			maxByteCount = *iterByteCounts;
	}
	boost::shared_array<char> buf(new char [maxByteCount]);
	
	for(unsigned int idx = 0; idx < entryStripOffsets.count; ++idx)
	{
		// Get data
		input.seekg(stripOffsets[idx], std::ios::beg);
		this->ReadBytes(layer, input, buf.get(), stripByteCounts[idx], rowsPerStrip * idx);
	}

	return;
}


void UncompressedDataReader::ReadBytes(boost::shared_ptr<RasterLayer<byte>> &strip, std::istream &input, char *buf, unsigned int byteCounts, int y0)
{
	// Todo: size check
	input.read(reinterpret_cast<char *>(const_cast<byte *>(strip->data(0, y0))), byteCounts);

	return;
}


void LZWDataReader::ReadBytes(boost::shared_ptr<RasterLayer<byte>> &strip, std::istream &input, char *buf, unsigned int byteCounts, int y0)
{
	// Todo: size check
	int width = strip->get_Width();
	int height = strip->get_Height();
	int spp = strip->get_SamplesPerPixel();

	/*** *** Begin LZW Compression *** ***/
	// Copied from LoadPlugin_GIF::Load

	// Build initial GIF directory
	std::vector<std::string> gifDic;
	//int initCodeSize = (int)input.get();
	int initCodeSize = 8;
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
	static RasterLayer<byte>::Scanline sl(width * spp * 2);   // Need extra space
	memset(&sl[0], 0, sizeof(width * spp * 2));
	bool exit = false;



	// Read buffer
	int bitPos = 0;
	input.read(buf, byteCounts);
	int iDicSize = static_cast<int>(gifDic.size());

	//if((int)input.tellg() == 40943)
	//	std::cout << "clearCode: " << clearCode << ", endOfInfoCode: " << endOfInfoCode << ", codeSize: " << codeSize << std::endl;

	while(!exit)
	{
		// Note that the value needs to be swapped, since the pixels are stored byte by byte.
		// This is not necessary in the GIF parser.
		int move = 32 - bitPos % 8 - codeSize;
		int iValue = (swapIntValue(*reinterpret_cast<int*>(&buf[bitPos / 8])) >> move) & mask;
		bitPos += codeSize;

		//if((int)input.tellg() == 40943)
		//	std::cout << "value: " << iValue << ", dic size: " << iDicSize << ", bit pos: " << bitPos << std::endl;
		if(bitPos > (int)byteCounts * 8)
		{
			// Trying to read value beyond the buffer, without encountering an EndOfInfo code
			exit = true;
			break;
		}

		if(iValue == clearCode)
		{
			// ClearCode will be encountered automatically when the dictionary grows too large.
			initDic(initCodeSize, gifDic);
			codeSize = initCodeSize + 1;
			iDicSize = static_cast<int>(gifDic.size());

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
			++x;

			gifDic.back() += prev;

			// Add new value to dictionary
			gifDic.push_back(std::string(1, prev));
			++iDicSize;
		}
		//else if(iValue < static_cast<int>(gifDic.size()))
		else if(iValue < iDicSize)
		{
			// Existing string needed
			std::vector<std::string>::iterator entry = gifDic.begin() + iValue;
			gifDic.back() += (*entry)[0];

			int len = static_cast<int>(entry->length());
			memcpy(&sl[x], &(*entry)[0], len);
			x += len;
			
			// Add new value to dictionary
			gifDic.push_back(*entry);
			++iDicSize;
		}
		else
		{
			// Value too big
			std::cerr << "Value: " << iValue << ", pos: " << input.tellg() << ", buf size: " << byteCounts << ", dic size: " << iDicSize << std::endl;
			throw "Invalid GIF code encountered in data";
		}


		// NB: for GIF files of a single color, it is possible that more than one scanline
		// can be set from a single entry.
		while(x >= width * spp)
		{
			// Store scanline
			if(y + y0 < height)
			{
				if(m_predictor == TIFFPredictor::HorizontalDifferencing)
				{
					// Apply horizontal differencing
					for(int i = spp; i < width * spp; ++i)
					{
						sl[i] = static_cast<byte>(sl[i] + sl[i - spp]);
					}
				}

				strip->setScanline(sl.begin(), y + y0);
			}

			memcpy(&sl[0], &sl[0] + width * spp, std::min(static_cast<int>(sl.size() - width * spp), width * spp));
			x -= width * spp;
			++y;
		}

		int iDicSize = static_cast<int>(gifDic.size());
		//int testMask = (1 << initCodeSize);
		//for(int i = initCodeSize; i < 12; ++i, testMask <<= 1)
		for(int testMask = (1 << (initCodeSize + 1)); testMask <= iDicSize; testMask <<= 1)
		{
			//showVal(testMask);
			//if(iDicSize == (1 << i))   // NB: in the GIF parser this is if(iDicSize == (1 << i) + 1)
			if(iDicSize == testMask)
			{
				codeSize++;
				mask = mask << 1 | 1;
			}
		}
	}

	/*** *** End LZW Compression *** ***/

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
