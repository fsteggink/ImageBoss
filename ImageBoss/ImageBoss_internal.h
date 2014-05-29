// ImageBoss_internal.h
// FS, 13-03-2007

#ifndef IMAGEBOSS_INTERNAL_H
#define IMAGEBOSS_INTERNAL_H

#include "../../Inc/ImageBoss.h"
#include "../../Inc/Geo.h"
#include "../../Inc/Geo_wkt.h"

#include <fstream>


////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////

//#ifdef _DEBUG
//const std::string PLUGIN_DIR = "E:\\Frank\\private\\Dev\\CPP\\ImageBoss\\debug\\Plugins\\";
//#else
const std::wstring PLUGIN_DIR = L".\\Plugins\\";
//#endif

//const std::string SUPPORTS_INTERFACE = "SupportsInterface";
//const std::string SUPPORTED_INTERFACES = "SupportedInterfaces";
//const std::string CREATE_PLUGIN = "CreatePlugin";
const std::string SUPPORTS_INTERFACE = "?SupportsInterface@@YA_NW4Interface@@@Z";
const std::string SUPPORTED_INTERFACES = "?SupportedInterfaces@@YA?AW4Interface@@XZ";
const std::string CREATE_PLUGIN = "?CreatePlugin@@YA?AV?$shared_ptr@VIImagePlugin@@@boost@@W4Interface@@@Z";


////////////////////////////////////////////////////////////
// Type definitions
////////////////////////////////////////////////////////////

typedef bool (*fn_SupportsInterface)(Interface eIF);
typedef Interface (*fn_SupportedInterfaces)();
typedef boost::shared_ptr<IImagePlugin> (*fn_CreatePlugin)(Interface eIF);


////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////

extern std::vector<HMODULE> g_vecModules;
extern std::map<boost::shared_ptr<IImagePlugin>, HMODULE> g_mapPluginModules;
extern std::map<Interface, std::vector<boost::shared_ptr<IImagePlugin>>> g_mapPlugins;


////////////////////////////////////////////////////////////
// Function declarations
////////////////////////////////////////////////////////////

void LoadPlugins();
void UnloadPlugins();


////////////////////////////////////////////////////////////
// Structure declarations
////////////////////////////////////////////////////////////


#endif // IMAGEBOSS_INTERNAL_H
