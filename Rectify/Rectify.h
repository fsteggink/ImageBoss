// Rectify.h
// FS, 20-04-2007

#ifndef RECTIFY_H
#define RECTIFY_H

#include <iostream>
#include <fstream>
#include <cmath>
#include "../../Inc/ImageGeo.h"
#include "../../Inc/Matrix.h"
#include "../../Inc/StringLib.h"
#include "../../Inc/CmdApp.h"
//#include "../ECWHandler/ECWHandler.h"
#include <boost/thread/thread.hpp>


#undef INFINITE

#define showVal(val) std::cout << #val " = " << (val) << std::endl
#define show(val) #val ": " << (val)


// Constants
const std::string PROGNAME = "Rectify";
const std::string PROGVERSION = "0.1a";
const std::string WHITESPACE = " \t\r\n\f";
const double EPSILON = 1e-12;
const double EPSILON2 = 1e-24;
const double INFINITE = 1e50;
const int GRID_DIST = 10;


// Type definitions
typedef StringLib<char> SL;


// Global variables

extern int g_polygoncontains;
extern std::map<std::string, boost::shared_ptr<CoordinateSystem>> g_mapCS;

class Sheet;

template<class T, class U>
class CoordinateCache;

extern std::map<boost::shared_ptr<const Sheet>, boost::shared_ptr<CoordinateCache<CoordXY, PixelCoord>>> g_coordCaches_img;
extern std::map<boost::shared_ptr<const Sheet>, boost::shared_ptr<CoordinateCache<CoordXY, CoordXY>>> g_coordCaches_map;

// Inline function definitions

inline double atof(const std::string &value)
{
	return atof(value.c_str());
}


inline int sign(double value)
{
	return(value < 0) ? -1 : (value > 0) ? 1 : 0;
}


// Class definitions

struct Vector
{
	double x, y;
	Vector(double _x, double _y): x(_x), y(_y) {};
};


inline bool operator==(const CoordXY &A, const CoordXY &B)
{
	return A.x == B.x && A.y == B.y;
}


inline bool operator==(const PixelCoord &A, const PixelCoord &B)
{
	return A.x == B.x && A.y == B.y;
}


inline Vector operator-(const CoordXY &A, const CoordXY &B)
{
	return Vector(B.x - A.x, B.y - A.y);
}


inline Vector operator-(const PixelCoord &A, const PixelCoord &B)
{
	return Vector(B.x - A.x, B.y - A.y);
}


// Calculate the determinant of two vectors
inline double det2(const Vector &V, const Vector &W)
{
	return V.x * W.y - W.x * V.y;
}


template<typename T>
inline std::ostream &operator<<(std::ostream &str, const Box<T> &box)
{
	str << box.lower << "-" << box.upper;
	return str;
}

struct Anchor
{
	PixelCoord orig;
	CoordXY rect;
	Anchor(const PixelCoord &_orig, const CoordXY &_rect): orig(_orig), rect(_rect) {};
};

class ImageCoord
{
private:

protected:
	const std::vector<Anchor> &m_anchors;

	mutable std::vector<double> m_wParams_x, m_aParams_x, m_wParams_y, m_aParams_y;
	mutable std::vector<double> m_wInvParams_x, m_aInvParams_x, m_wInvParams_y, m_aInvParams_y;

	ImageCoord(const std::vector<Anchor> &anchors);

	void calculateTPSParams() const;

	inline double U(double r) const
	{
		return pow2(r) * log(r);
	}

	inline double U2(double r2) const
	{
		return r2 * log(r2) / 2;
	}

	inline double dist(const PixelCoord &from, const PixelCoord &to) const
	{
		return sqrt(pow2(from.x - to.x) + pow2(from.y - to.y));
	}

	inline double dist(const CoordXY &from, const CoordXY &to) const
	{
		return sqrt(pow2(from.x - to.x) + pow2(from.y - to.y));
	}

	inline double dist2(const CoordXY &from, const CoordXY &to) const
	{
		return pow2(from.x - to.x) + pow2(from.y - to.y);
	}
};

class ToImageCoord: public XYTrans<CoordXY, PixelCoord>, public ImageCoord
{
public:
	ToImageCoord(const std::vector<Anchor> &anchors);

	PixelCoord execute(const CoordXY &mapCoord) const;
};

class FromImageCoord: public XYTrans<PixelCoord, CoordXY>, public ImageCoord
{
public:
	FromImageCoord(const std::vector<Anchor> &anchors);

	CoordXY execute(const PixelCoord &imageCoord) const;
};

class ImageProjection
{
	std::vector<Anchor> m_anchors;

	boost::shared_ptr<ToImageCoord> m_tp;
	boost::shared_ptr<FromImageCoord> m_fp;

public:
	ImageProjection::ImageProjection(const std::vector<Anchor> &anchors);

