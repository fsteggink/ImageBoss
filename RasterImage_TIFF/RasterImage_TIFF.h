// RasterImage_TIFF.h
// FS, 25-01-2009

#ifndef RASTERIMAGE_TIFF_H
#define RASTERIMAGE_TIFF_H

#ifndef DLLAPI_RASTERIMAGE_TIFF
	#ifdef DLL_INTERNAL_COMPILE
		#define DLLAPI_RASTERIMAGE_TIFF __declspec(dllexport)
	#else
		#define DLLAPI_RASTERIMAGE_TIFF __declspec(dllimport)
	#endif
#endif

#include "../../Inc/ImageBoss.h"
#include "../../Inc/RasterLayer.h"
#include <fstream>
#include <algorithm>


////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////

const Interface g_ceSupportedInterfaces = \
//static_cast<Interface>(Interface::RasterLoad | Interface::RasterSave | Interface::RasterInfo | Interface::RasterLineLoad | Interface::RasterLineSave);
static_cast<Interface>(Interface::RasterLoad | Interface::RasterInfo);

struct TIFFFieldType
{
	static const int BYTE = 1;         // 8-bit unsigned integer
	static const int ASCII = 2;        // 8-bit byte that contains a 7-bit ASCII CODE; terminates with NULL
	static const int SHORT = 3;        // 16-bit (2-byte) unsigned integer
	static const int LONG = 4;         // 32-bit (4-byte) unsigned integer
	static const int RATIONAL = 5;     // Two LONGs: numerator and denominator
	static const int SBYTE = 6;        // An 8-bit signed (twos-complement) integer
	static const int UNDEFINED = 7;    // An 8-bit byte that may contain anything; depending on the definition of the field
	static const int SSHORT = 8;       // A 16-bit (2-byte) signed (twos-complement) integer
	static const int SLONG = 9;        // A 32-bit (4-byte) signed (twos-complement) integer
	static const int SRATIONAL = 10;   // Two SLongs: numerator and denominator
	static const int FLOAT = 11;       // Single precision (4-byte) IEEE format
	static const int DOUBLE = 12;      // Double precision (8-byte) IEEE format

	static int SIZES[];                // Holds the sizes of the various data types; indexed by value - 1
};

int TIFFFieldType::SIZES[] = { 1, 1, 2, 4, 8, 1, 1, 2, 4, 8, 4, 8};

struct TIFFTag
{
	static const int NewSubfileType = 254;
	static const int SubfileType = 255;
	static const int ImageWidth = 256;
	static const int ImageLength = 257;
	static const int BitsPerSample = 258;
	static const int Compression = 259;                 // See TIFFCompression
	static const int PhotometricInterpretation = 262;   // See TIFFPhotometricInterpretation
	static const int Threshholding = 263;
	static const int CellWidth = 264;
	static const int CellLength = 265;
	static const int FillOrder = 266;
	static const int DocumentName = 269;
	static const int ImageDescription = 270;
	static const int Make = 271;
	static const int Model = 272;
	static const int StripOffsets = 273;
	static const int Orientation = 274;
	static const int SamplesPerPixel = 277;
	static const int RowsPerStrip = 278;
	static const int StripByteCounts = 279;
	static const int MinSampleValue = 280;
	static const int MaxSampleValue = 281;
	static const int XResolution = 282;
	static const int YResolution = 283;
	static const int PlanarConfiguration = 284;         // 1 = Chunky, 2 = Planar
	static const int PageName = 285;
	static const int XPosition = 286;
	static const int YPosition = 287;
	static const int FreeOffsets = 288;
	static const int FreeByteCounts = 289;
	static const int GrayResponseUnit = 290;
	static const int GrayResponseCurve = 291;
	static const int T4Options = 292;
	static const int T6Options = 293;
	static const int ResolutionUnit = 296;
	static const int PageNumber = 297;
	static const int TransferFunction = 301;
	static const int Software = 305;
	static const int DateTime = 306;
	static const int Artist = 315;
	static const int HostComputer = 316;
	static const int Predictor = 317;
	static const int WhitePoint = 318;
	static const int PrimaryChromaticities = 319;
	static const int ColorMap = 320;                    // 3 * (2**BitsPerSample)
	static const int HalftoneHints = 321;
	static const int TileWidth = 322;
	static const int TileLength = 323;
	static const int TileOffsets = 324;
	static const int TileByteCounts = 325;
	static const int InkSet = 332;
	static const int InkNames = 333;
	static const int NumberOfInks = 334;
	static const int DotRange = 336;
	static const int TargetPrinter = 337;
	static const int ExtraSamples = 338;
	static const int SampleFormat = 339;
	static const int SMinSampleValue = 340;
	static const int SMaxSampleValue = 341;
	static const int TransferRange = 342;
	static const int JPEGProc = 512;
	static const int JPEGInterchangeFormat = 513;
	static const int JPEGInterchangeFormatLength = 514;
	static const int JPEGRestartInterval = 515;
	static const int JPEGLosslessPredictors = 517;
	static const int JPEGPointTransforms = 518;
	static const int JPEGQTables = 519;
	static const int JPEGDCTables = 520;
	static const int JPEGACTables = 521;
	static const int YCbCrCoefficients = 529;
	static const int YCbCrSubSampling = 530;
	static const int YCbCrPositioning = 531;
	static const int ReferenceBlackWhite = 532;
	static const int Copyright = 33432;
};

