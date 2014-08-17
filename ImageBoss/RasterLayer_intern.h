// RasterLayer_intern.h
// FS, 28-01-2009

#ifndef RASTERLAYER_INTERN_H
#define RASTERLAYER_INTERN_H

#define DLLAPI_IMAGEBOSS __declspec(dllexport)
#define TEMPLATE_SPEC template

#include "ImageBoss_internal.h"


typedef unsigned char byte;

template<typename T>
inline T interpolate(T a, T b, double f)
{
	return static_cast<T>(a * (1.0 - f) + b * f + 0.5);
}


template<typename T>
inline T interpolate(T a, T b, float f)
{
	return static_cast<T>(a * (1.0f - f) + b * f + 0.5f);
}


template<typename T>
class RasterLayer: public IRasterLayer
{
public:
	typedef std::vector<T> Pixel;
	typedef std::vector<T> Scanline;
	typedef RasterLayer<T> Tile;
	typedef std::vector<T> Palette;

	typedef typename std::vector<T>::iterator DataIter;
	typedef typename std::vector<T>::const_iterator CDataIter;

private:
	unsigned int m_iWidth, m_iHeight, m_iSPP;
	boost::shared_array<T> m_vecData;
	Pixel m_pxBackground;
	Palette m_palPalette;
	int m_iMinT, m_iMaxT;

	//// Returns offset in data
	//int operator()(int x, int y) const;
	//int operator()(int y) const;
	//int operator()(const PixelCoordI &pc) const;

	//template<int N>
	//void blur(RasterFilter &filter, int radius);

public:
	RasterLayer(unsigned int width, unsigned int height, unsigned int spp): \
		m_iWidth(width), m_iHeight(height), m_iSPP(spp)
	{
		m_iMinT = 0;
		m_iMaxT = (1 << sizeof(T) * 8) - 1;
		if(m_iMinT > m_iMaxT)
		{
			m_iMaxT >>= 1;
			m_iMinT = ~m_iMaxT;
		}

		m_pxBackground = Pixel(spp);
		memset(&m_pxBackground[0], 0, spp * sizeof(T));

		m_vecData = boost::shared_array<T>(new T[this->get_ScanlineWidth() * height]);
	}


	RasterLayer(unsigned int width, unsigned int height, unsigned int spp, const Pixel &pxBackground): \
		m_iWidth(width), m_iHeight(height), m_iSPP(spp)
	{
		m_iMinT = 0;
		m_iMaxT = (1 << sizeof(T) * 8) - 1;
		if(m_iMinT > m_iMaxT)
		{
			m_iMaxT >>= 1;
			m_iMinT = ~m_iMaxT;
		}

		m_vecData = boost::shared_array<T>(new T[this->get_ScanlineWidth() * height]);

		this->setBackground(pxBackground);
		this->clear();

		m_pxBackground = pxBackground;
	}


	~RasterLayer()
	{
	}


	RasterLayer clone() const
	{
		RasterLayer<T> tile(m_iWidth, m_iHeight, m_iSPP, m_pxBackground);

		if(this->hasPalette())
		{
			tile.m_palPalette.reserve(m_palPalette.size());
			memcpy(&tile.m_palPalette[0], &m_palPalette[0], m_palPalette.size() * sizeof(T));
		}

		this->getTile(tile, PixelCoordI(0, 0));

		return tile;
	}


	// Returns offset in data (private)
	ptrdiff_t operator()(unsigned int x, unsigned int y) const
	{
		return y * this->get_ScanlineWidth() + x * m_iSPP;
	}


	// Returns offset in data (private)
	ptrdiff_t operator()(unsigned int y) const
	{
		return y * this->get_ScanlineWidth();
	}


	// Returns offset in data (private)
	ptrdiff_t operator()(const PixelCoordI &pc) const
	{
		return pc.y * this->get_ScanlineWidth() + pc.x * m_iSPP;
	}


	unsigned int get_Width() const
	{
		return m_iWidth;
	}


	unsigned int get_Height() const
	{
		return m_iHeight;
	}


	unsigned int get_SamplesPerPixel() const
	{
		return m_iSPP;
	}


