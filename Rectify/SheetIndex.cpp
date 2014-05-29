// SheetIndex.cpp
// FS, 15-07-2007

#include "Rectify.h"

extern ContextData g_uContext;


SheetIndex::SheetIndex()
{
}


SheetIndex::~SheetIndex()
{
}


std::vector<boost::shared_ptr<Sheet>> &SheetIndex::getSheets()
{
	return m_sheets;
}


const std::vector<boost::shared_ptr<Sheet>> &SheetIndex::getSheets() const
{
	return m_sheets;
}


std::vector<boost::shared_ptr<Sheet>>::size_type SheetIndex::size() const
{
	return m_sheets.size();
}


std::vector<boost::shared_ptr<Sheet>>::iterator SheetIndex::begin()
{
	return m_sheets.begin();
}


std::vector<boost::shared_ptr<Sheet>>::iterator SheetIndex::end()
{
	return m_sheets.end();
}


std::vector<boost::shared_ptr<Sheet>>::const_iterator SheetIndex::begin() const
{
	return m_sheets.begin();
}


std::vector<boost::shared_ptr<Sheet>>::const_iterator SheetIndex::end() const
{
	return m_sheets.end();
}


boost::shared_ptr<Sheet> &SheetIndex::operator[](std::vector<Sheet>::size_type idx)
{
	return m_sheets[idx];
}


const boost::shared_ptr<Sheet> &SheetIndex::operator[](std::vector<Sheet>::size_type idx) const
{
	return m_sheets[idx];
}


void SheetIndex::readGeneral(std::ifstream &fIn)
{
	std::string line;

	while(std::getline(fIn, line))
	{
		//std::cout << line << '\n' << std::endl;
		line = SL::trim(line, WHITESPACE);

		if(fIn.peek() == '[')
		{
			std::cout << "next section found" << std::endl;
			break;
		}

		if(line.empty())
			continue;
		switch(line[0])
		{
		case ';':   // Comment
		case '#':   // Comment
			continue;

		case '[':
			break;

		default:
			std::vector<std::string> vecWords = SL::split(line, "=");
			if(vecWords.size() < 2)
				continue;   // Can't parse

			if(vecWords[0] == "ProjectionAlias")
			{
				std::vector<std::string> vecAliasParts = SL::split(vecWords[1], "|");
				if(vecAliasParts.size() > 1)
				{
					// New projection alias
					//std::cout << "Create projection alias, name: " << vecAliasParts[0] << std::endl;
					g_mapCS[vecAliasParts[0]] = CoordinateSystem::CreateCS(vecAliasParts[1]);
				}
			}
			break;
		}

	}

	return;
}