struct TIFFCompression
{
	static const int None = 1;
	static const int CCITTGroup3 = 2;
	static const int CCITTGroup4 = 3;
	static const int CCITTGroup6 = 4;
	static const int LZW = 5;
	static const int JPEG = 6;
	static const int PackBits = 32773;
};

struct TIFFPhotometricInterpretation
{
	static const int WhiteIsZero = 0;
	static const int BlackIsZero = 1;
	static const int RGB = 2;
	static const int PaletteColor = 3;
	static const int TransparencyMask = 4;
	static const int CMYK = 5;
	static const int YCbCr = 6;
	static const int CIELAB = 8;
};

struct TIFFPredictor
{
	static const int None = 1;
	static const int HorizontalDifferencing = 2;
};


////////////////////////////////////////////////////////////
// Exported function declarations
////////////////////////////////////////////////////////////

bool DLLAPI_RASTERIMAGE_TIFF SupportsInterface(Interface eIF);
Interface DLLAPI_RASTERIMAGE_TIFF SupportedInterfaces();
boost::shared_ptr<IImagePlugin> DLLAPI_RASTERIMAGE_TIFF CreatePlugin(Interface eIF);


////////////////////////////////////////////////////////////
// Inline function definitions
////////////////////////////////////////////////////////////

inline void swapShort(unsigned short &value)
{
	value = (value & 0xFF00) >> 8 | (value & 0xFF) << 8;
}

inline void swapInt(unsigned int &value)
{
	value =
		(value & 0xFF000000) >> 24 |
		(value & 0xFF0000) >> 8 |
		(value & 0xFF00) << 8 |
		(value & 0xFF) << 24;
}

inline void swapLong(unsigned __int64 &value)
{
	value =
		(value & 0xFF00000000000000) >> 56 |
		(value & 0xFF000000000000) >> 40 |
		(value & 0xFF0000000000) >> 24 |
		(value & 0xFF00000000) >> 8 |
		(value & 0xFF000000) << 8 |
		(value & 0xFF0000) << 24 |
		(value & 0xFF00) << 40 |
		(value & 0xFF) << 56;
}

inline unsigned short swapShortValue(unsigned short value)
{
	return (value & 0xFF00) >> 8 | (value & 0xFF) << 8;
}

inline unsigned int swapIntValue(unsigned int value)
{
	return
		(value & 0xFF000000) >> 24 |
		(value & 0xFF0000) >> 8 |
		(value & 0xFF00) << 8 |
		(value & 0xFF) << 24;
}

