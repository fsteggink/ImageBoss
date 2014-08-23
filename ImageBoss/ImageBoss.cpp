// ImageBoss.cpp
// FS, 10-03-2007

// Testing framework for ImageBoss
// In this DLL a lot of interfaces are defined

#define DLLAPI_IMAGEBOSS __declspec(dllexport)

#include "RasterLayer_intern.h"
#include "ImageBoss_internal.h"
#include "../../Inc/StringLib.h"


////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////

std::vector<HMODULE> g_vecModules;
std::map<boost::shared_ptr<IImagePlugin>, HMODULE> g_mapPluginModules;
std::map<Interface, std::vector<boost::shared_ptr<IImagePlugin>>> g_mapPlugins;

bool g_bHasLoadedPlugins = false;
LARGE_INTEGER g_llStart;


void dummy()
{
	RasterLayer<byte> z(500, 500, 3);
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		try
		{
			QueryPerformanceCounter(&g_llStart);

			RasterImage::Init();
			LoadPlugins();
		}
		catch(ImageBossException exc)
		{
			std::cerr << exc.get_Message() << std::endl;
			return FALSE;
		}
		catch(std::exception exc)
		{
			return FALSE;
		}
		break;

	case DLL_THREAD_ATTACH:
		break;
	
	case DLL_THREAD_DETACH:
		break;
	
	case DLL_PROCESS_DETACH:
		UnloadPlugins();

		LARGE_INTEGER llEnd, llFreq;
		QueryPerformanceCounter(&llEnd);
		QueryPerformanceFrequency(&llFreq);
		double dDuration = static_cast<double>(llEnd.QuadPart - g_llStart.QuadPart) /
			static_cast<double>(llFreq.QuadPart);

		std::cout << "Duration: " << dDuration << " sec" << std::endl;

		break;
	}

    return TRUE;
}


void LoadPlugins()
{
	WIN32_FIND_DATA uFindFileData;
	HANDLE hResult;
	BOOL bResult;

	std::wstring sPattern = PLUGIN_DIR + L"*.dll";
	std::wstring sFile;

	hResult = FindFirstFile(sPattern.c_str(), &uFindFileData);
	if(hResult != INVALID_HANDLE_VALUE)
	{
		do
		{
			sFile = PLUGIN_DIR + L"\\" + uFindFileData.cFileName;
			HMODULE hPlugin = LoadLibrary(sFile.c_str());

			if(hPlugin != 0)
			{
				// Load plugin functions
				fn_SupportedInterfaces funcSuppInt = reinterpret_cast<fn_SupportedInterfaces>(
					GetProcAddress(hPlugin, SUPPORTED_INTERFACES.c_str()));
				fn_CreatePlugin funcCreate = reinterpret_cast<fn_CreatePlugin>(
					GetProcAddress(hPlugin, CREATE_PLUGIN.c_str()));

				if(funcSuppInt == 0)
					throw ImageBossException("funcSuppInt is null");
				if(funcCreate == 0)
					throw ImageBossException("funcCreate is null");

				Interface eIF = funcSuppInt();

				// Use a single byte as an integer against all possible interfaces
				Interface eIter = static_cast<Interface>(1);
				while(eIter)
				{
					if(eIF & eIter)
					{
						// Create plugin which supports this interface, and add it to the plugin
						// mappings
						boost::shared_ptr<IImagePlugin> ifPlugin = funcCreate(eIter);
						g_mapPluginModules[ifPlugin] = hPlugin;
						g_mapPlugins[eIter].push_back(ifPlugin);
					}

					eIter = static_cast<Interface>(eIter << 1);
				}

				g_vecModules.push_back(hPlugin);
			}
			bResult = FindNextFile(hResult, &uFindFileData);
		} while (bResult);

		bResult = FindClose(hResult);
	}
	else
	{
		std::cout << "Can't find files" << std::endl;
	}

	return;
}


void UnloadPlugins()
{
	// Unload plugins
	g_mapPluginModules.clear();

	// Unload modules
	for(std::vector<HMODULE>::iterator iterModule = g_vecModules.begin(); \
		iterModule != g_vecModules.end(); ++iterModule)
	{
		FreeLibrary(*iterModule);
		*iterModule = 0;
	}
	g_vecModules.clear();

	return;
}


// Required destructor implementations of interfaces
//Param::~Param()
//{
//}


ILayer::~ILayer()
{
}


IVectorLayer::~IVectorLayer()
{
}


IRasterLayer::~IRasterLayer()
{
}


//IBand::~IBand()
//{
//}


IImage::~IImage()
{
}


IVectorImage::~IVectorImage()
{
}


//IRasterImage::~IRasterImage()
//{
//}


IImagePlugin::~IImagePlugin()
{
}


IVectorPlugin::~IVectorPlugin()
{
}


IRasterPlugin::~IRasterPlugin()
{
}


IRasterLoadPlugin::~IRasterLoadPlugin()
{
}


IRasterSavePlugin::~IRasterSavePlugin()
{
}


