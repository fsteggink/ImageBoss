// RasterImage_PNG.cpp
// FS, 25-01-2009

#define DLLAPI_RASTERIMAGE_PNG __declspec(dllexport)

#include "RasterImage_PNG.h"

boost::shared_ptr<LoadPlugin_PNG> g_LoadPlugin;
boost::shared_ptr<SavePlugin_PNG> g_SavePlugin;
boost::shared_ptr<InfoPlugin_PNG> g_InfoPlugin;
boost::shared_ptr<LoadByLinePlugin_PNG> g_LoadByLinePlugin;
boost::shared_ptr<SaveByLinePlugin_PNG> g_SaveByLinePlugin;

/* Table of CRCs of all 8-bit messages. */
unsigned int crc_table[256];
/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;



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


bool DLLAPI_RASTERIMAGE_PNG SupportsInterface(Interface eIF)
{
	return (g_ceSupportedInterfaces & eIF) != 0;
}


Interface DLLAPI_RASTERIMAGE_PNG SupportedInterfaces()
{
	return g_ceSupportedInterfaces;
}


boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_PNG CreatePlugin(Interface eIF)
{
	switch(eIF)
	{
	case Interface::RasterLoad:
		g_LoadPlugin = boost::shared_ptr<LoadPlugin_PNG>(new LoadPlugin_PNG());
		return g_LoadPlugin;
		break;

	case Interface::RasterSave:
		g_SavePlugin = boost::shared_ptr<SavePlugin_PNG>(new SavePlugin_PNG());
		return g_SavePlugin;
		break;

	case Interface::RasterInfo:
		g_InfoPlugin = boost::shared_ptr<InfoPlugin_PNG>(new InfoPlugin_PNG());
		return g_InfoPlugin;
		break;

	case Interface::RasterLineLoad:
		g_LoadByLinePlugin = boost::shared_ptr<LoadByLinePlugin_PNG>(new LoadByLinePlugin_PNG());
		return g_LoadByLinePlugin;
		break;

	case Interface::RasterLineSave:
		g_SaveByLinePlugin = boost::shared_ptr<SaveByLinePlugin_PNG>(new SaveByLinePlugin_PNG());
		return g_SaveByLinePlugin;
		break;
	}

	return boost::shared_ptr<IImagePlugin>();
}


FileMode LoadPlugin_PNG::CanLoad(const MimeType &mimeType) const
{
	if(mimeType == MIME_PNG)
		return FileMode::All;
	else
		return FileMode::None;
}


boost::shared_ptr<IRasterLoadByLinePlugin> LoadPlugin_PNG::GetLoadByLinePlugin() const
{
	return boost::shared_ptr<IRasterLoadByLinePlugin>(new LoadByLinePlugin_PNG());
}


FileMode SavePlugin_PNG::CanSave(const MimeType &mimeType) const
{
	if(mimeType == MIME_PNG)
		return FileMode::All;
	else
		return FileMode::None;
}


boost::shared_ptr<IRasterSaveByLinePlugin> SavePlugin_PNG::GetSaveByLinePlugin() const
{
	return boost::shared_ptr<IRasterSaveByLinePlugin>(new SaveByLinePlugin_PNG());
}


bool InfoPlugin_PNG::CanInfo(const MimeType &mimeType) const
{
	return (mimeType == MIME_PNG);
}