	unsigned int get_BitsPerSample() const
	{
		return sizeof(T) * 8;
	}


	unsigned int get_ScanlineWidth() const
	{
		return m_iWidth * m_iSPP;
	}


	unsigned int get_ScanlineWidth_B() const
	{
		return this->get_ScanlineWidth() * sizeof(T);
	}


	unsigned int get_PixelSize_B() const
	{
		return m_iSPP * sizeof(T);
	}


	void setBackground(const Pixel &px)
	{
		if(px.size() != m_iSPP)
			throw ImageBossException(EXC_INVALID_SAMPLE_COUNT);

		m_pxBackground = px;

		return;
	}


	void getBackground(Pixel &px) const
	{
		if(px.size() != m_iSPP)
			throw ImageBossException(EXC_INVALID_SAMPLE_COUNT);

		px = m_pxBackground;

		return;
	}


	void clear()
	{
		// Prepare scanline for clearing data
		Scanline scanline(this->get_ScanlineWidth());
		for(unsigned int x = 0; x < m_iWidth; ++x)
		{
			memcpy(&scanline[x * m_iSPP], &m_pxBackground[0], this->get_PixelSize_B());
		}
		
		// Clear data with given background color
		for(unsigned int y = 0; y < m_iHeight; ++y)
		{
			memcpy(&m_vecData[(*this)(y)], &scanline[0], this->get_ScanlineWidth_B());
		}

		return;
	}


	void setPixel(CDataIter &px, const PixelCoordI &pc)
	{
		memcpy(&m_vecData[(*this)(pc)], &*px, this->get_PixelSize_B());

		return;
	}


	void setPixel(CDataIter &px, unsigned int x, unsigned int y)
	{
		memcpy(&m_vecData[(*this)(x, y)], &*px, this->get_PixelSize_B());

		return;
	}


	void getPixel(DataIter &px, const PixelCoordI &pc) const
	{
		if(this->hasPalette())
		{
			memcpy(&*px, &m_palPalette.begin()[m_vecData[(*this)(pc)] * 3], 3);
		}
		else
		{
			memcpy(&*px, &m_vecData[(*this)(pc)], this->get_PixelSize_B());
		}

		return;
	}


	void getPixel(DataIter &px, unsigned int x, unsigned int y) const
	{
		if(this->hasPalette())
		{
			memcpy(&*px, &m_palPalette.begin()[m_vecData[(*this)(x, y)] * 3], 3);
		}
		else
		{
			memcpy(&*px, &m_vecData[(*this)(x, y)], this->get_PixelSize_B());
		}

		return;
	}