inline unsigned __int64 swapLongValue(unsigned __int64 value)
{
	return
		(value & 0xFF00000000000000) >> 56 |
		(value & 0xFF000000000000) >> 40 |
		(value & 0xFF0000000000) >> 24 |
		(value & 0xFF00000000) >> 8 |
		(value & 0xFF000000) << 8 |
		(value & 0xFF0000) << 24 |
		(value & 0xFF00) << 40 |
		(value & 0xFF) << 56;
}


////////////////////////////////////////////////////////////
// Struct definitions
////////////////////////////////////////////////////////////

#pragma pack(push, 1)

struct TIFFHeader
{
	unsigned short endianness;
	unsigned short check;
	unsigned int firstIFD;
};

struct IFDEntry
{
	unsigned short tag;
	unsigned short fieldType;
	unsigned int count;
	union
	{
		unsigned int value;
		unsigned int offset;
	};
};

#pragma pack(pop)

// Reads the first IFD entry value
template<typename T>
T readIFDValue(const IFDEntry &entry, std::istream &input, bool bigEndian, T dummy)
{
	std::vector<T> values = readIFDValues(entry, input, bigEndian, dummy);
	return values[0];
}

// Reads all IFD entry values, and returns them as an vector
template<typename T>
std::vector<T> readIFDValues(const IFDEntry &entry, std::istream &input, bool bigEndian, T dummy)
{
	unsigned int size = entry.count * TIFFFieldType::SIZES[entry.fieldType - 1];
	boost::shared_array<char> buf(new char [size]);

	if(size <= 4)
	{
		// Interpret union field as value: copy data
		memcpy(buf.get(), &entry.value, size);
	}
	else
	{
		// Interpret union field as file offset: read from stream
		input.seekg(entry.offset, std::ios::beg);
		input.read(buf.get(), size);
	}

	// Swap values, if needed. This also depends on the data size.
	if(bigEndian)
	{
		switch(TIFFFieldType::SIZES[entry.fieldType - 1])
		{
		case 2:
			{
				unsigned short *data = reinterpret_cast<unsigned short *>(buf.get());
				for(unsigned int i = 0; i < entry.count; ++i)
				{
					swapShort(data[i]);
				}
			}
			break;

		case 4:
			{
				unsigned int *data = reinterpret_cast<unsigned int *>(buf.get());
				for(unsigned int i = 0; i < entry.count; ++i)
				{
					swapInt(data[i]);
				}
			}
			break;

		case 8:
			{
				unsigned __int64 *data = reinterpret_cast<unsigned __int64 *>(buf.get());
				for(unsigned int i = 0; i < entry.count; ++i)
				{
					swapLong(data[i]);
				
				}
			}
			break;

		default:
			// Do nothing
			break;
		}
	}

	// Copy values to results array
	std::vector<T> result(entry.count);
	switch(entry.fieldType)
	{
	case TIFFFieldType::BYTE:
		{
			unsigned char *data = reinterpret_cast<unsigned char *>(buf.get());
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				result[i] = static_cast<T>(data[i]);
			}
		}
		break;

	case TIFFFieldType::ASCII:
	case TIFFFieldType::UNDEFINED:
		{
			char *data = buf.get();
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				result[i] = static_cast<T>(data[i]);
			}
		}
		break;

	case TIFFFieldType::SHORT:
		{
			unsigned short *data = reinterpret_cast<unsigned short *>(buf.get());
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				result[i] = static_cast<T>(data[i]);
			}
		}
		break;

	case TIFFFieldType::LONG:
		{
			unsigned int *data = reinterpret_cast<unsigned int *>(buf.get());
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				result[i] = static_cast<T>(data[i]);
			}
		}
		break;

	case TIFFFieldType::RATIONAL:
		{
			unsigned __int64 *data = reinterpret_cast<unsigned __int64 *>(buf.get());
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				result[i] = static_cast<T>(static_cast<double>(data[i] << 32) / static_cast<double>(data[i] >> 32));
			}
		}
		break;

	case TIFFFieldType::SBYTE:
		{
			signed char *data = reinterpret_cast<signed char *>(buf.get());
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				result[i] = static_cast<T>(data[i]);
			}
		}
		break;

	case TIFFFieldType::SSHORT:
		{
			short *data = reinterpret_cast<short *>(buf.get());
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				result[i] = static_cast<T>(data[i]);
			}
		}
		break;

	case TIFFFieldType::SLONG:
		{
			int *data = reinterpret_cast<int *>(buf.get());
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				result[i] = static_cast<T>(data[i]);
			}
		}
		break;

	case TIFFFieldType::SRATIONAL:
		{
			__int64 *data = reinterpret_cast<__int64 *>(buf.get());
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				// Need to cast both parts explicitly to int32?
				result[i] = static_cast<T>(static_cast<double>(data[i] << 32) / static_cast<double>(data[i] >> 32));
			}
		}
		break;

	case TIFFFieldType::FLOAT:
		{
			float *data = reinterpret_cast<float *>(buf.get());
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				result[i] = static_cast<T>(data[i]);
			}
		}
		break;

	case TIFFFieldType::DOUBLE:
		{
			double *data = reinterpret_cast<double *>(buf.get());
			for(unsigned int i = 0; i < entry.count; ++i)
			{
				result[i] = static_cast<T>(data[i]);
			}
		}
		break;
	}

	return result;
}