void SheetIndex::read(std::ifstream &fIn, const boost::shared_ptr<CoordinateSystem> &cs, const Box<CoordXY> &targetBox)
{
	//std::ifstream fIn(inputFile.c_str());

	std::string line;
	boost::shared_ptr<Sheet>sheet(new Sheet());
	bool firstSheet = true;
	std::vector<Anchor> anchors;

	while(std::getline(fIn, line))
	{
		line = SL::trim(line, WHITESPACE);

		if(line.empty())
			continue;
		switch(line[0])
		{
		case ';':   // Comment
		case '#':   // Comment
			continue;

		case '[':
			if(line == "[SheetFile]")
			{
				// New sheetfile, empty sheet
				if(!firstSheet)
				{
					sheet->setAnchors(anchors);
					if(sheet->init())
						m_sheets.push_back(sheet);
				}
				firstSheet = false;

				sheet = boost::shared_ptr<Sheet>(new Sheet());
				anchors.clear();
			}
			break;

		default:
			std::vector<std::string> vecWords = SL::split(line, "=");
			if(vecWords.size() < 2)
				continue;   // Can't parse

			if(vecWords[0] == "FileName")
			{
				// Read file name
				sheet->setName(vecWords[1]);
			}
			else if(vecWords[0] == "Anchor")
			{
				std::vector<std::string> vecAPoints = SL::split(vecWords[1], "|");
				if(vecAPoints.size() < 4)
					continue;
				Anchor anchor(
					PixelCoord(atof(vecAPoints[0]), atof(vecAPoints[1])),
					CoordXY(atof(vecAPoints[2]), atof(vecAPoints[3])));
				anchors.push_back(anchor);
			}
			else if(vecWords[0] == "BoundingShape" || vecWords[0] == "AdditionalShape")
			{
				if(vecWords[0] == "AdditionalShape")
					sheet->setPolygonIsAdditional(true);

				std::vector<std::string> vecBShape = SL::split(vecWords[1], "|");
				PolyGon<PixelCoord> polygonL;
				for(std::vector<std::string>::iterator iterOrd = vecBShape.begin(); \
					iterOrd < vecBShape.end(); iterOrd += 2)
				{
					PixelCoord pt(atof(*iterOrd), atof(*(iterOrd + 1)));
					polygonL.addPoint(pt);
				}
				sheet->setPolygonL(polygonL);
			}
			else if(vecWords[0] == "ProjectionAlias")
			{
				std::vector<std::string> vecAliasParts = SL::split(vecWords[1], "|");
				if(vecAliasParts.size() > 1)
				{
					// New projection alias
					std::cout << "Create projection alias, name: " << vecAliasParts[0] << std::endl;
					g_mapCS[vecAliasParts[0]] = CoordinateSystem::CreateCS(vecAliasParts[1]);
				}
				else
				{
					// Refers to an existing projection
					std::map<std::string, boost::shared_ptr<CoordinateSystem>>::iterator iterCS =
						g_mapCS.find(vecWords[1]);
					if(iterCS == g_mapCS.end())
					{
						char err[255];
						_snprintf(err, 255, "Projection alias '%s' not found!", vecWords[1].c_str());
						throw err;
					}
					sheet->setMapCS(iterCS->second);
				}
			}
			else if(vecWords[0] == "RegisteredCoordAlias")
			{
				// Refers to an existing projection
				std::map<std::string, boost::shared_ptr<CoordinateSystem>>::iterator iterCS =
					g_mapCS.find(vecWords[1]);
				if(iterCS == g_mapCS.end())
				{
					char err[255];
					_snprintf(err, 255, "Registered coordinate alias '%s' not found!", vecWords[1].c_str());
					throw err;
				}
				sheet->setRegisteredCS(iterCS->second);
			}
			else if(vecWords[0] == "Priority")
			{
				// Sheet priority
				int priority = atoi(vecWords[1].c_str());
				sheet->setPriority(priority);
			}
			else if(vecWords[0] == "BrightnessAndContrast")
			{
				std::vector<std::string> vecParams = SL::split(vecWords[1], ",");
				int brightness = atoi(vecParams[0].c_str());
				float contrast = static_cast<float>(atof(vecParams[1].c_str()));
				sheet->setBrightnessAndContrast(brightness, contrast);
			}
			else if(vecWords[0] == "Colorize")
			{
				std::vector<std::string> vecParams = SL::split(vecWords[1], ",");
				RasterLayer<byte>::Pixel color(3);
				float colorOpacity = static_cast<float>(atof(vecParams[1].c_str()));

				// Parse color
				if(vecParams[0][0] == '#')
					vecParams[0] = vecParams[0].substr(1);
				if(vecParams[0].compare(0, 2, "0x") != 0 && vecParams[0].compare(0, 2, "0X") != 0)
					vecParams[0] = "0x" + vecParams[0];
				int temp = strtol(vecParams[0].c_str(), 0, 16);

				color[0] = (temp & 0xFF0000) >> 16;
				color[1] = (temp & 0xFF00) >> 8;
				color[2] = temp & 0xFF;

				sheet->setColorAndOpacity(color, colorOpacity);
			}
			else if(vecWords[0] == "AdjustRGB")
			{
				std::vector<std::string> vecParams = SL::split(vecWords[1], ",");
				float adjustRed = static_cast<float>(atof(vecParams[0].c_str()));
				float adjustGreen = static_cast<float>(atof(vecParams[1].c_str()));
				float adjustBlue = static_cast<float>(atof(vecParams[2].c_str()));
				sheet->setAdjustRGB(adjustRed, adjustGreen, adjustBlue);
			}

			break;
		}
	}
	
	// Add last sheet
	if(!firstSheet)
	{
		sheet->setAnchors(anchors);
		if(sheet->init())
			m_sheets.push_back(sheet);
	}

	return;
}


