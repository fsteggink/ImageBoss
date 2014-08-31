// Sheet.cpp
// FS, 22-04-2007

#include "Rectify.h"


Sheet::Sheet(): m_bInit(false), m_priority(0), m_brightnessAndContrastSet(false), m_colorizeSet(false),
	m_brightness(0), m_contrast(0.0f), m_color(RasterLayer<byte>::Pixel(3)), m_colorOpacity(0.0f),
	m_adjustRGBSet(false), m_adjustRed(0.0f), m_adjustGreen(0.0f), m_adjustBlue(0.0f)
{
}


Sheet::~Sheet()
{
}


bool Sheet::init()
{
	if(m_mapCS == 0 || m_registeredCS == 0)
		throw "One or more projections for sheet missing!";

	m_transRegMap = CreateTransformation(m_mapCS, m_registeredCS);
	m_transMapReg = CreateTransformation(m_registeredCS, m_mapCS);
	bool result = calculateAndAnalyzeBBox();

	m_bInit = true;

	return result;
}


bool Sheet::isOpen() const
{
	return m_lyr;
}


void Sheet::load(const RasterLayer<byte>::Pixel &px)
{
	if(m_lyr)
		return;

	boost::shared_ptr<RasterImage> img(new RasterImage());
	//char state = 'A';
	try
	{
		showVal(m_name);
		img->Load(m_name);
		//state = 'B';
		//m_lyr = boost::dynamic_pointer_cast<RasterLayer<byte>, IRasterLayer>(m_img->Layers()[0]);
		boost::shared_ptr<RasterLayer<byte>> lyr = boost::dynamic_pointer_cast<RasterLayer<byte>, IRasterLayer>(img->Layers()[0]);

		//showVal(lyr->get_SamplesPerPixel());
		//showVal(px.size());
		RasterLayer<byte>::Pixel bgColor(lyr->get_SamplesPerPixel());
		memset(&bgColor[0], 0xFF, bgColor.size());
		memcpy(&bgColor[0], &px[0], std::min(px.size(), bgColor.size()));

		/*for(RasterLayer<byte>::Pixel::iterator iter = bgColor.begin(); iter != bgColor.end(); ++iter)
		{
			showVal(int(*iter));
		}*/

		m_lyr = boost::shared_ptr<RasterLayer<byte>>(new RasterLayer<byte>((int)m_boxClipped.width(), (int)m_boxClipped.height(), lyr->get_SamplesPerPixel(), bgColor));
		lyr->getTile(*m_lyr, m_boxClipped.lower);
		img.reset();
		//state = 'C';

		if(m_adjustRGBSet)
			m_lyr->adjustRGB(m_adjustRed, m_adjustGreen, m_adjustBlue);
		//state = 'D';

		if(m_brightnessAndContrastSet)
			m_lyr->brightnessAndContrast(m_brightness, m_contrast);
		//state = 'E';

		// Depalettize only after operations which act on a palette have been performed
		// This is unnecessary, lyr->getPixel_filtered already does this!
		//if(m_lyr->hasPalette())
		//	m_lyr->depalettize();
		//state = 'F';
		
		//m_lyr->setBackground(px);
		//state = 'G';
		
		if(m_colorizeSet)
		{
			RasterLayer<byte> lyrTemp(m_lyr->get_Width(), m_lyr->get_Height(), 3, m_color);
			//state = 'H';
			m_lyr->merge(lyrTemp, m_colorOpacity);
		}
		//state = 'I';

		// Determine coordinate cache
		PixelCoord pc1((m_lyr->get_Width() - GRID_DIST) / 0.5, (m_lyr->get_Height() - GRID_DIST) / 0.5);
		PixelCoord pc2((m_lyr->get_Width() + GRID_DIST) / 0.5, (m_lyr->get_Height() + GRID_DIST) / 0.5);
		CoordXY mc1 = getMapCoord_internal(pc1);
		CoordXY mc2 = getMapCoord_internal(pc2);

		double gridDist = mc1.dist(mc2) * sqrt(0.5);
		m_coordCache = boost::shared_ptr<CoordinateCache<CoordXY, CoordXY>>(
						new CoordinateCache<CoordXY, CoordXY>(m_transMapReg, m_boxSheet, gridDist, gridDist));
	}
	catch(ImageBossException &exc)
	{
		std::cerr << "ERROR: " << exc.get_Message() << std::endl;
		//m_img.reset();
	}
	catch(const char *exc)
	{
		std::cerr << "ERROR: " << exc << std::endl;
	}
	catch(...)
	{
		std::cerr << "ERROR: unknown exception in Sheet::load (" << this->getName() << ")" << std::endl;
		//std::cerr << "state: " << state << std::endl;
		throw;
	}

	return;
}