	const boost::shared_ptr<ToImageCoord> &getToImageCoord() const;
	const boost::shared_ptr<FromImageCoord> &getFromImageCoord() const;

	std::vector<Anchor> &getAnchors();
};

//class ImageCS: public CoordinateSystem
//{
//
//
//public:
//	ImageCS();
//	~ImageCS();
//};

// Simple polygon, with no holes
template<typename T>
class PolyGon
{
private:
	mutable std::vector<T> ring;
	Box<T> box;

public:
	PolyGon()
	{
		box = Box<T>(T(INFINITE, INFINITE), T(-INFINITE, -INFINITE));
	}

	const std::vector<T> &getRing() const
	{
		return ring;
	}

	std::vector<T> &getRing()
	{
		return ring;
	}

	void addPoint(const T &coord)
	{
		ring.push_back(coord);
		if(coord.x < box.lower.x)
			box.lower.x = coord.x;
		if(coord.y < box.lower.y)
			box.lower.y = coord.y;
		if(coord.x > box.upper.x)
			box.upper.x = coord.x;
		if(coord.y > box.upper.y)
			box.upper.y = coord.y;

		return;
	}

	bool contains(const T &coord) const
	{
		// Check the number of points and box
		if(ring.size() < 3 || !this->box.contains(coord))
		{
			return false;
		}
		g_polygoncontains++;


		// Test intersection inside of ring

		// Initialization
		int iCount = 0;
		Vector uV(1, 0);	// This means that only the intersections to the right
							// of the given point are calculated

		// Assume that polygon rings are closed.
		bool bIsClosed = (ring.front() == ring.back());
		if(!bIsClosed)
			ring.push_back(ring.front());

		// Loop through the rings and points and calculate the intersections
		std::vector<T>::const_iterator iterPrev = ring.begin();
		std::vector<T>::const_iterator iterActive = iterPrev + 1;

		for(; iterActive != ring.end(); ++iterActive, ++iterPrev)
		{
			// Direction of the edge
			Vector uW = *iterActive - *iterPrev;

			// Position of the start of the edge as seen from the point
			Vector uS = *iterPrev - coord;

			// Vertical orientation of the edge
			int iSign = sign(uW.y);

			// If an intersection with a non-horizontal line is found, add it to the sign
			// Do this only when there is really an intersection
			if(iSign)
			{
				// Position of the intersection as seen from the point in dir v
				double t = det2(uS, uW) / det2(uV, uW);

				// Position of the intersection as seen from the edge, 0 = start, 1 = end
				double u = det2(uV, uS) / -det2(uV, uW);

				if(t > 0 && u >= 0 && u < 1)
				{
					iCount += iSign;
				}
			}
		}

		bool bResult = (iCount % 2 != 0);

		if(!bIsClosed)
			ring.pop_back();
		
		return bResult;
	}
};

class Sheet
{
	// Terminology:
	// rect, world: rectified coordinates, in certain projection
	// orig, local: image coordinates

	// Note, incoming coordinates are always in the target coordinate system!
	// (Except during the initialization phase, coming from SheetIndex.read.)

	// Bij het bepalen of een sheet een coordinaat bevat wordt eerst uitgegaan van de polygon. Indien die niet aanwezig
	// is, wordt uitgegaan van de bounding box van de _geregistreerde_ coordinaten. Niet van de map-coordinaten, omdat
	// sheets vaak niet rechthoekig zijn (bijv. ONC en USSR) als ze in WGS84/WGS72/Pulkovo zijn geregistreerd. Als ze
	// dat wel zijn, is de reg-projectie hetzelfde als de map-projectie.

	bool m_bInit;
	std::string m_name;
	mutable Box<CoordXY> m_boxReg;        // Bounding box (reg coords)
	mutable Box<CoordXY> m_boxSheet;      // Bounding box (map coords)
	Box<CoordXY> m_boxTarget;             // Bounding box (target coords)

	//--> TODO: wordt alleen gebruikt voor de contains-method. Kan net zo goed pixel coords bepalen (of is dat te duur?) en m_polygonL gebruiken
	mutable PolyGon<CoordXY> m_polygon;   // BoundingShape (map coords)
	mutable PolyGon<PixelCoord> m_polygonL;       // BoundingShape (pixel coords)
	bool m_polygonIsAdditional;

	mutable Box<PixelCoordI> m_boxClipped;

	int m_priority;


	boost::shared_ptr<RasterLayer<byte>> m_lyr;

	boost::shared_ptr<CoordinateSystem> m_mapCS, m_registeredCS;
	boost::shared_ptr<XYTrans<CoordXY, CoordXY>> m_transMapReg, m_transRegMap;
	boost::shared_ptr<ImageProjection> m_ip;