void InfoPlugin_PNG::Info(const std::string &fileName, int &width, int &height,
						   int &samplesPerPixel, int &bitsPerSample) const
{
	width = height = samplesPerPixel = bitsPerSample = -1;

	std::ifstream input(fileName.c_str(), std::ios::binary);
	if(!input)
		throw ImageBossException(EXC_CANT_LOAD_FILE);


	// Read file header
	try
	{
		char hdr[sizeof(PNG_SIGNATURE)];
		input.read(reinterpret_cast<char *>(&hdr), sizeof(PNG_SIGNATURE));

		if(memcmp(hdr, PNG_SIGNATURE, sizeof(PNG_SIGNATURE)) != 0)
			throw "Invalid PNG header";

		PNGChunk chunk = PNGChunk::read(input);
		if(chunk.length != 13)
			throw "Length of IHDR chunk is not 13";
		if(chunk.chunk != PNGChunk::IHDR)
			throw "First PNG chunk is not IHDR";

		IHDR_Data ihdr_data;
		chunk.copyData(&ihdr_data);
		
		width = swapIntValue(ihdr_data.width);
		height = swapIntValue(ihdr_data.height);
		bitsPerSample = ihdr_data.bitDepth;
		samplesPerPixel = PNGColorType::samplesPerPixel(ihdr_data.colorType);
		if(samplesPerPixel == 0)
			throw "Unrecognized color type";
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


FileMode LoadByLinePlugin_PNG::CanLoad(const MimeType &mimeType) const
{
	if(mimeType == MIME_PNG)
		return FileMode::All;
	else
		return FileMode::None;
}


IContext *LoadByLinePlugin_PNG::Open(const std::string &fileName) const
{
	m_fIn.open(fileName.c_str(), std::ios::binary);
	if(!m_fIn)
		throw ImageBossException(EXC_CANT_LOAD_FILE);

	IContext *context = Open(m_fIn);
	dynamic_cast<LoadByLineContext *>(context)->isFile = true;

	return context;
}


IContext *LoadByLinePlugin_PNG::Open(std::istream &input) const
{
	if(!input)
		throw ImageBossException(EXC_INVALID_INPUTSTREAM);

	LoadByLineContext *oContext = new LoadByLineContext(input);

	try
	{
		// Read file header
		char hdr[sizeof(PNG_SIGNATURE)];
		input.read(reinterpret_cast<char *>(&hdr), sizeof(PNG_SIGNATURE));

		if(memcmp(hdr, PNG_SIGNATURE, sizeof(PNG_SIGNATURE)) != 0)
			throw "Invalid PNG header";

		PNGChunk chunk = PNGChunk::read(input);
		if(chunk.length != 13)
			throw "Length of IHDR chunk is not 13";
		if(chunk.chunk != PNGChunk::IHDR)
			throw "First PNG chunk is not IHDR";

		chunk.copyData(&oContext->header);
		
		// Parse header values
		swapInt(oContext->header.width);
		swapInt(oContext->header.height);
		unsigned int width = oContext->header.width;
		unsigned int height = oContext->header.height;
		unsigned int bitsPerSample = oContext->header.bitDepth;
		unsigned int samplesPerPixel = PNGColorType::samplesPerPixel(oContext->header.colorType);
		if(samplesPerPixel == 0)
			throw "Unrecognized PNG color type";

		// Check compression method
		if(oContext->header.compression != PNGCompressionMethod::Deflate)
			throw "Unrecognized PNG compression method";

		if(oContext->header.filter != PNGFilterMethod::Adaptive)
			throw "Unrecognized PNG filter method";

		// Check interlace method
		switch(oContext->header.interlace)
		{
		case PNGInterlaceMethod::None:
			break;

		case PNGInterlaceMethod::Adam7:
			throw "PNG interlace method Adam7 is not supported";

		default:
			throw "Unrecognized PNG interlace method";
		}
	}
	catch(...)
	{
		// Ignore
	}

	// TIJDELIJK: lees data in
	// Aanpassen in on demand inlezen en inflaten

	boost::shared_array<byte> data;
	unsigned int dataLen = 0;

	// Read chunks
	PNGChunk chunk = PNGChunk::read(oContext->input);
	while(chunk.chunk != PNGChunk::IEND)
	{
		switch(chunk.chunk)
		{
		case PNGChunk::PLTE:
			readPalette(oContext->palette, chunk);
			break;

		case PNGChunk::IDAT:
			boost::shared_array<byte> newData(new unsigned char[dataLen + chunk.length]);
			memcpy(newData.get(), data.get(), dataLen);
			memcpy(newData.get() + dataLen, chunk.data.get(), chunk.length);
			data.swap(newData);
			dataLen += chunk.length;

			break;
		}

		chunk = PNGChunk::read(oContext->input);
	}

	// Read data
	int spp = PNGColorType::samplesPerPixel(oContext->header.colorType);
	int slw = oContext->header.width * spp;

	// Decompress (inflate) data
	oContext->strm.zalloc = (alloc_func)0;
	oContext->strm.zfree  = (free_func)0;
	oContext->strm.opaque = (voidpf)0;
	inflateInit(&oContext->strm);

	oContext->strm.next_in = data.get();
	oContext->strm.avail_in = dataLen;

	int uncomprLen = (slw + 1) * oContext->header.height;
	oContext->data = boost::shared_array<byte>(new byte[uncomprLen]);

	if(oContext->strm.total_in == 0)
	{
		oContext->strm.next_out = oContext->data.get();
		oContext->strm.avail_out = uncomprLen;
	}

	int oldTotalIn = oContext->strm.total_in;
	while (oContext->strm.total_in - oldTotalIn < dataLen)
	{
		int retVal = ::inflate(&oContext->strm, Z_NO_FLUSH);
		if(retVal == Z_STREAM_END) break;
	}

	inflateEnd(&oContext->strm);

	// Unfilter data
	oContext->scanlinePrev = RasterLayer<byte>::Scanline(slw);
	oContext->y = 0;

	return oContext;
}


void LoadByLinePlugin_PNG::ReadScanline(IContext *context, RasterLayer<byte>::Scanline &scanline) const
{
	if(!context)
		return;

	LoadByLineContext *oContext = dynamic_cast<LoadByLineContext *>(context);

	// Unfilter the next scanline
	int slw = static_cast<int>(scanline.size());
	int spp = PNGColorType::samplesPerPixel(oContext->header.colorType);
	unsigned int sppa = spp + (PNGColorType::hasAlpha(oContext->header.colorType)) ? 1 : 0;
	int bps = oContext->header.bitDepth;

	int idx = oContext->y * (slw + 1);
	memcpy(&scanline[0], &oContext->data[idx + 1], slw);
	filterFunc filter = getFilter(oContext->data[idx]);
	filterScanline(filter, 1, &scanline[0], 0, &oContext->scanlinePrev[0], oContext->header.width * sppa, 
		sppa, bps);

	/*if(btBPS < 8)
	{
		memcpy(abtScanlineTemp, abtScanline, iUncomprLen);
		expandScanline(abtScanline + 1, abtScanlineTemp + 1, oLayer->getWidth(), btBPS);
	}*/

	std::copy(scanline.begin(), scanline.end(), oContext->scanlinePrev.begin());

	oContext->y++;
}


void LoadByLinePlugin_PNG::Close(IContext *&context) const
{
	if(!context)
		return;

	LoadByLineContext *oContext = dynamic_cast<LoadByLineContext *>(context);

	oContext->data.reset();

	delete context;
	context = 0;

	if(m_fIn)
		m_fIn.close();

	return;
}


int LoadByLinePlugin_PNG::GetWidth(IContext *context) const
{
	return dynamic_cast<LoadByLineContext *>(context)->header.width;
}


int LoadByLinePlugin_PNG::GetHeight(IContext *context) const
{
	return dynamic_cast<LoadByLineContext *>(context)->header.height;
}


int LoadByLinePlugin_PNG::GetSamplesPerPixel(IContext *context) const
{
	return PNGColorType::samplesPerPixel(dynamic_cast<LoadByLineContext *>(context)->header.colorType);
}


bool LoadByLinePlugin_PNG::HasPalette(IContext *context) const
{
	return dynamic_cast<LoadByLineContext *>(context)->palette.size() != 0;
}


const RasterLayer<byte>::Palette &LoadByLinePlugin_PNG::GetPalette(IContext *context) const
{
	return dynamic_cast<LoadByLineContext *>(context)->palette;
}


void LoadByLinePlugin_PNG::readPalette(RasterLayer<byte>::Palette &palette, const PNGChunk &chunk) const
{
	palette.resize(chunk.length);
	chunk.copyData(&palette[0]);

	return;
}


FileMode SaveByLinePlugin_PNG::CanSave(const MimeType &mimeType) const
{
	if(mimeType == MIME_PNG)
		return FileMode::All;
	else
		return FileMode::None;
}


IContext *SaveByLinePlugin_PNG::Open(const std::string &fileName, int width, int height, int spp) const
{
	m_fOut.open(fileName.c_str(), std::ios::binary);
	if(!m_fOut)
		throw ImageBossException(EXC_CANT_SAVE_FILE);

	// NOTE: Open(filename) does not work well! Probably because context is copied, and that in
	// combination with a reference to the stream, might cause problems.
	IContext *context = Open(m_fOut, width, height, spp);
	dynamic_cast<SaveByLineContext *>(context)->isFile = true;

	return context;
}


IContext *SaveByLinePlugin_PNG::Open(std::ostream &output, int width, int height, int spp) const
{
	if(!output)
		throw ImageBossException(EXC_INVALID_OUTPUTSTREAM);

	SaveByLineContext *oContext = new SaveByLineContext(output);

	try
	{
		if(width <= 0 || height <= 0)
			throw ImageBossException(EXC_INVALID_DIMENSIONS);
		if(width > MAX_PNG_SIZE || height > MAX_PNG_SIZE)
			throw ImageBossException(EXC_INVALID_DIMENSIONS);
		int slw = width * spp;

		// Write signature
		oContext->output.write(PNG_SIGNATURE, sizeof(PNG_SIGNATURE));

		// Write header
		//IHDR_Data hdrData;
		oContext->header.width = swapIntValue(width);
		oContext->header.height = swapIntValue(height);
		//oContext->header.bitDepth = layer.get_BitsPerSample();
		oContext->header.bitDepth = 8;
		//oContext->header.colorType = PNGColorType::colorType(false, true, layer.hasPalette());
		oContext->header.colorType = PNGColorType::RGB;
		oContext->header.compression = PNGCompressionMethod::Deflate;
		oContext->header.filter = PNGFilterMethod::Adaptive;
		oContext->header.interlace = PNGInterlaceMethod::None;

		PNGChunk::write(oContext->output, PNGChunk::IHDR, reinterpret_cast<unsigned char *>(&oContext->header), sizeof(oContext->header));

		// Write palette
		// TODO: not supported

		// Initialize deflate
		oContext->strm.zalloc = (alloc_func)0;
		oContext->strm.zfree  = (free_func)0;
		oContext->strm.opaque = (voidpf)0;

		deflateInit(&oContext->strm, Z_DEFAULT_COMPRESSION);

		// Prepare previous scanline
		oContext->scanlinePrev.resize(slw);
		memset(&oContext->scanlinePrev[0], 0, slw);

		// Initialize deflate buffer
		oContext->compData = boost::shared_array<byte>(new byte[DEFLATE_BUFFER]);
		oContext->strm.next_out = &oContext->compData[0];
		oContext->strm.avail_out = DEFLATE_BUFFER;
	}
	catch(...)
	{
		// Ignore
	}


	return oContext;
}


void SaveByLinePlugin_PNG::WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanlineOrig) const
{
	if(!context)
		return;

	SaveByLineContext *oContext = dynamic_cast<SaveByLineContext *>(context);

	int slw = static_cast<int>(scanlineOrig.size());
	RasterLayer<byte>::Scanline scanline(slw + 1);

	applyFilter(scanline, scanlineOrig, oContext->scanlinePrev, slw, PNGColorType::samplesPerPixel(oContext->header.colorType), oContext->header.bitDepth);

	// Compress scanline
	oContext->strm.next_in = &scanline[0];
	oContext->strm.avail_in = slw + 1;

	// Write data
	deflateAndWriteData(Z_NO_FLUSH, oContext);

	oContext->scanlinePrev = scanlineOrig;

	return;
}


bool nearZero(byte value)
{
	return value == 0;
}

void SaveByLinePlugin_PNG::applyFilter(RasterLayer<byte>::Scanline &scanline, const RasterLayer<byte>::Scanline &scanlineOrig,
	const RasterLayer<byte>::Scanline &scanlinePrev, unsigned int slw, unsigned int spp, unsigned int bps) const
{
	if(true)
	{
		// Filter: always Paeth
		scanline[0] = PNGFilterType::Paeth;
		filterFunc filter = getFilter(scanline[0]);
		filterScanline(filter, -1, &scanline[1], &scanlineOrig[0], &scanlinePrev[0], slw, spp, bps);
	}
	else
	{
		// The following does not result in better compression
		RasterLayer<byte>::Scanline scanlineNone(slw), scanlineSub(slw), scanlineUp(slw), scanlineAverage(slw), scanlinePaeth(slw);

		filterScanline(getFilter(PNGFilterType::None), -1, &scanlineNone[0], &scanlineOrig[0], &scanlinePrev[0], slw, spp, bps);
		filterScanline(getFilter(PNGFilterType::Sub), -1, &scanlineSub[0], &scanlineOrig[0], &scanlinePrev[0], slw, spp, bps);
		filterScanline(getFilter(PNGFilterType::Up), -1, &scanlineUp[0], &scanlineOrig[0], &scanlinePrev[0], slw, spp, bps);
		filterScanline(getFilter(PNGFilterType::Average), -1, &scanlineAverage[0], &scanlineOrig[0], &scanlinePrev[0], slw, spp, bps);
		filterScanline(getFilter(PNGFilterType::Paeth), -1, &scanlinePaeth[0], &scanlineOrig[0], &scanlinePrev[0], slw, spp, bps);

		// Count all zeroes
		//int countNone = std::count_if(scanlineNone.begin(), scanlineNone.end(), nearZero);
		//int countSub = std::count_if(scanlineSub.begin(), scanlineSub.end(), nearZero);
		//int countUp = std::count_if(scanlineUp.begin(), scanlineUp.end(), nearZero);
		//int countAverage = std::count_if(scanlineAverage.begin(), scanlineAverage.end(), nearZero);
		//int countPaeth = std::count_if(scanlinePaeth.begin(), scanlinePaeth.end(), nearZero);
		//std::cout << countNone << "," << countSub << "," << countUp << "," << countAverage << "," << countPaeth << " ";

		//int maxCount = std::max(countNone, std::max(countSub, std::max(countUp, std::max(countAverage, countPaeth))));
		//if(maxCount == countNone)
		//{
		//	scanline[0] = PNGFilterType::None;
		//	memcpy(&scanline[1], &scanlineNone[0], slw);
		//	std::cout << "None" << std::endl;
		//}
		//else if(maxCount == countSub)
		//{
		//	scanline[0] = PNGFilterType::Sub;
		//	memcpy(&scanline[1], &scanlineSub[0], slw);
		//	std::cout << "Sub" << std::endl;
		//}
		//else if(maxCount == countUp)
		//{
		//	scanline[0] = PNGFilterType::Up;
		//	memcpy(&scanline[1], &scanlineUp[0], slw);
		//	std::cout << "Up" << std::endl;
		//}
		//else if(maxCount == countAverage)
		//{
		//	scanline[0] = PNGFilterType::Average;
		//	memcpy(&scanline[1], &scanlineAverage[0], slw);
		//	std::cout << "Average" << std::endl;
		//}
		//else if(maxCount == countPaeth)
		//{
		//	scanline[0] = PNGFilterType::Paeth;
		//	memcpy(&scanline[1], &scanlinePaeth[0], slw);
		//	std::cout << "Paeth" << std::endl;
		//}

		//RasterLayer<byte>::Scanline scanlineCopy(slw);

		//std::copy(scanlineNone.begin(), scanlineNone.end(), scanlineCopy.begin());
		//std::sort(scanlineCopy.begin(), scanlineCopy.end());
		//int countUniqueNone = std::unique(scanlineCopy.begin(), scanlineCopy.end()) - scanlineCopy.begin();
		//std::copy(scanlineSub.begin(), scanlineSub.end(), scanlineCopy.begin());
		//std::sort(scanlineCopy.begin(), scanlineCopy.end());
		//int countUniqueSub = std::unique(scanlineCopy.begin(), scanlineCopy.end()) - scanlineCopy.begin();
		//std::copy(scanlineUp.begin(), scanlineUp.end(), scanlineCopy.begin());
		//std::sort(scanlineCopy.begin(), scanlineCopy.end());
		//int countUniqueUp = std::unique(scanlineCopy.begin(), scanlineCopy.end()) - scanlineCopy.begin();
		//std::copy(scanlineAverage.begin(), scanlineAverage.end(), scanlineCopy.begin());
		//std::sort(scanlineCopy.begin(), scanlineCopy.end());
		//int countUniqueAverage = std::unique(scanlineCopy.begin(), scanlineCopy.end()) - scanlineCopy.begin();
		//std::copy(scanlinePaeth.begin(), scanlinePaeth.end(), scanlineCopy.begin());
		//std::sort(scanlineCopy.begin(), scanlineCopy.end());
		//int countUniquePaeth = std::unique(scanlineCopy.begin(), scanlineCopy.end()) - scanlineCopy.begin();

		int countUniqueNone = sumArray(scanlineNone);
		int countUniqueSub = sumArray(scanlineSub);
		int countUniqueUp = sumArray(scanlineUp);
		int countUniqueAverage = sumArray(scanlineAverage);
		int countUniquePaeth = sumArray(scanlinePaeth);
		//double entropyNone = entropyArray(scanlineNone);
		//double entropySub = entropyArray(scanlineSub);
		//double entropyUp = entropyArray(scanlineUp);
		//double entropyAverage = entropyArray(scanlineAverage);
		//double entropyPaeth = entropyArray(scanlinePaeth);

		//std::cout << countUniqueNone << "," << countUniqueSub << "," << countUniqueUp << "," << countUniqueAverage << "," << countUniquePaeth << " ";


		int minCount = std::min(countUniqueNone, std::min(countUniqueSub, std::min(countUniqueUp, std::min(countUniqueAverage, countUniquePaeth))));
		if(minCount == countUniqueNone)
		{
			scanline[0] = PNGFilterType::None;
			memcpy(&scanline[1], &scanlineNone[0], slw);
			//std::cout << "None" << std::endl;
		}
		else if(minCount == countUniqueSub)
		{
			scanline[0] = PNGFilterType::Sub;
			memcpy(&scanline[1], &scanlineSub[0], slw);
			//std::cout << "Sub" << std::endl;
		}
		else if(minCount == countUniqueUp)
		{
			scanline[0] = PNGFilterType::Up;
			memcpy(&scanline[1], &scanlineUp[0], slw);
			//std::cout << "Up" << std::endl;
		}
		else if(minCount == countUniqueAverage)
		{
			scanline[0] = PNGFilterType::Average;
			memcpy(&scanline[1], &scanlineAverage[0], slw);
			//std::cout << "Average" << std::endl;
		}
		else if(minCount == countUniquePaeth)
		{
			scanline[0] = PNGFilterType::Paeth;
			memcpy(&scanline[1], &scanlinePaeth[0], slw);
			//std::cout << "Paeth" << std::endl;
		}
	}
}


int SaveByLinePlugin_PNG::sumArray(const std::vector<byte> &array) const
{
	int sum = 0;
	for(std::vector<byte>::const_iterator iter = array.begin(); iter != array.end(); ++iter)
	{
		sum += abs(static_cast<signed char>(*iter));
	}

	return sum;
}


double SaveByLinePlugin_PNG::entropyArray(const std::vector<byte> &array) const
{
	// Entropy
	int hist[256] = {0};
	double len = 1.0f * array.size();
	for(std::vector<byte>::const_iterator iter = array.begin(); iter != array.end(); ++iter)
	{
		++hist[*iter];
	}

	double ent = 0;
	for(int i = 0; i < 256; ++i)
	{
		//showVal(hist[i]);
		if(hist[i] > 0)
			ent += hist[i] / 256.0 * log(hist[i] / 256.0);
	}

	return -ent;
}


void SaveByLinePlugin_PNG::Close(IContext *&context) const
{
	if(!context)
		return;

	SaveByLineContext *oContext = dynamic_cast<SaveByLineContext *>(context);

	// Write data
	deflateAndWriteData(Z_FINISH, oContext);

	unsigned int have = DEFLATE_BUFFER - oContext->strm.avail_out;
	PNGChunk::write(oContext->output, PNGChunk::IDAT, &oContext->compData[0], have);

	deflateEnd(&oContext->strm);

	// Write end chunk
	PNGChunk::write(oContext->output, PNGChunk::IEND, 0, 0);

	delete context;
	context = 0;

	if(m_fOut)
		m_fOut.close();
}


void SaveByLinePlugin_PNG::deflateAndWriteData(int flush, SaveByLineContext *oContext) const
{
	do
	{
		int ret = deflate(&oContext->strm, flush);
		assert(ret != Z_STREAM_ERROR);

		if(oContext->strm.avail_out != 0)
			break;

		unsigned int have = DEFLATE_BUFFER - oContext->strm.avail_out;

		PNGChunk::write(oContext->output, PNGChunk::IDAT, &oContext->compData[0], have);
		oContext->strm.avail_out = DEFLATE_BUFFER;
		oContext->strm.next_out = &oContext->compData[0];

	} while(true);
}


bool PNGColorType::hasAlpha(unsigned int colorType)
{
	return (colorType & 4) != 0;
}

bool PNGColorType::hasColor(unsigned int colorType)
{
	return (colorType & 2) != 0;
}

bool PNGColorType::hasPalette(unsigned int colorType)
{
	return (colorType & 1) != 0;
}

unsigned int PNGColorType::samplesPerPixel(unsigned int colorType)
{
	switch(colorType)
	{
	case PNGColorType::Grayscale:
		return 1;
	case PNGColorType::RGB:
		return 3;
	case PNGColorType::Palette:
		return 1;
	case PNGColorType::GrayscaleAlpha:
		return 2;
	case PNGColorType::RGBAlpha:
		return 4;
	default:
		return 0;
	}
}

unsigned int PNGColorType::colorType(bool hasAlpha, bool hasColor, bool hasPalette)
{
	return (hasAlpha ? 4 : 0) | (hasColor ? 2 : 0) | (hasPalette ? 1 : 0);
}

bool PNGChunk::isAncillary(unsigned int chunk)
{
	return (chunk & 0x10000000) != 0;
}

bool PNGChunk::isPrivate(unsigned int chunk)
{
	return (chunk & 0x100000) != 0;
}

bool PNGChunk::isReserved(unsigned int chunk)
{
	return (chunk & 0x1000) != 0;
}

bool PNGChunk::isSafeToCopy(unsigned int chunk)
{
	return (chunk & 0x10) != 0;
}

PNGChunk::PNGChunk():
	length(0), chunk(0), crc(0)
{
}

PNGChunk PNGChunk::read(std::istream &str)
{
	PNGChunk chunk;

	// Read length
	str.read(reinterpret_cast<char *>(&chunk.length), sizeof(chunk.length));
	swapInt(chunk.length);

	// Read chunk type
	str.read(reinterpret_cast<char *>(&chunk.chunk), sizeof(chunk.chunk));
	swapInt(chunk.chunk);

	// Read data
	chunk.data = boost::shared_array<unsigned char>(new unsigned char[chunk.length]);
	str.read(reinterpret_cast<char *>(chunk.data.get()), chunk.length);

	// Read crc
	str.read(reinterpret_cast<char *>(&chunk.crc), sizeof(chunk.crc));
	swapInt(chunk.crc);

	// Validate data
	/*
	showVal((int)chunk.data.get()[3]);
	showVal(chunk.length);
	if(static_cast<unsigned long>(chunk.crc) != ::crc(chunk.data.get(), chunk.length))
	{
		std::cout.setf(std::ios::hex);
		showVal(chunk.crc);
		showVal(::crc(chunk.data.get(), chunk.length));
		std::cout.setf(std::ios::dec);
		throw "Invalid CRC";
	}
	*/

	return chunk;
}

void PNGChunk::copyData(void *dest) const
{
	memcpy(dest, this->data.get(), this->length);
}

void PNGChunk::write(std::ostream &str, unsigned int chunkType, const unsigned char *data, unsigned int length)
{
	// Write length
	unsigned int lengthSwapped = swapIntValue(length);
	str.write(reinterpret_cast<char *>(&lengthSwapped), sizeof(lengthSwapped));

	// Write chunk type
	//std::cout << "write chunk type" << std::endl;
	swapInt(chunkType);
	str.write(reinterpret_cast<char *>(&chunkType), sizeof(chunkType));

	// Write data
	//std::cout << "write data" << std::endl;
	str.write(reinterpret_cast<const char *>(data), length);

	// Calculate crc
	//std::cout << "calculate crc" << std::endl;
	//unsigned int crcC = ::crc(data, length);
	unsigned int crcC = 0xFFFFFFFF;
	crcC = update_crc(crcC, reinterpret_cast<Byte *>(&chunkType), sizeof(chunkType));
	crcC = update_crc(crcC, data, length);
	crcC ^= 0xFFFFFFFFL;
	//std::cout << crcC << std::endl;

	// Write crc
	swapInt(crcC);
	str.write(reinterpret_cast<char *>(&crcC), sizeof(crcC));
}

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
	unsigned int c, n, k;

	for(n = 0; n < 256; n++)
	{
		c = n;
		for(k = 0; k < 8; k++)
		{
			if(c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[n] = c;
	}
	crc_table_computed = 1;
}

/*  Update a running CRC with the bytes buf[0..len-1]-the CRC
	should be initialized to all 1's, and the transmitted value
	is the 1's complement of the final running CRC (see the
	crc() routine below). */

unsigned int update_crc(unsigned int crc, const unsigned char *buf, unsigned int len)
{
	unsigned int c = crc, n;

	if(!crc_table_computed)
		make_crc_table();

	for(n = 0; n < len; n++)
	{
		c = crc_table[(c ^ buf[n]) & 0xFF] ^ (c >> 8);
	}
	return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
//unsigned int crc(const unsigned char *buf, unsigned int len)
//{
//	return update_crc(0xFFFFFFFFL, buf, len) ^ 0xFFFFFFFFL;
//}

filterFunc getFilter(const int filterType)
{
	filterFunc filter = 0;

	// Select filter
	switch(filterType)
	{
	case PNGFilterType::Paeth:
		filter = filterPaeth;
		break;
	case PNGFilterType::None:
		filter = filterNone;
		break;
	case PNGFilterType::Sub:
		filter = filterSub;
		break;
	case PNGFilterType::Up:
		filter = filterUp;
		break;
	case PNGFilterType::Average:
		filter = filterAverage;
		break;
	}

	return filter;
}

int filterScanline(filterFunc filter, int filterDir, byte *sl, const byte *slOrig, const byte *slPrev, unsigned int width, unsigned int sppa, unsigned int bps)
{
	if(!filter)
		return -1;

	const byte *slGet;
	if(filterDir > 0)
		slGet = sl;
	else
		slGet = slOrig;

	unsigned int bppa = sppa * bps / 8;
	const byte *pArgA = slGet;
	const byte *pArgC = slPrev;

	const byte *pArgB = pArgA - bppa;
	const byte *pArgD = pArgC - bppa;
	for(unsigned int x = 0; x < width; ++x, ++pArgA, ++pArgB, ++pArgC, ++pArgD)
	{
		if(x < bppa)
			sl[x] = filter(filterDir, *pArgA, 0, *pArgC, 0);
		else
			sl[x] = filter(filterDir, *pArgA, *pArgB, *pArgC, *pArgD);
	}

	return 0;
}