void SheetIndex::matchBox(const Box<CoordXY> &box, const boost::shared_ptr<CoordinateSystem> &cs)
{
	// NOTE: iterator can't be used, because it raises an error when the first sheet has to be
	// deleted
	showVal(box);
	for(int i = 0; i < static_cast<int>(m_sheets.size()); ++i)
	{
		// TODO: sheet moet gebruik maken van de bbox in map coords
		Box<CoordXY> boxSheetTgt = reprojectSheetBox(m_sheets[i]->getBoxSheetMap(), box, m_sheets[i]->getMapCS(), cs);
		//showVal(boxSheetTgt);
		if(boxSheetTgt.lower.y > box.upper.y || boxSheetTgt.upper.y < box.lower.y || \
			boxSheetTgt.lower.x > box.upper.x || boxSheetTgt.upper.x < box.lower.x)
		{
			m_sheets.erase(m_sheets.begin() + i);
			i--;
		}
	}

	return;
}


void SheetIndex::loadSheets(SheetIndex &feederSheets, const Box<CoordXY> &box, int y, double rectY, const boost::shared_ptr<CoordinateSystem> &cs)
{
	static double distX = fabs(GRID_DIST * g_uContext.resx);
	static double distY = fabs(GRID_DIST * g_uContext.resy);

	std::map<boost::shared_ptr<Sheet>, boost::shared_ptr<CoordinateCache<CoordXY, PixelCoord>>>::const_iterator iterCC_img;
	std::map<boost::shared_ptr<Sheet>, boost::shared_ptr<CoordinateCache<CoordXY, CoordXY>>>::const_iterator iterCC_map;

	// NOTE: iterator can't be used, because it raises an error when the first sheet has to be
	// deleted
	for(int i = 0; i < static_cast<int>(feederSheets.size()); ++i)
	{
		Box<CoordXY> boxSheetTgt = reprojectSheetBox(feederSheets[i]->getBoxSheetMap(), box, feederSheets[i]->getMapCS(), cs);
		if(boxSheetTgt.lower.y <= rectY && boxSheetTgt.upper.y >= rectY && \
			boxSheetTgt.lower.x <= box.upper.x && boxSheetTgt.upper.x >= box.lower.x)
		{
			std::cout << y << ", " << rectY << ": add active sheet " << i << "/" <<
				feederSheets.size() << ", name: " << feederSheets[i]->getName() << std::endl;
			std::cout << "box: " << boxSheetTgt << std::endl;
			m_sheets.push_back(feederSheets[i]);

			// Bepaal de coordinate caches
			boost::shared_ptr<XYTrans<CoordXY, CoordXY>> targetToMap = CreateTransformation(feederSheets[i]->getMapCS(), cs);

			iterCC_img = g_coordCaches_img.find(feederSheets[i]);
			if(iterCC_img == g_coordCaches_img.end())
			{
				boost::shared_ptr<XYTrans<CoordXY, PixelCoord>> targetToImage =
					boost::shared_ptr<XYTrans<CoordXY, PixelCoord>>(new TransChain<CoordXY, PixelCoord, CoordXY>(targetToMap, feederSheets[i]->getImageProjection()->getToImageCoord()));

				g_coordCaches_img[feederSheets[i]] = boost::shared_ptr<CoordinateCache<CoordXY, PixelCoord>>(
					new CoordinateCache<CoordXY, PixelCoord>(targetToImage, boxSheetTgt, distX, distY));
				iterCC_img = g_coordCaches_img.find(feederSheets[i]);
			}

			iterCC_map = g_coordCaches_map.find(feederSheets[i]);
			if(iterCC_map == g_coordCaches_map.end())
			{
				g_coordCaches_map[feederSheets[i]] = boost::shared_ptr<CoordinateCache<CoordXY, CoordXY>>(
					new CoordinateCache<CoordXY, CoordXY>(targetToMap, boxSheetTgt, distX, distY));
				iterCC_map = g_coordCaches_map.find(feederSheets[i]);
			}

			// Verwijder de te laden sheet uit de feederlijst
			feederSheets.getSheets().erase(feederSheets.getSheets().begin() + i);
			i--;
		}
	}

	return;
}