	bool m_brightnessAndContrastSet;
	bool m_colorizeSet;
	bool m_adjustRGBSet;
	int m_brightness;
	float m_contrast;
	RasterLayer<byte>::Pixel m_color;
	float m_colorOpacity;
	float m_adjustRed, m_adjustGreen, m_adjustBlue;


	//PixelCoord getOrigCoord_internal(const CoordXY &mapCoord) const;
	CoordXY getMapCoord_internal(const PixelCoord &imageCoord) const;
	bool calculateAndAnalyzeBBox() const;

	const boost::shared_ptr<RasterLayer<byte>> getLayer() const;

public:
	Sheet();
	~Sheet();

	bool init();
	void load(const RasterLayer<byte>::Pixel &px);
	void unload();
	//PixelCoord getOrigCoord(const CoordXY &mapCoord) const;
	boost::shared_ptr<const ImageProjection> getImageProjection() const;

	bool contains(const CoordXY &mapCoord) const;
	bool isOpen() const;

	void setName(const std::string &name);
	const std::string &getName() const;
	void setAnchors(const std::vector<Anchor> &anchors);
	void setPolygonL(const PolyGon<PixelCoord> &polygonL);
	void setPolygonIsAdditional(bool value);

	Box<CoordXY> getBoxSheetMap() const;
	const Box<CoordXY> &getBoxSheetTarget() const;
	void setBoxSheetTarget(const Box<CoordXY> &boxTarget);

	void setMapCS(const boost::shared_ptr<CoordinateSystem> &mapCS);
	const boost::shared_ptr<CoordinateSystem> Sheet::getMapCS() const;
	void setRegisteredCS(const boost::shared_ptr<CoordinateSystem> &registeredCS);

	void getPixel(RasterLayer<byte>::DataIter &px, const PixelCoord &pc, float decX, float decY, const RasterFilter &uFilter);

	// With regard to priority, one thing that doesn't yet work is that it is automatically detected
	// that a different sheet with a higher priority should be used while the lower priority sheet
	// is being processed.
	int getPriority() const;
	void setPriority(int newPriority);

	int getBrightness() const;
	float getContrast() const;
	void setBrightnessAndContrast(int brightness, float contrast);
	const RasterLayer<byte>::Pixel &getColor() const;
	float getColorOpacity() const;
	void setColorAndOpacity(const RasterLayer<byte>::Pixel &color, float colorOpacity);
	float adjustRed() const;
	float adjustGreen() const;
	float adjustBlue() const;
	void setAdjustRGB(float red, float green, float blue);

};

inline double ipol(double a, double b, double fract)
{
	return a * (1 - fract) + b * fract;
}

inline double ipol_2(double a, double b, double c, double d, double fractX, double fractY)
{
	return ipol(ipol(a, b, fractX), ipol(c, d, fractX), fractY);
}

inline CoordXY ipol_2(const CoordXY &a, const CoordXY &b, const CoordXY &c, const CoordXY &d,
	double fractX, double fractY)
{
	return CoordXY(
		ipol_2(a.x, b.x, c.x, d.x, fractX, fractY),
		ipol_2(a.y, b.y, c.y, d.y, fractX, fractY));	
}

inline PixelCoord ipol_2(const PixelCoord &a, const PixelCoord &b,
						 const PixelCoord &c, const PixelCoord &d,
						 double fractX, double fractY)
{
	return PixelCoord(
		ipol_2(a.x, b.x, c.x, d.x, fractX, fractY),
		ipol_2(a.y, b.y, c.y, d.y, fractX, fractY));	
}

template<class T, class U>
class CoordinateCache: public XYTrans<T, U>
{
private:
	std::vector<U> m_targetCoords;
	Box<T> m_targetBox;
	double m_distX, m_distY;
	int m_dX, m_dY;
	//mutable int m_called;

public:
	CoordinateCache(
		const boost::shared_ptr<XYTrans<T, U>> &targetToImage,
		const Box<CoordXY> &targetBox,
		double distX, double distY):
		m_targetBox(targetBox), m_distX(distX), m_distY(distY)
	{
		m_targetBox.lower.x -= m_distX;
		m_targetBox.lower.y -= m_distY;
		m_targetBox.upper.x += m_distX;
		m_targetBox.upper.y += m_distY;

		m_dX = 2 + int(m_targetBox.width() / m_distX);
		m_dY = 2 + int(m_targetBox.height() / m_distY);
		m_targetCoords.reserve(m_dX * m_dY);

		//std::cout << "Building coord cache; m_dX: " << m_dX << ", m_dY: " << m_dY << std::endl;
		//showVal(m_targetBox);

		// Vul de cache
		for(int y = 0; y < m_dY; ++y)
		{
			for(int x = 0; x < m_dX; ++x)
			{
				T c = T(m_targetBox.lower.x + x * m_distX, m_targetBox.lower.y + y * m_distY);
				U pc = targetToImage->execute(c);
				m_targetCoords.push_back(pc);
			}
		}

		//std::cout << "Coordinate cache created, box: " << m_targetBox << std::endl;
		//showVal(m_targetCoords.back());
	}

