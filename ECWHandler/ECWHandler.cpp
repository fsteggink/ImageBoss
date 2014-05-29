// ECWHandler.cpp
// 13-09-2007

#define DLLAPI_ECWHANDLER __declspec(dllexport)

#include "ECWHandler.h"

funcReadLine g_fnCallback;


void DLLAPI_ECWHANDLER CreateECW(const char *outputFile, int width, int height, funcReadLine callback, float quality)
{
	if(!outputFile)
		throw "Output file expected";
	else if(!callback)
		throw "Callback expected";
	else if(quality < 1.0f)
		throw "Invalid value for quality";
	else if(width < 1)
		throw "Invalid value for width";
	else if(height < 1)
		throw "Invalid value for height";

	g_fnCallback = callback;

	NCSEcwCompressClient *pClient;

	NCSecwInit();
	
	if(pClient = NCSEcwCompressAllocClient())
	{
		NCSError eError;

		pClient->nInputBands = 3;
		pClient->nInOutSizeX = width;
		pClient->nInOutSizeY = height;

		pClient->eCompressFormat = COMPRESS_RGB;
		pClient->fTargetCompression = quality;

		strcpy(pClient->szOutputFilename, outputFile);

		pClient->pReadCallback = ReadLineECW;
		pClient->pClientData = (void*)0;

		eError = NCSEcwCompressOpen(pClient, FALSE);

		if(eError == NCS_SUCCESS)
		{
			eError = NCSEcwCompress(pClient);
			NCSEcwCompressClose(pClient);
		}

		if(eError == NCS_SUCCESS)
		{

			fprintf(stdout, "Target ratio:    %.1lf\r\n"
							"Actual ratio:    %.1lf\r\n"
							"Output size:     %I64d bytes\r\n"
							"Time taken:      %.1lf seconds\r\n"
							"Data Rate:       %.1lf MB/s\r\n",
							pClient->fTargetCompression,
							pClient->fActualCompression,
							pClient->nOutputSize,
							pClient->fCompressionSeconds,
							pClient->fCompressionMBSec);
			fflush(stdout);
		}
		else
		{
			fprintf(stderr, "Compression error: %s\r\n", NCSGetErrorText(eError)); 
			fflush(stderr);
		}

		NCSEcwCompressFreeClient(pClient);
	}
	else
	{
		fprintf(stderr, "NCSEcwCompressAllocClient() failed!");
		fflush(stderr);
	}

	NCSecwShutdown();

	return;
}


static BOOLEAN ReadLineECW(NCSEcwCompressClient *pClient,
							unsigned int nNextLine,
							float **ppInputArray)
{
	//void ReadLine(const ContextData &uContext, RasterLayer<byte>::Scanline &sl, int y)

	//static RasterLayer<byte>::Scanline sl(pClient->nInOutSizeX * pClient->nInputBands);
	//const ContextData &uContext = *reinterpret_cast<const ContextData *>(pClient->pClientData);
	//ReadLine(uContext, sl, nNextLine);

	static std::vector<byte> sl(pClient->nInOutSizeX * pClient->nInputBands);
	g_fnCallback(sl, nNextLine);


	float *bandR = ppInputArray[0];
	float *bandG = ppInputArray[1];
	float *bandB = ppInputArray[2];

	for(int x = 0; x < (int)pClient->nInOutSizeX; ++x)
	{
		bandR[x] = (float)sl[x * 3 + 0];
		bandG[x] = (float)sl[x * 3 + 1];
		bandB[x] = (float)sl[x * 3 + 2];
	}

	return(TRUE);	/* would return FALSE on an error */
}
