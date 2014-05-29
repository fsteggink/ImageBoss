// SplitTiles.h
// FS, 17-07-2007

#ifndef SPLITTILES_H
#define SPLITTILES_H

//#include <boost/filesystem.hpp>
#include <iostream>
#include "../../Inc/CmdApp.h"
#include "../../Inc/ImageBoss.h"
#include "../../Inc/RasterLayer_extern.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//namespace bfs=boost::filesystem;


const std::string PROGNAME = "SplitTiles";
const std::string PROGVERSION = "0.1a";


inline int atoi(const std::string &value)
{
	return atoi(value.c_str());
}


inline double atof(const std::string &value)
{
	return atof(value.c_str());
}


inline std::string s(int value)
{
	char buf[20];
	_snprintf(buf, 20, "%d", value);
	return std::string(buf);
}


void CheckAndCreateDir(std::string dir);


#endif // SPLITTILES_H