	// This method returns a pixel. A filter is used to get the pixel
	// The parameters dDecX and dDecY specify the (approximate) decimation factor; used for downsampling
	// If the decimation factor is lower than 1, then the filter is used for interpolation
	void getPixel_filtered(DataIter &px, const PixelCoord &pc, float dDecX, float dDecY, const RasterFilter &uFilter, BorderPixelBehavior ePixBeh = BorderPixelBehavior::UseBackgroundColor) const
	{
		//std::cout << "In getPixel_filtered" << std::endl;
		float dIX, dIY;
		float phiX = modf(static_cast<float>(pc.x), &dIX) - 0.5f;
		float phiY = modf(static_cast<float>(pc.y), &dIY) - 0.5f;
		//std::cout << "a" << std::endl;

		static FilterCoeffs coeffsX, coeffsY;
		uFilter.getNormalizedCoeffs(coeffsX, phiX, dDecX);
		uFilter.getNormalizedCoeffs(coeffsY, phiY, dDecY);
		//std::cout << "b" << std::endl;

		unsigned int sizeX = static_cast<int>(coeffsX.coeffs.size());
		unsigned int sizeY = static_cast<int>(coeffsY.coeffs.size());
		int iX = static_cast<int>(dIX) + coeffsX.min;
		int iY = static_cast<int>(dIY) + coeffsY.min;
		//std::cout << "c" << std::endl;


		// Iterate in both dimensions over the coefficients and calculate new pixel
		unsigned int slw = this->get_ScanlineWidth();
		bool bHasPalette = this->hasPalette();
		//const T *dataStart = &m_vecData[(*this)(iX, iY)];
		//std::cout << "d" << std::endl;

		unsigned int fromX = std::max(0, -iX);
		unsigned int fromY = std::max(0, -iY);
		unsigned int toX = std::min(sizeX, m_iWidth - iX);
		unsigned int toY = std::min(sizeY, m_iHeight - iY);
		//dataStart += slw * fromY + m_iSPP * fromX;
		const T *dataStart = &m_vecData[(*this)(iX + fromX, iY + fromY)];
		//std::cout << "e" << std::endl;

		/*static*/ float rgb_arr[5];   // Last entry is sum of coeffs product, fourth entry is alpha (if present)
		memset(rgb_arr, 0, 5 * sizeof(float));
		float *cX = &coeffsX.coeffs[0];
		float *cY = &coeffsY.coeffs[0];
		//std::cout << "f" << std::endl;

		unsigned int maxK = (bHasPalette) ? 3 : m_iSPP;
		T pixBuf[4];

		for(unsigned int y = fromY; y < toY; ++y, dataStart += slw)
		{
			const T *data = dataStart;
			float cy = cY[y];
			for(unsigned int x = fromX; x < toX; ++x, data += m_iSPP)
			{
				float xy = cX[x] * cy;
				const T *pt = (bHasPalette) ? &m_palPalette[*data * 3] : data;

				for(unsigned int k = 0; k < maxK; ++k)
				{
					rgb_arr[k] += xy * static_cast<float>(pt[k]);
				}
				rgb_arr[4] += xy;
			}
		}
		//std::cout << "g" << std::endl;
		//std::cout << rgb_arr[0] << ", " << rgb_arr[1] << ", " << rgb_arr[2] << ", " << rgb_arr[3] << ", " << rgb_arr[4] << ", spp: " << m_iSPP << std::endl;

		if (ePixBeh == BorderPixelBehavior::UseCurrentColor)
		{
			// Use only current color for the pixel
			for(unsigned int k = 0; k < maxK; ++k)
			{
				pixBuf[k] = Bounds(static_cast<int>(rgb_arr[k]/rgb_arr[4] + 0.5f), m_iMinT, m_iMaxT);
			}
		}
		else
		{
			// Use the background color to 'complete' the pixel
			const T *bg = (bHasPalette) ? &m_palPalette[m_pxBackground[0] * 3] : &m_pxBackground[0];
			for(unsigned int k = 0; k < maxK; ++k)
			{
				rgb_arr[k] += bg[k] * (1.0f - rgb_arr[4]);
				//std::cout << k << std::endl;
				pixBuf[k] = Bounds(static_cast<int>(rgb_arr[k] + 0.5f), m_iMinT, m_iMaxT);
			}
			//std::cout << "h" << std::endl;
		}

		// Process alpha
		//std::cout << int(px[0]) << ", " << int(px[1]) << ", " << int(px[2]) << ", " << int(px[3]) << ", " << int(px[4]) << std::endl;

		if(m_iSPP > 3 && pixBuf[3] < m_iMaxT)
		{
			//std::cout << "ipol" << std::endl;
			for(unsigned int k = 0; k < 3; ++k)
			{
				float fract = pixBuf[3] / float(m_iMaxT);
				//a * (1 - fract) + b * fract
				px[k] = Bounds(static_cast<int>(pixBuf[k] * fract + m_pxBackground[k] * (1 - fract)), m_iMinT, m_iMaxT);
			}
		}
		else
		{
			for(unsigned int k = 0; k < 3; ++k)
			{
				px[k] = pixBuf[k];
			}
		}
		//std::cout << int(px[0]) << ", " << int(px[1]) << ", " << int(px[2]) << ", " << int(px[3]) << ", " << int(px[4]) << std::endl;

		return;
	}


	void setScanline(CDataIter &scanline, unsigned int y)
	{
		memcpy(&m_vecData[(*this)(y)], &*scanline, this->get_ScanlineWidth_B());

		return;
	}