void Sheet::unload()
{
	//m_img.reset();
	m_bInit = false;
}


bool Sheet::calculateAndAnalyzeBBox() const
{
	if(m_ip->getAnchors().size() == 0)
		return false;

	// Get the standard longitude (when available), so registered coordinates can be normalized towards it
	bool regIsGeoCS = (boost::dynamic_pointer_cast<GeographicCS>(m_registeredCS) != 0);
	double lon0 = 0;

	boost::shared_ptr<ProjectedCS> projMapCS = boost::dynamic_pointer_cast<ProjectedCS>(m_mapCS);
	if(projMapCS != 0)
	{
		const std::vector<Parameter> &params = projMapCS->getParameters();
		for(std::vector<Parameter>::const_iterator iterParam = params.begin(); \
			iterParam != params.end(); ++iterParam)
		{
			if(iterParam->name.compare(g_PARAM_LON0) == 0)
			{
				lon0 = iterParam->value;
				break;
			}
		}
	}

	// Create basic information
	int w, h, spp, bps;
	try
	{
		RasterImage::GetImageData(m_name, w, h, spp, bps);
	}
	catch(const ImageBossException &exc)
	{
		std::cout << m_name << ": " << exc.get_Message() << std::endl;
		return false;
	}

	// Update anchor coordinates (must be in map coordinates, instead of registered coordinates)
	// Also determine range of pixel coordinates, so the image is not loaded prematurely
	m_boxClipped = Box<PixelCoordI>(PixelCoordI(w, h), PixelCoordI(0, 0));
	m_boxReg = Box<CoordXY>(CoordXY(INFINITE, INFINITE), CoordXY(-INFINITE, -INFINITE));
	for(std::vector<Anchor>::iterator iterAnchor = m_ip->getAnchors().begin(); \
		iterAnchor != m_ip->getAnchors().end(); ++iterAnchor)
	{
		// Normalize registered coordinates first
		if(regIsGeoCS && lon0 != 0)
			iterAnchor->rect = normalizeLon(iterAnchor->rect, lon0);

		m_boxReg.expand(iterAnchor->rect);
		
		iterAnchor->rect = m_transRegMap->execute(iterAnchor->rect);

		m_boxClipped.expand(PixelCoordI((int)iterAnchor->orig.x, (int)iterAnchor->orig.y));
	}

	for(std::vector<PixelCoord>::const_iterator iterCoord = m_boundingPolygonL.getRing().begin(); \
		iterCoord != m_boundingPolygonL.getRing().end(); ++iterCoord)
	{
		m_boxClipped.expand(PixelCoordI((int)iterCoord->x, (int)iterCoord->y));
	}

	for(std::vector<PolyGon<PixelCoord>>::const_iterator iterPol = m_additionalPolygonsL.begin(); \
		iterPol != m_additionalPolygonsL.end(); ++iterPol)
	{
		for(std::vector<PixelCoord>::const_iterator iterCoord = iterPol->getRing().begin(); \
			iterCoord != iterPol->getRing().end(); ++iterCoord)
		{
			m_boxClipped.expand(PixelCoordI((int)iterCoord->x, (int)iterCoord->y));
		}
	}

	// Expand corners a bit --> TODO: use decimation, although it doesn't seem to be necessary

	// TODO: adjust for curvature, in case it goes beyond the anchors
	const int ANCHOR_CORR = 10;
	m_boxClipped.lower.x = std::max(0, m_boxClipped.lower.x - ANCHOR_CORR);
	m_boxClipped.lower.y = std::max(0, m_boxClipped.lower.y - ANCHOR_CORR);
	m_boxClipped.upper.x = std::min(w, m_boxClipped.upper.x + ANCHOR_CORR);
	m_boxClipped.upper.y = std::min(h, m_boxClipped.upper.y + ANCHOR_CORR);
	//showVal(m_boxClipped);

	// Correct original anchors and polygon
	for(std::vector<Anchor>::iterator iterAnchor = m_ip->getAnchors().begin(); \
		iterAnchor != m_ip->getAnchors().end(); ++iterAnchor)
	{
		iterAnchor->orig.x -= m_boxClipped.lower.x;
		iterAnchor->orig.y -= m_boxClipped.lower.y;
	}

	// Correct polygons and also calculate reprojected polygons
	for(std::vector<PixelCoord>::iterator iterCoord = m_boundingPolygonL.getRing().begin(); \
		iterCoord != m_boundingPolygonL.getRing().end(); ++iterCoord)
	{
		iterCoord->x -= m_boxClipped.lower.x;
		iterCoord->y -= m_boxClipped.lower.y;
		m_boundingPolygon.addPoint(getMapCoord_internal(*iterCoord));
	}

	for(std::vector<PolyGon<PixelCoord>>::iterator iterPol = m_additionalPolygonsL.begin(); \
		iterPol != m_additionalPolygonsL.end(); ++iterPol)
	{
		m_additionalPolygons.push_back(PolyGon<CoordXY>());
		for(std::vector<PixelCoord>::iterator iterCoord = iterPol->getRing().begin(); \
			iterCoord != iterPol->getRing().end(); ++iterCoord)
		{
			iterCoord->x -= m_boxClipped.lower.x;
			iterCoord->y -= m_boxClipped.lower.y;
			m_additionalPolygons.back().addPoint(getMapCoord_internal(*iterCoord));
		}
	}

	for(std::vector<PolyGon<PixelCoord>>::iterator iterPol = m_subtractionalPolygonsL.begin(); \
		iterPol != m_subtractionalPolygonsL.end(); ++iterPol)
	{
		m_subtractionalPolygons.push_back(PolyGon<CoordXY>());
		for(std::vector<PixelCoord>::iterator iterCoord = iterPol->getRing().begin(); \
			iterCoord != iterPol->getRing().end(); ++iterCoord)
		{
			iterCoord->x -= m_boxClipped.lower.x;
			iterCoord->y -= m_boxClipped.lower.y;
			m_subtractionalPolygons.back().addPoint(getMapCoord_internal(*iterCoord));
		}
	}

	// Determine the corners of the image which contain the data
	std::vector<PixelCoord> corners;
	corners.push_back(PixelCoord(m_boxClipped.lower.x, m_boxClipped.lower.y));
	corners.push_back(PixelCoord(m_boxClipped.lower.x, m_boxClipped.upper.y));
	corners.push_back(PixelCoord(m_boxClipped.upper.x, m_boxClipped.lower.y));
	corners.push_back(PixelCoord(m_boxClipped.upper.x, m_boxClipped.upper.y));

	// Add intermediate points, to account for curvature
	const int NUM_INTERMEDIATE = 10;
	for (int i = 1; i < NUM_INTERMEDIATE; i++)
	{
		corners.push_back(PixelCoord(m_boxClipped.lower.x, m_boxClipped.lower.y + i * m_boxClipped.height() / NUM_INTERMEDIATE));
		corners.push_back(PixelCoord(m_boxClipped.upper.x, m_boxClipped.lower.y + i * m_boxClipped.height() / NUM_INTERMEDIATE));
		corners.push_back(PixelCoord(m_boxClipped.lower.x + i * m_boxClipped.width() / NUM_INTERMEDIATE, m_boxClipped.lower.y));
		corners.push_back(PixelCoord(m_boxClipped.lower.x + i * m_boxClipped.width() / NUM_INTERMEDIATE, m_boxClipped.upper.y));
	}

	// And calculate boxSheet, based on the corners
	m_boxSheet = Box<CoordXY>(CoordXY(INFINITE, INFINITE), CoordXY(-INFINITE, -INFINITE));
	for(std::vector<PixelCoord>::iterator iterCorner = corners.begin(); \
		iterCorner != corners.end(); ++iterCorner)
	{
		CoordXY cornerR = getMapCoord_internal(PixelCoord(iterCorner->x - m_boxClipped.lower.x, iterCorner->y - m_boxClipped.lower.y));
		m_boxSheet.expand(cornerR);
	}

	return true;
}


