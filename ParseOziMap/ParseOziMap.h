// ParseOziMap.h
// FS, 08-09-2007

#ifndef PARSEOZIMAP_H
#define PARSEOZIMAP_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "../../Inc/StringLib.h"
#include "../../Inc/CmdApp.h"
#include "../../Inc/Geo_wkt.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


// Constants
const std::string PROGNAME = "ParseOziMap";
const std::string PROGVERSION = "0.1a";
const std::string WHITESPACE = " \t\r\n\f";

const std::string g_ALIAS_PREFIX = "ALIAS_";

const std::string g_OZIPROJ_TM = "Transverse Mercator";
const std::string g_OZIPROJ_UTM = "(UTM) Universal Transverse Mercator";
const std::string g_OZIPROJ_LCC = "Lambert Conformal Conic";
const std::string g_OZIPROJ_BONNE = "Bonne";

const std::string g_OZIDAT_WGS84 = "WGS 84";
const std::string g_OZIDAT_WGS72 = "WGS 72";
const std::string g_OZIDAT_PULKOVO1 = "Pulkovo 1942 (1)";
const std::string g_OZIDAT_PULKOVO2 = "Pulkovo 1942 (2)";
const std::string g_OZIDAT_POTSDAM_DHDN = "Potsdam Rauenberg DHDN";
const std::string g_OZIDAT_RD = "Rijksdriehoeksmeting";


// Type definitions
typedef StringLib<char> SL;


// Function declarations
void parseMapFile(const std::string &sDir, const std::string &sOziMapFile, std::ostream &strHeader, std::ostream &strData);


// Inline function definitions
inline double atof(const std::string &sValue)
{
	return atof(sValue.c_str());
}


#endif // PARSEOZIMAP_H