	void getScanline(DataIter &scanline, unsigned int y) const
	{
		memcpy(&*scanline, &m_vecData[(*this)(y)], this->get_ScanlineWidth_B());

		return;
	}


	void setTile(const Tile &tile, const PixelCoordI &pc)
	{
		// No bounds check necessary

		// Offset in current layer
		unsigned int minx = std::max(0, pc.x);
		unsigned int miny = std::max(0, pc.y);
		unsigned int maxx = std::min(m_iWidth, pc.x + tile.get_Width());
		unsigned int maxy = std::min(m_iHeight, pc.y + tile.get_Height());

		// Offset in tile
		unsigned int dx = std::max(0, -pc.x);
		unsigned int dy = std::max(0, -pc.y);

		// Check for overlap
		if(minx < maxx && miny < maxy)
		{
			unsigned int bufferWidth_B = (maxx - minx) * this->get_PixelSize_B();

			for(unsigned int y = miny; y < maxy; ++y)
			{
				const T *src = &tile.m_vecData[tile(dx, y - miny + dy)];
				T *dest = &m_vecData[(*this)(minx, y)];
				memcpy(dest, src, bufferWidth_B);
			}
		}

		return;
	}


	void getTile(Tile &tile, const PixelCoordI &pc) const
	{
		// No bounds check necessary

		//RasterLayer tile(width, height, m_iSPP);
		if(tile.get_SamplesPerPixel() != m_iSPP)
			throw ImageBossException(EXC_BUFFER_INCOMPATIBLE); 

		if(this->hasPalette())
		{
			Palette palette;
			this->getPalette(palette);
			tile.setPalette(palette);
		}

		unsigned int width = tile.get_Width();
		unsigned int height = tile.get_Height();

		// Offset in current layer
		unsigned int minx = std::max(0, pc.x);
		unsigned int miny = std::max(0, pc.y);
		unsigned int maxx = std::min(m_iWidth, pc.x + width);
		unsigned int maxy = std::min(m_iHeight, pc.y + height);

		// Offset in tile
		unsigned int dx = std::max(0, -pc.x);
		unsigned int dy = std::max(0, -pc.y);

		// Check for overlap
		if(minx < maxx && miny < maxy)
		{
			unsigned int bufferWidth_B = (maxx - minx) * this->get_PixelSize_B();

			for(unsigned int y = miny; y < maxy; ++y)
			{
				const T *src = &m_vecData[(*this)(minx, y)];
				T *dest = &tile.m_vecData[tile(dx, y - miny + dy)];
				memcpy(dest, src, bufferWidth_B);
			}
		}

		return;
	}


	const T * const data(unsigned int x, unsigned int y) const
	{
		return &m_vecData[(*this)(x, y)];
	}


	const T *data(unsigned int x, unsigned int y)
	{
		return &m_vecData[(*this)(x, y)];
	}


	inline bool hasPalette() const
	{
		return m_palPalette.size() > 0;
	}


	void setPalette(const Palette &palette)
	{
		m_palPalette = palette;
		return;
	}


	void getPalette(Palette &palette) const
	{
		palette = m_palPalette;
		return;
	}


	void depalettize()
	{
		if(!hasPalette())
			return;

		//char state = 'a';

		std::cout << "Depalettizing, image size: " << m_iWidth << " x " << m_iHeight << ", palette size: " << m_palPalette.size() << std::endl;
		T* t;

		try
		{
			t = new T[m_iWidth * m_iHeight * 3];
			boost::shared_array<T> newData(t);
			//state = 'b';
		
			T *src = m_vecData.get();
			//state = 'c';
			T *dest = newData.get();
			//state = 'd';

			for(unsigned int y = 0; y < m_iHeight; ++y)
			{
				for(unsigned int x = 0; x < m_iWidth; ++x, ++src, dest += 3)
				{
					memcpy(dest, &m_palPalette[*src * 3], 3);
				}
			}
			//state = 'e';

			m_vecData.swap(newData);
			//state = 'f';
			newData.reset();
			//state = 'g';
			m_palPalette.clear();
			//state = 'h';
			m_iSPP = 3;
			//state = 'i';
		}
		catch(std::bad_alloc)
		{
			delete[] t;
			std::cerr << "Memory exhausted while depalettizing" << std::endl;
			throw;
		}
		//catch(...)
		//{
		//	std::cerr << "Error while depalettizing, state: " << state << std::endl;
		//}

		return;
	}