class IFD
{
private:
	unsigned int m_nextIFD;
	std::vector<IFDEntry> m_entries;
	std::map<unsigned short, IFDEntry> m_entryMap;

public:
	IFD() {};
	
	void readEntries(std::istream &input, bool bigEndian)
	{
		unsigned short size;
		input.read(reinterpret_cast<char *>(&size), sizeof(unsigned short));
		if(bigEndian)
			swapShort(size);

		m_entries.resize(size);
		input.read(reinterpret_cast<char *>(&m_entries[0]), size * sizeof(IFDEntry));

		for(std::vector<IFDEntry>::iterator iterEntry = m_entries.begin();
			iterEntry != m_entries.end(); ++iterEntry)
		{
			if(bigEndian)
			{
				swapShort(iterEntry->tag);
				swapShort(iterEntry->fieldType);
				swapInt(iterEntry->count);
				swapInt(iterEntry->offset);
			}

			m_entryMap.insert(std::pair<unsigned short, IFDEntry>(iterEntry->tag, *iterEntry));
		}

		input.read(reinterpret_cast<char *>(&m_nextIFD), sizeof(unsigned int));
		if(bigEndian)
			swapInt(m_nextIFD);

		return;
	}

	const std::vector<IFDEntry> &entries() const
	{
		return m_entries;
	}

	//const std::map<unsigned short, IFDEntry> &entryMap() const
	//{
	//	return m_entryMap;
	//}

	unsigned int nextIFD() const
	{
		return m_nextIFD;
	}

	bool hasEntry(unsigned short tag) const
	{
		return m_entryMap.find(tag) != m_entryMap.end();
	}

	const IFDEntry &entry(unsigned short tag) const
	{
		return m_entryMap.find(tag)->second;
	}

	unsigned short fieldType(unsigned short tag) const
	{
		return m_entryMap.find(tag)->second.fieldType;
	}

	unsigned int count(unsigned short tag) const
	{
		return m_entryMap.find(tag)->second.count;
	}

	unsigned int value(unsigned short tag) const
	{
		return m_entryMap.find(tag)->second.value;
	}

	unsigned int offset(unsigned short tag) const
	{
		return m_entryMap.find(tag)->second.offset;
	}
};


////////////////////////////////////////////////////////////
// Class definitions
////////////////////////////////////////////////////////////

class IDataReader
{
public:
	IDataReader() {};

	void ReadData(boost::shared_ptr<RasterLayer<byte>> &layer, std::istream &input, const IFD &ifd, bool bigEndian);

protected:
	virtual void ReadBytes(boost::shared_ptr<RasterLayer<byte>> &strip, std::istream &input, char *buf, unsigned int byteCounts, int y0) = 0;
};


