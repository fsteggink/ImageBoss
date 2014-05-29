// CoordinateCache.cpp
// FS, 25-12-2011

#include "Rectify.h"

//
//inline double ipol(double a, double b, double fract)
//{
//	return a * (1 - fract) + b * fract;
//}
//
//inline double ipol_2(double a, double b, double c, double d, double fractX, double fractY)
//{
//	return ipol(ipol(a, b, fractX), ipol(c, d, fractX), fractY);
//}
//
//inline CoordXY ipol_2(const CoordXY &a, const CoordXY &b, const CoordXY &c, const CoordXY &d,
//	double fractX, double fractY)
//{
//	return CoordXY(
//		ipol_2(a.x, b.x, c.x, d.x, fractX, fractY),
//		ipol_2(a.y, b.y, c.y, d.y, fractX, fractY));	
//}
//
//inline PixelCoord ipol_2(const PixelCoord &a, const PixelCoord &b,
//						 const PixelCoord &c, const PixelCoord &d,
//						 double fractX, double fractY)
//{
//	return PixelCoord(
//		ipol_2(a.x, b.x, c.x, d.x, fractX, fractY),
//		ipol_2(a.y, b.y, c.y, d.y, fractX, fractY));	
//}
//
//
//CoordinateCache::CoordinateCache(
//	const boost::shared_ptr<XYTrans<CoordXY, CoordXY>> &targetToMap,
//	const boost::shared_ptr<XYTrans<CoordXY, PixelCoord>> &mapToImage,
//	const Box<CoordXY> &targetBox,
//	double distX, double distY):
//	m_targetBox(targetBox), m_distX(distX), m_distY(distY)
//{
//	boost::shared_ptr<XYTrans<CoordXY, PixelCoord>> targetToImage =
//		boost::shared_ptr<XYTrans<CoordXY, PixelCoord>>(new TransChain<CoordXY, PixelCoord, CoordXY>(targetToMap, mapToImage));
//	m_dX = 1 + int(targetBox.width() / distX);
//	m_dY = 1 + int(targetBox.height() / distY);
//
//	// Vul de cache
//	for(int y = 0; y <= m_dY; ++y)
//	{
//		for(int x = 0; x <= m_dX; ++x)
//		{
//			CoordXY c = CoordXY(targetBox.lower.x + x * distX, targetBox.lower.y + y * distY);
//			//showVal(c);
//			PixelCoord pc = targetToImage->execute(c);
//			//showVal(pc);
//			m_targetCoords.push_back(pc);
//		}
//	}
//}
//
//PixelCoord CoordinateCache::execute(const CoordXY &input) const
//{
//	if(!m_targetBox.contains(input))
//		throw "Input not contained by cache";
//
//	double ddX = (input.x - m_targetBox.lower.x) / m_distX;
//	double ddY = (input.y - m_targetBox.lower.y) / m_distY;
//	int dX = int(ddX);
//	int dY = int(ddY);
//
//	const PixelCoord &a = m_targetCoords[dY * m_dX + dX];
//	const PixelCoord &b = m_targetCoords[dY * m_dX + dX + 1];
//	const PixelCoord &c = m_targetCoords[(dY + 1) * m_dX + dX];
//	const PixelCoord &d = m_targetCoords[(dY + 1) * m_dX + dX + 1];
//
//	double fractX = ddX - dX;
//	double fractY = ddY - dY;
//
//	return ipol_2(a, b, c, d, fractX, fractY);
//}