	// Invert the current raster layer
	void invert()
	{
		if(this->hasPalette())
		{
			// Invert palette
			for(Palette::iterator iterPal = m_palPalette.begin();
				iterPal != m_palPalette.end(); ++iterPal)
			{
				*iterPal = ~*iterPal;
			}
		}
		else
		{
			// Invert pixel values
			__int64 size = m_iWidth * m_iHeight * m_iSPP;
			for(ptrdiff_t i = 0; i < size; ++i)
			{
				m_vecData[i] = ~m_vecData[i];
			}
		}

		return;
	}


	// Merge another raster layer into the current layer with the given opacity
	void merge(const RasterLayer<T> &layer, float opacity)
	{
		if(m_palPalette.size() > 0)
			throw "Can\'t merge another layer with a paletted layer";

		opacity = Bounds(opacity, 0.0f, 1.0f);

		int maxX = std::min(get_Width(), layer.get_Width());
		int maxY = std::min(get_Height(), layer.get_Height());

		if(layer.hasPalette())
		{
			if(get_SamplesPerPixel() != 3)
				throw "Can\'t merge another layer because of incorrect sample size";

			for(int y = 0; y < maxY; ++y)
			{
				for(int x = 0; x < maxX; ++x)
				{
					ptrdiff_t idxA = this->operator()(x, y);
					ptrdiff_t idxB = layer.operator()(x, y);
					T val = layer.m_vecData[idxB];

					for(int k = 0; k < 3; ++k)
					{
						m_vecData[idxA + k] = interpolate(m_vecData[idxA + k], layer.m_palPalette[val * 3 + k], opacity);
					}

				}
			}
		}
		else
		{
			if(get_SamplesPerPixel() != layer.get_SamplesPerPixel())
				throw "Can\'t merge another layer because of incorrect sample size";

			for(int y = 0; y < maxY; ++y)
			{
				for(int x = 0; x < maxX; ++x)
				{
					ptrdiff_t idxA = this->operator()(x, y);
					ptrdiff_t idxB = layer.operator()(x, y);
					for(unsigned int k = 0; k < m_iSPP; ++k)
					{
						m_vecData[idxA + k] = interpolate(m_vecData[idxA + k], layer.m_vecData[idxB + k], opacity);
					}
				}
			}
		}

		return;
	}


	// Blur current layer with the given filter, with given radius
	template<int N>
	void blur(RasterFilter &filter, int radius)
	{
		RasterLayer<T> layer = this->clone();

		static FilterCoeffs coeffsX, coeffsY;
		filter.getNormalizedCoeffs(coeffsX, 0.0f, static_cast<float>(radius));
		filter.getNormalizedCoeffs(coeffsY, 0.0f, static_cast<float>(radius));

		size_t sizeX = coeffsX.coeffs.size();
		size_t sizeY = coeffsY.coeffs.size();

		// Max value in filter: 4096
		std::vector<int> intFilter(sizeX * sizeY);
		for(unsigned int yy = 0; yy < sizeY; ++yy)
		{
			for(unsigned int xx = 0; xx < sizeX; ++xx)
			{
				intFilter[yy * sizeX + xx] = (static_cast<int>(coeffsX.coeffs[xx] * coeffsY.coeffs[yy] * 4096 + 0.5));
			}
		}

		unsigned int slw = layer.get_ScanlineWidth();
		bool bHasPalette = this->hasPalette();

		T *px = &this->m_vecData[0];

		for(unsigned int y = 0; y < m_iHeight; ++y)
		{
			unsigned int fromY = std::max(0, -static_cast<int>(y + coeffsY.min));
			unsigned int toY = std::min(static_cast<int>(sizeY), static_cast<int>(m_iHeight) - static_cast<int>(y + coeffsY.min));

			for(unsigned int x = 0; x < m_iWidth; ++x, px += N)
			{
				unsigned int fromX = std::max(0, -static_cast<int>(x + coeffsX.min));
				unsigned int toX = std::min(static_cast<int>(sizeX), static_cast<int>(m_iWidth) - static_cast<int>(x + coeffsX.min));

				int rgb_arr[N] = {0};
				const T *dataStart = &layer.m_vecData[(*this)(x + coeffsX.min + fromX, y + coeffsY.min + fromY)];
				int *pFilter = &intFilter[fromX];

				for(unsigned int yy = fromY; yy < toY; ++yy, dataStart += slw)
				{
					const T *data = dataStart;
					for(unsigned int xx = fromX; xx < toX; ++xx, ++pFilter)
					{
						int &xy = *pFilter;

						for(int k = 0; k < N; ++k, ++data)
						{
							rgb_arr[k] += xy * *data;
						}
					}
					pFilter += fromX;
				}

				for(int k = 0; k < N; ++k)
				{
					px[k] = rgb_arr[k] >> 12;
				}
			}
		}

		return;
	}