class UncompressedDataReader: public IDataReader
{
public:
	UncompressedDataReader() {};

private:
	void ReadBytes(boost::shared_ptr<RasterLayer<byte>> &strip, std::istream &input, char *buf, unsigned int byteCounts, int y0);
};


class LZWDataReader: public IDataReader
{
private:
	int m_predictor;

public:
	LZWDataReader(int predictor): m_predictor(predictor) {};

private:
	void ReadBytes(boost::shared_ptr<RasterLayer<byte>> &strip, std::istream &input, char *buf, unsigned int byteCounts, int y0);
};


class LoadPlugin_TIFF: public IRasterLoadPlugin
{
public:
	LoadPlugin_TIFF();
	~LoadPlugin_TIFF();

	FileMode CanLoad(const MimeType &mimeType) const;
	void Load(RasterImage &image, const std::string &fileName) const;
	void Load(RasterImage &image, const std::string &fileName, const Params &params) const;
	void Load(RasterImage &image, std::istream &input) const;
	void Load(RasterImage &image, std::istream &input, const Params &params) const;

private:
	boost::shared_ptr<IDataReader> CreateDataReader(int compression, int predictor) const;
};


//class SavePlugin_TIFF: public IRasterSavePlugin
//{
//public:
//	SavePlugin_TIFF();
//	~SavePlugin_TIFF();
//
//	FileMode CanSave(const MimeType &mimeType) const;
//	void Save(const RasterImage &image, const std::string &fileName) const;
//	void Save(const RasterImage &image, const std::string &fileName, const Params &params) const;
//	void Save(const RasterImage &image, std::ostream &output) const;
//	void Save(const RasterImage &image, std::ostream &output, const Params &params) const;
//};


class InfoPlugin_TIFF: public IRasterInfoPlugin
{
public:
	InfoPlugin_TIFF();
	~InfoPlugin_TIFF();

	bool CanInfo(const MimeType &mimeType) const;
	void Info(const std::string &fileName, int &width, int &height, int &samplesPerPixel,
		int &bitsPerSample) const;
};


//class LoadByLinePlugin_TIFF: public IRasterLoadByLinePlugin
//{
//	std::ifstream m_fIn;
//
//public:
//	LoadByLinePlugin_TIFF();
//	~LoadByLinePlugin_TIFF();
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
//
//
//class SaveByLinePlugin_TIFF: public IRasterSaveByLinePlugin
//{
//	std::ofstream m_fOut;
//
//public:
//	SaveByLinePlugin_TIFF();
//	~SaveByLinePlugin_TIFF();
//
//	FileMode CanSave(const MimeType &mimeType);
//	IContext *Open(const std::string &fileName, int width, int height, int spp);
//	IContext *Open(std::ostream &output, int width, int height, int spp);
//	void WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanline);
//	void Close(IContext *&context);
//};
//
//
//class LoadByLineContext: public IContext
//{
//public:
//	LoadByLineContext(std::istream &strIn): input(strIn), isFile(false)
//	{
//	}
//
//	std::istream &input;
//	bool isFile;
//};
//
//
//class SaveByLineContext: public IContext
//{
//public:
//	SaveByLineContext(std::ostream &strOut): output(strOut), isFile(false)
//	{
//	}
//
//	std::ostream &output;
//	bool isFile;
//};


////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////

bool OpenTiffFile(std::istream &input, TIFFHeader &header);
void initDic(int initCodeSize, std::vector<std::string> &gifDic);


////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////

extern boost::shared_ptr<LoadPlugin_TIFF> g_LoadPlugin;
//extern boost::shared_ptr<SavePlugin_TIFF> g_SavePlugin;
extern boost::shared_ptr<InfoPlugin_TIFF> g_InfoPlugin;
//extern boost::shared_ptr<LoadByLinePlugin_TIFF> g_LoadByLinePlugin;
//extern boost::shared_ptr<SaveByLinePlugin_TIFF> g_SaveByLinePlugin;


#endif //RASTERIMAGE_TIFF_H