void SheetIndex::unloadSheets(const Box<CoordXY> &box, int y, double rectY, const boost::shared_ptr<CoordinateSystem> &cs)
{
	std::map<boost::shared_ptr<Sheet>, boost::shared_ptr<CoordinateCache<CoordXY, PixelCoord>>>::iterator iterCC_img;
	std::map<boost::shared_ptr<Sheet>, boost::shared_ptr<CoordinateCache<CoordXY, CoordXY>>>::iterator iterCC_map;

	// NOTE: iterator can't be used, because it raises an error when the first sheet has to be
	// deleted
	for(int i = 0; i < static_cast<int>(m_sheets.size()); ++i)
	{
		Box<CoordXY> boxSheetTgt = reprojectSheetBox(m_sheets[i]->getBoxSheetMap(), box, m_sheets[i]->getMapCS(), cs);
		if(boxSheetTgt.lower.y > rectY || boxSheetTgt.upper.y < rectY)
		{
			// Verwijder de coordinate caches
			iterCC_img = g_coordCaches_img.find(m_sheets[i]);
			if(iterCC_img != g_coordCaches_img.end())
			{
				iterCC_img->second.reset();
				g_coordCaches_img.erase(iterCC_img);
			}

			iterCC_map = g_coordCaches_map.find(m_sheets[i]);
			if(iterCC_map != g_coordCaches_map.end())
			{
				iterCC_map->second.reset();
				g_coordCaches_map.erase(iterCC_map);
			}

			std::cout << y << ", " << rectY << ": remove active sheet " << i << "/" <<
				m_sheets.size() << ", name: " << m_sheets[i]->getName() << ", box: " << boxSheetTgt << std::endl;
			m_sheets[i]->unload();
			// Direct erase causes a crash! Swap sheet with last sheets works fine
			std::swap(m_sheets[i], m_sheets.back());

			m_sheets.erase(m_sheets.end() - 1);
			i--;
		}
	}
	
	return;
}


ConstSheetIterator SheetIndex::findActiveSheet(const CoordXY &rectCoord, const boost::shared_ptr<CoordinateSystem> &cs) const
{
	ConstSheetIterator activeSheet = m_sheets.end();
	int maxPriority = 0x80000000;

	//bool targetIsGeographic = (boost::dynamic_pointer_cast<GeographicCS>(cs) != 0);

	for(ConstSheetIterator iterSheet = m_sheets.begin(); \
		iterSheet != m_sheets.end(); ++iterSheet)
	{
		// Test if coord is contained by sheet.
		boost::shared_ptr<XYTrans<CoordXY, CoordXY>> targetToMap = CreateTransformation((*iterSheet)->getMapCS(), cs);
		CoordXY mapCoord;
		//if(targetIsGeographic)
		//	mapCoord = normalizeLon(targetToMap->execute(rectCoord), rectCoord.x);
		//else
			mapCoord = targetToMap->execute(rectCoord);

		bool bContains = (*iterSheet)->contains(mapCoord);

		if(bContains && (*iterSheet)->getPriority() > maxPriority)
		{
			activeSheet = iterSheet;
			maxPriority = (*iterSheet)->getPriority();
		}
	}

	return activeSheet;
}

