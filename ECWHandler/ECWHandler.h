// ECWHandler.h
// FS, 13-09-2007

#ifndef ECWHANDLER_H
#define ECWHANDLER_H

#ifndef DLLAPI_ECWHANDLER
	#ifdef DLL_INTERNAL_COMPILE
		#define DLLAPI_ECWHANDLER __declspec(dllexport)
	#else
		#define DLLAPI_ECWHANDLER __declspec(dllimport)
	#endif
#endif

#include <vector>

#include "C:\\Program Files\\Earth Resource Mapping\\ECW SDK\\Source\\include\\NCSECWCompressClient.h"

#pragma comment(lib, "C:\\PROGRA~1\\EARTHR~1\\ECWSDK~1\\lib\\vc71\\NCSEcw.lib")
#pragma comment(lib, "C:\\PROGRA~1\\EARTHR~1\\ECWSDK~1\\lib\\vc71\\NCSEcwCu.lib")
#pragma comment(lib, "C:\\PROGRA~1\\EARTHR~1\\ECWSDK~1\\lib\\vc71\\NCSUtil.lib")

#define WIN32_LEAN_AND_MEAN
#include <windows.h>


typedef void (*funcReadLine)(std::vector<byte> &, int);


void DLLAPI_ECWHANDLER CreateECW(const char *outputFile, int width, int height, funcReadLine callback, float quality = 10.0f);


static BOOLEAN ReadLineECW(NCSEcwCompressClient *pClient,
							unsigned int nNextLine,
							float **ppInputArray);

#endif // ECWHANDLER_H
