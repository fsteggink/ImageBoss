// JoinTiles.cpp
// FS, 11-03-2007

#include "JoinTiles.h"
#include <fstream>


void showSyntax();


int main(int argc, char *argv[])
{
	// This program joins a couple of tiles. This can be done with one- or two-dimensional tile
	// numbers.
	// It needs the following parameters
	// - Basic file name which can be used as a parameter for sprintf
	// - Output file name
	// - # dimensions (1 or 2)

	// One-dimensional:
	// - Start index number
	// - # tiles in x dir (negative if next column is left of current column)
	// - # tiles in y dir (negative if next row is above the current row)

	// Two-dimensional:
	// - Start index number X
	// - Start index number Y
	// - # tiles in x dir (negative if next column is left of current column)
	// - # tiles in y dir (negative if next row is above the current row)

	// It is assumed that all tiles have the same size, and the final image size will be calculated
	// as following: (#tilesX * img0.width) x (#tilesY * img0.height)
	// The size of the image will be adjusted for the size of the lower right tile.

	// If an image cannot be found, or if it is invalid, then it will not be used for the tiling.
	// This is also true for the first (top left/bottom left) tile. All images should be 24-bit RGB
	// images.

	if(argc < 4)
	{
		std::cout << "Not enough arguments" << std::endl;
		showSyntax();
		return 1;
	}

	std::string sPatt, sOutput;
	int dim = 0, sx = 0, sy = 0, dx = 0, dy = 0;
	sPatt = argv[1];
	sOutput = argv[2];
	dim = atoi(argv[3]);
	bool autoDiscover = false;

	if(dim == 1)
	{
		if(argc < 7)
		{
			std::cout << "Incorrect number of arguments for dimension 1" << std::endl;
			showSyntax();
			return 1;
		}

		sx = atoi(argv[4]);
		sy = 0;
		dx = atoi(argv[5]);
		dy = atoi(argv[6]);
	}
	else if(dim == 2)
	{
		if(argc < 6 || argc == 7)
		{
			std::cout << "Incorrect number of arguments for dimension 2" << std::endl;
			showSyntax();
			return 1;
		}

		sx = atoi(argv[4]);
		sy = atoi(argv[5]);

		if(argc >= 8)
		{
			dx = atoi(argv[6]);
			dy = atoi(argv[7]);
		}
		else
		{
			autoDiscover = true;

			// Try to find the last available tile, assuming there are no gaps
			dx = 1;
			dy = 1;
			
			while(true)
			{
				int idxx = (dim == 1) ? sx + (dy - 1) * dx + (dx - 1) : sx + (dx - 1);
				int idxy = (dim == 1) ? 0 : sy + (dy - 1);
				char sFile[1024] = {0};
				_snprintf(sFile, 1024, sPatt.c_str(), idxx, idxy);

				// Does file exist?
				std::ifstream fIn(sFile, std::ios::binary);
				if(!fIn)
					break;
				fIn.close();

				++dx;
			}
			--dx;
			
			while(true)
			{
				int idxx = (dim == 1) ? sx + (dy - 1) * dx + (dx - 1) : sx + (dx - 1);
				int idxy = (dim == 1) ? 0 : sy + (dy - 1);
				char sFile[1024] = {0};
				_snprintf(sFile, 1024, sPatt.c_str(), idxx, idxy);

				// Does file exist?
				std::ifstream fIn(sFile, std::ios::binary);
				if(!fIn)
					break;
				fIn.close();

				++dy;
			}
			--dy;

			std::cout << "Autodetect, dx: " << dx << ", dy: " << dy << std::endl;
		}
	}
	else
	{
		std::cout << "Invalid dimension" << std::endl;
		showSyntax();
		return 1;
	}

	try
	{
		bool bHasInitImage = false;

		boost::shared_ptr<RasterLayer<byte>> lyrOut;
		int width = 0, height = 0;

		for(int ix = 0; ix < dx; ++ix)
			std::cout << "-";
		std::cout << std::endl;

		// Get dimensions of last image
		int lastWidth = 0, lastHeight = 0;
		{
			int idxx = (dim == 1) ? sx + (dy - 1) * dx + (dx - 1) : sx + (dx - 1);
			int idxy = (dim == 1) ? 0 : sy + (dy - 1);
			char sFile[1024] = {0};
			_snprintf(sFile, 1024, sPatt.c_str(), idxx, idxy);
			
			RasterImage img;
			try
			{
				img.Load(sFile);
				boost::shared_ptr<RasterLayer<byte>> lyr = boost::dynamic_pointer_cast<RasterLayer<byte>>(img.Layers()[0]);
				lastWidth = lyr->get_Width();
				lastHeight = lyr->get_Height();
			}
			catch(...)
			{
				// Skip
			}
		}

		RasterImageWriter imgResult;
		IContext *oCtxt = 0;

		int newWidth = 0, newHeight = 0;
		unsigned int rowHeight = 0;

		// TODO: find reference image first, because otherwise it can occur no data is written when no image in a row is found

		for(int iy = 0; iy < dy; ++iy)
		{
			if(newWidth > 0)
			{
				lyrOut = boost::shared_ptr<RasterLayer<byte>>(new RasterLayer<byte>(newWidth, height, 3));

				// Color all pixels white
				// lyrOut->setBackground(pixel) doesn't work for some reason...
				RasterLayer<byte>::Scanline sl(lyrOut->get_ScanlineWidth());
				memset(&sl[0], 0xFF, lyrOut->get_ScanlineWidth());
				for(unsigned int y = 0; y < lyrOut->get_Height(); ++y)
				{
					lyrOut->setScanline(sl.begin(), y);
				}
			}

			for(int ix = 0; ix < dx; ++ix)
			{
				int idxx = (dim == 1) ? sx + iy * dx + ix : sx + ix;
				int idxy = (dim == 1) ? 0 : sy + iy;
				char sFile[1024] = {0};
				_snprintf(sFile, 1024, sPatt.c_str(), idxx, idxy);
				
				// Load image
				RasterImage img;
				try
				{
					img.Load(sFile);
					std::cout << "#";
				}
				catch(...)
				{
					//std::cout << "Cannot load image " << sFile << std::endl;
					std::cout << "O";
					continue;
				}

				//RasterLayer<byte> *lyr = dynamic_cast<RasterLayer<byte> *>(img.Layers()[0].get());
				boost::shared_ptr<RasterLayer<byte>> lyr = boost::dynamic_pointer_cast<RasterLayer<byte>>(img.Layers()[0]);
				if(!bHasInitImage)
				{
					// Create new layer
					width = lyr->get_Width();
					height = lyr->get_Height();

					newWidth = lastWidth > 0 ? width * (dx - 1) + lastWidth : width * dx;
					newHeight = lastHeight > 0 ? height * (dy - 1) + lastHeight : height * dy;

					lyrOut = boost::shared_ptr<RasterLayer<byte>>(new RasterLayer<byte>(newWidth, height, 3));
					oCtxt = imgResult.Open(sOutput, newWidth, newHeight, 3);

					// Color all pixels white
					// lyrOut->setBackground(pixel) doesn't work for some reason...
					RasterLayer<byte>::Scanline sl(lyrOut->get_ScanlineWidth());
					memset(&sl[0], 0xFF, lyrOut->get_ScanlineWidth());
					for(unsigned int y = 0; y < lyrOut->get_Height(); ++y)
					{
						lyrOut->setScanline(sl.begin(), y);
					}

					bHasInitImage = true;
				}

				// Copy layer to output layer
				unsigned int spp = lyr->get_SamplesPerPixel();
				RasterLayer<byte>::Scanline sl(lyr->get_Width() * 3);
				RasterLayer<byte>::Pixel px(3);
				for(unsigned int y = 0; y < lyr->get_Height(); ++y)
				{
					lyr->getScanline(sl.begin(), y);
					for(unsigned int x = 0; x < lyr->get_Width(); ++x)
					{
						if(spp == 3)
						{
							for(unsigned int k = 0; k < spp; ++k)
								px[k] = sl[x * spp + k];
						}
						else
						{
							for(unsigned int k = 0; k < 3; ++k)
								px[k] = sl[x];
						}
						lyrOut->setPixel(px.begin(), PixelCoordI(ix * width + x, y));
					}
				}
				rowHeight = lyr->get_Height();
			}

			// Write lyrOut to image
			for(unsigned int y = 0; y < rowHeight; ++y)
			{
				RasterLayer<byte>::Scanline sl(imgResult.get_ScanlineWidth());
				lyrOut->getScanline(sl.begin(), y);
				imgResult.WriteScanline(oCtxt, sl);
			}

			std::cout << std::endl;
		}

		/*if(lyrOut != 0)
		{
			RasterImage imgResult;
			imgResult.Layers().push_back(lyrOut);
			imgResult.Save(sOutput);
		}*/
		imgResult.Close(oCtxt);
	}
	catch(ImageBossException exc)
	{
		std::cout << "Exception caught: " << exc.get_Message() << std::endl;
	}
	catch(std::exception exc)
	{
		std::cout << "Unknown exception" << std::endl;
	}
	catch(char *exc)
	{
		std::cout << "Unknown exception: " << exc << std::endl;
	}
	catch(...)
	{
		std::cout << "Really unknown exception" << std::endl;
	}


	std::cout << "Done!" << std::endl;

	return 0;
}


void showSyntax()
{
	std::cout << "Syntax: JoinTiles.exe <patt> <output> 1 <sx> <dx> <dy>" << std::endl;
	std::cout << "        JoinTiles.exe <patt> <output> 2 <sx> <sy> <dx> <dy>" << std::endl;
	std::cout << "        JoinTiles.exe <patt> <output> 2 <sx> <sy>" << std::endl;   // Auto discovery

	return;
}