	// Blur current layer with tent filter, with given radius
	void tentBlur(int radius)
	{
		TentFilter filter;

		switch(m_iSPP)
		{
		case 1:
			blur<1>(filter, radius);
			return;

		case 3:
			blur<3>(filter, radius);
			return;

		default:
			throw "Tent blur not implemented for this pixel depth!";
		}
	}

	// Blur current layer with gaussian filter, with given radius
	template<int N>
	void gaussianBlur(int radius)
	{
		GaussianFilter<N> filter;

		switch(m_iSPP)
		{
		case 1:
			blur<1>(filter, radius);
			return;

		case 3:
			blur<3>(filter, radius);
			return;

		default:
			throw "Gaussian blur not implemented for this pixel depth!";
		}

		return;
	}

	// Blur current layer with lanczos filter, with given radius
	template<int N>
	void lanczosBlur(int radius)
	{
		LanczosFilter<N> filter;

		switch(m_iSPP)
		{
		case 1:
			blur<1>(filter, radius);
			return;

		case 3:
			blur<3>(filter, radius);
			return;

		default:
			throw "Lanczos blur not implemented for this pixel depth!";
		}

		return;
	}

	// Change brightness and contrast of current layer
	// Brightness: value from -max to max (-255 to 255)
	// Contrast: value from -1.0 to 1.0
	void brightnessAndContrast(int brightness, float contrast)
	{
		brightness = static_cast<int>(Bounds(brightness, -m_iMaxT, m_iMaxT));
		contrast = Bounds(contrast, -0.999999f, 0.999999f);
		T iAvgT = (m_iMinT >> 1) + (m_iMaxT >> 1);

		float multiplier = 1.0f / (1.0f - contrast);

		if(this->hasPalette())
		{
			for(Palette::iterator iterPal = m_palPalette.begin();
				iterPal != m_palPalette.end(); ++iterPal)
			{
				float temp = *iterPal;
				temp += brightness;

				if(contrast >= 0)
					temp = (temp - iAvgT) * multiplier + iAvgT;
				else
					temp = interpolate(temp, static_cast<float>(iAvgT), -contrast);

				*iterPal = Bounds(static_cast<int>(temp), m_iMinT, m_iMaxT);
			}
		}
		else
		{
			for(unsigned int y = 0; y < m_iHeight; ++y)
			{
				for(unsigned int x = 0; x < m_iWidth; ++x)
				{
					ptrdiff_t idxA = this->operator()(x, y);
					for(unsigned int k = 0; k < m_iSPP; ++k)
					{
						float temp = m_vecData[idxA + k];
						temp += brightness;

						if(contrast >= 0)
							temp = (temp - iAvgT) * multiplier + iAvgT;
						else
							temp = interpolate(temp, static_cast<float>(iAvgT), -contrast);

						m_vecData[idxA + k] = Bounds(static_cast<int>(temp), m_iMinT, m_iMaxT);
					}
				}
			}
		}

		return;
	}