/*PixelCoord Sheet::getOrigCoord(const CoordXY &mapCoord) const
{
	return getOrigCoord_internal(mapCoord);
}*/

/*PixelCoord Sheet::getOrigCoord_internal(const CoordXY &mapCoord) const
{
	return m_ip->getToImageCoord()->execute(mapCoord);
}*/

CoordXY Sheet::getMapCoord_internal(const PixelCoord &imageCoord) const
{
	return m_ip->getFromImageCoord()->execute(imageCoord);
}

boost::shared_ptr<const ImageProjection> Sheet::getImageProjection() const
{
	return m_ip;
}


bool Sheet::contains(const CoordXY &mapCoord) const
{
	// Outer box
	if(!this->m_boxSheet.contains(mapCoord))
	{
		return false;
	}

	// Check subtracting polygons
	for(PolygonXYIter iterPol = m_subtractionalPolygons.begin(); iterPol != m_subtractionalPolygons.end(); ++iterPol)
	{
		if(iterPol->contains(mapCoord))
		{
			return false;
		}
	}

	// Check bounding polygon
	if(this->m_boundingPolygon.getRing().size() > 0)
	{
		return (this->m_boundingPolygon.contains(mapCoord));
	}

	// Check additional polygons
	for(PolygonXYIter iterPol = m_additionalPolygons.begin(); iterPol != m_additionalPolygons.end(); ++iterPol)
	{
		if(iterPol->contains(mapCoord))
		{
			return true;
		}
	}

	// Check extent
	CoordXY cReg;
	if(m_coordCache)
	{
		m_coordCache->tryExecute(cReg, mapCoord);
	}
	else
	{
		cReg = m_transMapReg->execute(mapCoord);
	}

	bool regIsGeographic = (boost::dynamic_pointer_cast<GeographicCS>(m_registeredCS) != 0);
	if(regIsGeographic)
		cReg = normalizeLon(cReg, this->m_boxReg.center().x);

	return this->m_boxReg.contains(cReg);
}