Box<CoordXY> SheetIndex::reprojectSheetBox(const Box<CoordXY> &sourceBox, const Box<CoordXY> &boxTgt, const boost::shared_ptr<CoordinateSystem> &sourceCS, const boost::shared_ptr<CoordinateSystem> &targetCS)
{
	bool targetIsGeographic = (boost::dynamic_pointer_cast<GeographicCS>(targetCS) != 0);

	Box<CoordXY> targetBox(CoordXY(INFINITE, INFINITE), CoordXY(-INFINITE, -INFINITE));
	boost::shared_ptr<XYTrans<CoordXY, CoordXY>> trans = CreateTransformation(targetCS, sourceCS);

	// NB: houdt geen rekening met latlon/cylindrical CS
	if(targetIsGeographic)
	{
		double cx = boxTgt.center().x;
		targetBox.expand(normalizeLon(trans->execute(CoordXY(sourceBox.lower.x, sourceBox.lower.y)), cx));
		targetBox.expand(normalizeLon(trans->execute(CoordXY(sourceBox.center().x, sourceBox.lower.y)), cx));
		targetBox.expand(normalizeLon(trans->execute(CoordXY(sourceBox.upper.x, sourceBox.lower.y)), cx));
		targetBox.expand(normalizeLon(trans->execute(CoordXY(sourceBox.lower.x, sourceBox.center().y)), cx));
		targetBox.expand(normalizeLon(trans->execute(CoordXY(sourceBox.center().x, sourceBox.center().y)), cx));
		targetBox.expand(normalizeLon(trans->execute(CoordXY(sourceBox.upper.x, sourceBox.center().y)), cx));
		targetBox.expand(normalizeLon(trans->execute(CoordXY(sourceBox.lower.x, sourceBox.upper.y)), cx));
		targetBox.expand(normalizeLon(trans->execute(CoordXY(sourceBox.center().x, sourceBox.upper.y)), cx));
		targetBox.expand(normalizeLon(trans->execute(CoordXY(sourceBox.upper.x, sourceBox.upper.y)), cx));
	}
	else
	{
		//boost::shared_ptr<ProjectedCS> proj1 = boost::dynamic_pointer_cast<ProjectedCS>(sourceCS);
		//proj1->getTransformation()
		//boost::shared_ptr<XYTrans<CoordXY, CoordXY>> z = CreateTransformation(g_csWGS84, sourceCS);
		//showVal(sourceBox.lower);
		//showVal(z->execute(sourceBox.lower));

		targetBox.expand(trans->execute(CoordXY(sourceBox.lower.x, sourceBox.lower.y)));
		targetBox.expand(trans->execute(CoordXY(sourceBox.center().x, sourceBox.lower.y)));
		targetBox.expand(trans->execute(CoordXY(sourceBox.upper.x, sourceBox.lower.y)));
		targetBox.expand(trans->execute(CoordXY(sourceBox.lower.x, sourceBox.center().y)));
		targetBox.expand(trans->execute(CoordXY(sourceBox.center().x, sourceBox.center().y)));
		targetBox.expand(trans->execute(CoordXY(sourceBox.upper.x, sourceBox.center().y)));
		targetBox.expand(trans->execute(CoordXY(sourceBox.lower.x, sourceBox.upper.y)));
		targetBox.expand(trans->execute(CoordXY(sourceBox.center().x, sourceBox.upper.y)));
		targetBox.expand(trans->execute(CoordXY(sourceBox.upper.x, sourceBox.upper.y)));
	}	

	return targetBox;
}