IRasterInfoPlugin::~IRasterInfoPlugin()
{
}


IRasterLoadByLinePlugin::~IRasterLoadByLinePlugin()
{
}


IRasterSaveByLinePlugin::~IRasterSaveByLinePlugin()
{
}


void RasterLoadPlugin::Load(RasterImage &image, const std::string &fileName) const
{
	Load(image, fileName, Params());
}


void RasterLoadPlugin::Load(RasterImage &image, const std::string &fileName, const Params &params) const
{
	std::ifstream input(fileName.c_str(), std::ios::binary);
	if(!input)
		throw ImageBossException(EXC_CANT_LOAD_FILE);

	Load(image, input, params);
	input.close();
}


void RasterLoadPlugin::Load(RasterImage &image, std::istream &input) const
{
	Load(image, input, Params());
}


void RasterLoadPlugin::Load(RasterImage &image, std::istream &input, const Params &params) const
{
	if(!input)
		throw ImageBossException(EXC_INVALID_INPUTSTREAM);

	//LoadByLinePlugin_PNG lblp;
	boost::shared_ptr<IRasterLoadByLinePlugin> lblp = GetLoadByLinePlugin();
	IContext *ctxt = lblp->Open(input);

	int width = lblp->GetWidth(ctxt);
	int height = lblp->GetHeight(ctxt);
	int spp = lblp->GetSamplesPerPixel(ctxt);

	boost::shared_ptr<RasterLayer<byte>> layer = boost::shared_ptr<RasterLayer<byte>>(new RasterLayer<byte>(width, height, spp));
	if(lblp->HasPalette(ctxt))
		layer->setPalette(lblp->GetPalette(ctxt));

	int slw = layer->get_ScanlineWidth();
	RasterLayer<byte>::Scanline scanline(slw);

	for(int y = 0; y < height; ++y)
	{
		lblp->ReadScanline(ctxt, scanline);
		layer->setScanline(scanline.begin(), y);
	}

	lblp->Close(ctxt);

	image.Layers().push_back(layer);
}


boost::shared_ptr<IRasterLoadByLinePlugin> RasterLoadPlugin::GetLoadByLinePlugin() const
{
	return boost::shared_ptr<IRasterLoadByLinePlugin>();
}


void RasterSavePlugin::Save(const RasterImage &image, const std::string &fileName) const
{
	Save(image, fileName, Params());

	return;
}


void RasterSavePlugin::Save(const RasterImage &image, const std::string &fileName,
						   const Params &params) const
{
	std::ofstream output(fileName.c_str(), std::ios::binary);
	if(!output)
		throw ImageBossException(EXC_CANT_SAVE_FILE);

	Save(image, output, params);
	output.close();

	return;
}


void RasterSavePlugin::Save(const RasterImage &image, std::ostream &output) const
{
	Save(image, output, Params());

	return;
}


void RasterSavePlugin::Save(const RasterImage &image, std::ostream &output,
						   const Params &params) const
{
	if(!output)
		throw ImageBossException(EXC_INVALID_OUTPUTSTREAM);

	boost::shared_ptr<IRasterLayer> ifLayer = image.Layers()[0];

	RasterLayer<byte> &layer =
		*boost::dynamic_pointer_cast<RasterLayer<byte>, IRasterLayer>(ifLayer);

	int width = layer.get_Width();
	int height = layer.get_Height();
	int spp = layer.get_SamplesPerPixel();
	int slw = width * spp;
	RasterLayer<byte>::Scanline scanline(slw);

	//SaveByLinePlugin_PNG sblp;
	boost::shared_ptr<IRasterSaveByLinePlugin> sblp = GetSaveByLinePlugin();
	IContext *ctxt = sblp->Open(output, width, height, spp);

	// Write palette
	if(layer.hasPalette())
	{
		RasterLayer<byte>::Palette palette;
		layer.getPalette(palette);
		//PNGChunk::write(output, PNGChunk::PLTE, reinterpret_cast<unsigned char *>(&palette[0]), static_cast<unsigned int>(palette.size()));
		// TODO: write palette, needs a new method in RasterSaveByLinePlugin
	}
	
	for(int y = 0; y < height; ++y)
	{
		layer.getScanline(scanline.begin(), y);
		sblp->WriteScanline(ctxt, scanline);
	}

	sblp->Close(ctxt);
}


boost::shared_ptr<IRasterSaveByLinePlugin> RasterSavePlugin::GetSaveByLinePlugin() const
{
	return boost::shared_ptr<IRasterSaveByLinePlugin>();
}


IRasterManipulationPlugin::~IRasterManipulationPlugin()
{
}


ImageBossException::ImageBossException(std::string message): \
	m_sMessage(message)
{
}


const std::string &ImageBossException::get_Message() const
{
	return m_sMessage;
}


CoordinateOutOfBoundsException::CoordinateOutOfBoundsException(): \
	ImageBossException(EXC_COORDS_OUT_OF_BOUNDS)
{
}