	void adjustRGB(float red, float green, float blue)
	{
		red = Bounds(red, -1.0f, 1.0f);
		green = Bounds(green, -1.0f, 1.0f);
		blue = Bounds(blue, -1.0f, 1.0f);

		if(this->hasPalette())
		{
			size_t size = m_palPalette.size() / 3;
			for(unsigned int idx = 0; idx < size; ++idx)
			{
				m_palPalette[idx * 3 + 0] = Bounds(static_cast<int>(m_palPalette[idx * 3 + 0] * (1.0f + red  )), m_iMinT, m_iMaxT);
				m_palPalette[idx * 3 + 1] = Bounds(static_cast<int>(m_palPalette[idx * 3 + 1] * (1.0f + green)), m_iMinT, m_iMaxT);
				m_palPalette[idx * 3 + 2] = Bounds(static_cast<int>(m_palPalette[idx * 3 + 2] * (1.0f + blue )), m_iMinT, m_iMaxT);
			}
		}
		else
		{
			for(unsigned int y = 0; y < m_iHeight; ++y)
			{
				for(unsigned int x = 0; x < m_iWidth; ++x)
				{
					ptrdiff_t idxA = this->operator()(x, y);

					m_vecData[idxA + 0] = Bounds(static_cast<int>(m_vecData[idxA + 0] * (1.0f + red  )), m_iMinT, m_iMaxT);
					m_vecData[idxA + 1] = Bounds(static_cast<int>(m_vecData[idxA + 1] * (1.0f + green)), m_iMinT, m_iMaxT);
					m_vecData[idxA + 2] = Bounds(static_cast<int>(m_vecData[idxA + 2] * (1.0f + blue )), m_iMinT, m_iMaxT);
				}
			}
		}

		return;
	}


	void greyscale()
	{
		// TODO: palette: change colours

		if(m_iSPP == 1)
			return;

		RasterLayer<T> layer = this->clone();

		m_iSPP = 1;
		m_pxBackground = Pixel(m_iSPP);
		memset(&m_pxBackground[0], 0, m_iSPP * sizeof(T));

		m_vecData = boost::shared_array<T>(new T[this->get_ScanlineWidth() * m_iHeight]);

		for(unsigned int y = 0; y < m_iHeight; ++y)
		{
			for(unsigned int x = 0; x < m_iWidth; ++x)
			{
				ptrdiff_t idx = this->operator()(x, y);
				ptrdiff_t idxL = layer(x, y);

				m_vecData[idx] = static_cast<T>(
					layer.m_vecData[idxL + 0] * 0.299 +
					layer.m_vecData[idxL + 1] * 0.587 +
					layer.m_vecData[idxL + 2] * 0.114 + 0.5);
			}
		}

		return;
	}

	/*
	void resize(unsigned int newWidth, unsigned int newHeight)
	{
		RasterLayer<T> layer = this->clone();
		float factorX = static_cast<float>(m_iWidth) / static_cast<float>(newWidth);
		float factorY = static_cast<float>(m_iHeight) / static_cast<float>(newHeight);

		m_iWidth = newWidth;
		m_iHeight = newHeight;
		m_vecData = boost::shared_array<T>(new T[this->get_ScanlineWidth() * m_iHeight]);

		unsigned int slw = this->get_ScanlineWidth();
		Scanline sl(slw);

		if(this->hasPalette())
		{
			for(unsigned int y = 0; y < m_iHeight; ++y)
			{
				unsigned int yy = static_cast<int>(y * factorY);

				for(unsigned int x = 0; x < m_iWidth; ++x)
				{
					unsigned int xx = static_cast<int>(x * factorX);
					layer.getPixel(sl.begin() + x * m_iSPP, xx, yy);
				}

				memcpy(&m_vecData[y * slw], &sl[0], slw * sizeof(T));
			}
		}
		else
		{
			TentFilter filter;

			for(unsigned int y = 0; y < m_iHeight; ++y)
			{

				for(unsigned int x = 0; x < m_iWidth; ++x)
				{
					layer.getPixel_filtered(sl.begin() + x * m_iSPP, PixelCoord(x * factorX, y * factorY), factorX, factorY, filter);
				}

				memcpy(&m_vecData[y * slw], &sl[0], slw * sizeof(T));
			}
		}

		return;
	}
	*/