	~CoordinateCache()
	{
		//std::cout << "Coordinate cache removed, box: " << m_targetBox << std::endl;
		//std::cout << "Coordinate cache " << m_targetBox << " called: " << m_called << std::endl;
	}

	U execute(const T &input) const
	{
		if(!m_targetBox.contains(input))
		{
			std::cerr << "ERROR, target box: " << m_targetBox << std::endl;
			throw "Input not contained by cache";
		}

		//++m_called;

		double ddX = (input.x - m_targetBox.lower.x) / m_distX;
		double ddY = (input.y - m_targetBox.lower.y) / m_distY;
		int dX = int(ddX);
		int dY = int(ddY);

		const U &a = m_targetCoords[dY * m_dX + dX];
		const U &b = *(&a + 1);
		const U &c = *(&a + m_dX);
		const U &d = *(&c + 1);

		double fractX = ddX - dX;
		double fractY = ddY - dY;

		return ipol_2(a, b, c, d, fractX, fractY);
	}
};

typedef std::vector<boost::shared_ptr<Sheet>>::iterator SheetIterator;
typedef std::vector<boost::shared_ptr<Sheet>>::const_iterator ConstSheetIterator;

class SheetIndex
{
	std::vector<boost::shared_ptr<Sheet>> m_sheets;
	Box<CoordXY> reprojectSheetBox(const Box<CoordXY> &sourceBox, const Box<CoordXY> &boxTgt, const boost::shared_ptr<CoordinateSystem> &sourceCS, const boost::shared_ptr<CoordinateSystem> &targetCS);

public:
	SheetIndex();
	~SheetIndex();

	std::vector<boost::shared_ptr<Sheet>> &getSheets();
	const std::vector<boost::shared_ptr<Sheet>> &getSheets() const;
	
	std::vector<boost::shared_ptr<Sheet>>::size_type size() const;
	std::vector<boost::shared_ptr<Sheet>>::iterator begin();
	std::vector<boost::shared_ptr<Sheet>>::iterator end();
	std::vector<boost::shared_ptr<Sheet>>::const_iterator begin() const;
	std::vector<boost::shared_ptr<Sheet>>::const_iterator end() const;

	boost::shared_ptr<Sheet> &operator[](std::vector<boost::shared_ptr<Sheet>>::size_type idx);
	const boost::shared_ptr<Sheet> &operator[](std::vector<boost::shared_ptr<Sheet>>::size_type idx) const;
	
	void readGeneral(std::ifstream &fIn);
	void read(std::ifstream &fIn, const boost::shared_ptr<CoordinateSystem> &cs, const Box<CoordXY> &targetBox);

	void matchBox(const Box<CoordXY> &box, const boost::shared_ptr<CoordinateSystem> &cs);
	void loadSheets(SheetIndex &feederSheets, const Box<CoordXY> &box, int y, double rectY, const boost::shared_ptr<CoordinateSystem> &cs);
	void unloadSheets(const Box<CoordXY> &box, int y, double rectY, const boost::shared_ptr<CoordinateSystem> &cs);
	ConstSheetIterator findActiveSheet(const CoordXY &rectCoord, const boost::shared_ptr<CoordinateSystem> &cs) const;
};

struct ContextData
{
	int width, height;
	double resx, resy;
	RasterFilter *oFilter;
	Box<CoordXY> box;
	SheetIndex sheetIndex;
};


// Function declarations
RasterImage CreateBitmap(const boost::shared_ptr<CoordinateSystem> &cs);
boost::shared_ptr<XYTrans<CoordXY, CoordXY>> CreateTransformation(const boost::shared_ptr<CoordinateSystem> &cs1, const boost::shared_ptr<CoordinateSystem> &cs2);
void CreateImage(const std::string &outputFile, const boost::shared_ptr<CoordinateSystem> &cs);
void CreateImage_scanline(const std::string &outputFile, const boost::shared_ptr<CoordinateSystem> &cs);
void LoadAndReadLine(RasterLayer<byte>::Scanline &sl, int y, const boost::shared_ptr<CoordinateSystem> &cs);
CoordXY normalizeLon(const CoordXY &coord, double normLon);

ConstSheetIterator getActiveSheet(
	const SheetIndex &activeSheets,
	const CoordXY &r,
	const boost::shared_ptr<CoordinateSystem> &cs);

void ReadLine(RasterLayer<byte>::Scanline &sl, int start, int end, int y, const SheetIndex &activeSheets, const ImageMatrix &im, const boost::shared_ptr<CoordinateSystem> &cs);

#endif // RECTIFY_H