bool Sheet::contains(const CoordXY &mapCoord, const PixelCoord &imageCoord) const
{
	// TBD: nodig voor Deense kaarten?
	//bool hasPolygon = (this->m_polygon.getRing().size() > 0);
	
	// Falls within image?
	if(this->m_lyr /*&& !hasPolygon*/)
	{
		if(!containsPixel(imageCoord))
		{
			return false;
		}
	}

	return this->contains(mapCoord);
}


void Sheet::setName(const std::string &name)
{
	m_name = name;

	return;
}


const std::string &Sheet::getName() const
{
	return m_name;
}


void Sheet::setAnchors(const std::vector<Anchor> &anchors)
{
	m_ip = boost::shared_ptr<ImageProjection>(new ImageProjection(anchors));
}


void Sheet::setBoundingPolygon(const PolyGon<PixelCoord> &polygon)
{
	m_boundingPolygonL = polygon;
}


void Sheet::addAdditionalPolygon(const PolyGon<PixelCoord> &polygon)
{
	m_additionalPolygonsL.push_back(polygon);
}


void Sheet::addSubtractionalPolygon(const PolyGon<PixelCoord> &polygon)
{
	m_subtractionalPolygonsL.push_back(polygon);
}


Box<CoordXY> Sheet::getBoxSheetMap() const
{
	return m_boxSheet;
}


const Box<CoordXY> &Sheet::getBoxSheetTarget() const
{
	return m_boxTarget;
}


void Sheet::setBoxSheetTarget(const Box<CoordXY> &boxTarget)
{
	m_boxTarget = boxTarget;
}


const boost::shared_ptr<RasterLayer<byte>> Sheet::getLayer() const
{
	return m_lyr;
}


bool Sheet::containsPixel(const PixelCoord &imageCoord) const
{
	return
		imageCoord.x >= -0.5 &&
		imageCoord.y >= -0.5 &&
		imageCoord.x < m_lyr->get_Width() + 0.5 &&
		imageCoord.y < m_lyr->get_Height() + 0.5;
}


void Sheet::setMapCS(const boost::shared_ptr<CoordinateSystem> &mapCS)
{
	m_mapCS = mapCS;
	if(m_registeredCS == 0)
		m_registeredCS = mapCS;

	return;
}


const boost::shared_ptr<CoordinateSystem> Sheet::getMapCS() const
{
	return m_mapCS;
}


void Sheet::setRegisteredCS(const boost::shared_ptr<CoordinateSystem> &registeredCS)
{
	m_registeredCS = registeredCS;

	return;
}


void Sheet::getPixel(RasterLayer<byte>::DataIter &px, const PixelCoord &pc, float decX, float decY,
					 const RasterFilter &uFilter)
{
	if(containsPixel(pc))
	{
		m_lyr->getPixel_filtered(px, pc, decX, decY, uFilter, BorderPixelBehavior::UseCurrentColor);
	}

	return;
}


int Sheet::getPriority() const
{
	return m_priority;
}


void Sheet::setPriority(int newPriority)
{
	m_priority = newPriority;
	return;
}


int Sheet::getBrightness() const
{
	return m_brightness;
}


float Sheet::getContrast() const
{
	return m_contrast;
}


void Sheet::setBrightnessAndContrast(int brightness, float contrast)
{
	m_brightness = brightness;
	m_contrast = contrast;
	m_brightnessAndContrastSet = true;

	return;
}


const RasterLayer<byte>::Pixel &Sheet::getColor() const
{
	return m_color;
}


float Sheet::getColorOpacity() const
{
	return m_colorOpacity;
}


void Sheet::setColorAndOpacity(const RasterLayer<byte>::Pixel &color, float colorOpacity)
{
	m_color = color;
	m_colorOpacity = colorOpacity;
	m_colorizeSet = true;

	return;
}


float Sheet::adjustRed() const
{
	return m_adjustRed;
}


float Sheet::adjustGreen() const
{
	return m_adjustGreen;
}


float Sheet::adjustBlue() const
{
	return m_adjustBlue;
}


void Sheet::setAdjustRGB(float red, float green, float blue)
{
	m_adjustRed = red;
	m_adjustGreen = green;
	m_adjustBlue = blue;
	m_adjustRGBSet = true;

	return;
}