	void resize(unsigned int newWidth, unsigned int newHeight)
	{
		if(hasPalette())   // effe niet...
			return;

		float factorX = static_cast<float>(m_iWidth) / static_cast<float>(newWidth);
		float factorY = static_cast<float>(m_iHeight) / static_cast<float>(newHeight);

		boost::shared_array<T> newData(new T[newWidth * newHeight * m_iSPP]);
		
		TentFilter filter;
		T *dest = newData.get();
		Pixel px;
		px.resize(m_iSPP);

		for(unsigned int y = 0; y < newHeight; ++y)
		{

			for(unsigned int x = 0; x < newWidth; ++x, dest += m_iSPP)
			{
				//layer.getPixel_filtered(sl.begin() + x * m_iSPP, PixelCoord(x * factorX, y * factorY), factorX, factorY, filter);
				this->getPixel_filtered(px.begin(), PixelCoord(x * factorX, y * factorY), factorX, factorY, filter);
				memcpy(dest, &px[0], m_iSPP * sizeof(T));
			}
		}

		m_vecData.swap(newData);
		newData.reset();
		m_iWidth = newWidth;
		m_iHeight = newHeight;

		return;
	}

};


TEMPLATE_SPEC class DLLAPI_IMAGEBOSS RasterLayer<byte>;


class DLLAPI_IMAGEBOSS RasterImageReader
{
	int m_width, m_height, m_spp;
	boost::shared_ptr<IRasterLoadByLinePlugin> m_ifPlugin;

public:
	IContext *Open(const std::string &fileName);
	IContext *Open(std::istream &input, const MimeType &mimeType);
	void ReadScanline(IContext *context, RasterLayer<byte>::Scanline &scanline);
	void Close(IContext *context);

	int get_Width() const;
	int get_Height() const;
	int get_SamplesPerPixel() const;

	int get_ScanlineWidth() const;
};


class DLLAPI_IMAGEBOSS RasterImageWriter
{
	int m_width, m_height, m_spp;
	boost::shared_ptr<IRasterSaveByLinePlugin> m_ifPlugin;

public:
	RasterImageWriter();

	IContext *Open(const std::string &fileName, int width, int height, int spp);
	IContext *Open(std::ostream &output, const MimeType &mimeType, int width, int height, int spp);
	void WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanline);
	void Close(IContext *context);

	int get_ScanlineWidth() const;
};


class DLLAPI_IMAGEBOSS IRasterLoadByLinePlugin: public IRasterPlugin
{
public:
	virtual ~IRasterLoadByLinePlugin() = 0;

	virtual FileMode CanLoad(const MimeType &mimeType) = 0;
	virtual IContext *Open(const std::string &fileName) = 0;
	virtual IContext *Open(std::istream &input) = 0;
	virtual void ReadScanline(IContext *context, RasterLayer<byte>::Scanline &scanline) = 0;
	virtual void Close(IContext *&context) = 0;

	virtual int GetWidth(IContext *context) const = 0;
	virtual int GetHeight(IContext *context) const = 0;
	virtual int GetSamplesPerPixel(IContext *context) const = 0;
	virtual bool HasPalette(IContext *context) const = 0;
	virtual const RasterLayer<byte>::Palette &GetPalette(IContext *context) const = 0;
};


class DLLAPI_IMAGEBOSS IRasterSaveByLinePlugin: public IRasterPlugin
{
public:
	virtual ~IRasterSaveByLinePlugin() = 0;

	virtual FileMode CanSave(const MimeType &mimeType) = 0;
	virtual IContext *Open(const std::string &fileName, int width, int height, int spp) = 0;
	virtual IContext *Open(std::ostream &output, int width, int height, int spp) = 0;
	virtual void WriteScanline(IContext *context, const RasterLayer<byte>::Scanline &scanline) = 0;
	virtual void Close(IContext *&context) = 0;
};


#endif // RASTERLAYER_INTERN_H
