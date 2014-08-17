// Geo_wkt.cpp
// FS, 11-07-2007

#define DLLAPI_GEO __declspec(dllexport)

#include "Geo_internal.h"


bool Parameter::operator==(const Parameter &other) const
{
	return this->name == other.name &&
		this->value == other.value;
}


void Parameter::parseWKT(const std::string &wkt)
{
	// <parameter> = PARAMETER["<name>", <value>]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	name = parseName(pos, wkt);

	// Parse value
	value = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	return;
}


MathTransform::MathTransform()
{
}


MathTransform::~MathTransform()
{
}


boost::shared_ptr<MathTransform> MathTransform::CreateMT(const std::string &wkt)
{
	//MathTransform *mt = 0;
	boost::shared_ptr<MathTransform> mt;

	std::string::size_type pos = wkt.find("[");
	std::string front = wkt.substr(0, pos);
	if(front.compare(g_PARAM_MT) == 0)
		mt = boost::shared_ptr<MathTransform>(new ParamMT());
	else if(front.compare(g_CONCAT_MT) == 0)
		mt = boost::shared_ptr<MathTransform>(new ConcatMT());
	else if(front.compare(g_INVERSE_MT) == 0)
		mt = boost::shared_ptr<MathTransform>(new InverseMT());
	else if(front.compare(g_PASSTHROUGH_MT) == 0)
		mt = boost::shared_ptr<MathTransform>(new PassthroughMT());
	else
		return mt;   // CS not recognized
	mt->parseWKT(wkt.substr(pos + 1));

	// Go to next pos
	pos = findMatchingBracket(pos, wkt);

	return mt;
}


ParamMT::ParamMT()
{
}


ParamMT::~ParamMT()
{
}


const std::string &ParamMT::getClassificationName() const
{
	return m_classificationName;
}


const std::vector<Parameter> &ParamMT::getParams() const
{
	return m_params;
}


void ParamMT::parseWKT(const std::string &wkt)
{
	// <param mt> = PARAM_MT["<classification name>" {,<parameter>}* ]

	// Parse classification name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_classificationName = parseName(pos, wkt);

	// Parse parameters
	while(wkt.find(g_PARAMETER, pos) != std::string::npos)
	{
		pos = wkt.find(g_PARAMETER, pos);
		pos += g_PARAMETER.length() + 1;
		Parameter param;
		param.parseWKT(wkt.substr(pos));
		m_params.push_back(param);
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


ConcatMT::ConcatMT()
{
}


ConcatMT::~ConcatMT()
{
	//for(std::vector<MathTransform *>::iterator iterMT = m_mathTransforms.begin(); \
	//	iterMT != m_mathTransforms.end(); ++iterMT)
	//{
	//	delete *iterMT;
	//}
}


const std::vector<boost::shared_ptr<MathTransform>> &ConcatMT::getMathTransforms() const
{
	return m_mathTransforms;
}


void ConcatMT::parseWKT(const std::string &wkt)
{
	// <concat mt> = CONCAT_MT[<math transform> {,<math transform>}* ]

	// Parse math transforms
	std::string::size_type pos = 0;

	while(wkt.find_first_of(g_IDENTIFIER_STARTCHARS, pos) != std::string::npos)
	{
		pos = wkt.find_first_of(g_IDENTIFIER_STARTCHARS, pos);
		if(pos == std::string::npos)
			throw "WKT syntax error";
		boost::shared_ptr<MathTransform> mt = MathTransform::CreateMT(wkt.substr(pos));
		m_mathTransforms.push_back(mt);

		pos = findMatchingBracket(pos, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


InverseMT::InverseMT() //: m_mathTransform(0)
{
}


InverseMT::~InverseMT()
{
	//delete m_mathTransform;
}


const MathTransform &InverseMT::getMathTransform() const
{
	return *m_mathTransform;
}


void InverseMT::parseWKT(const std::string &wkt)
{
	// <inv mt> = INVERSE_MT[<math transform>]

	// Parse math transform
	std::string::size_type pos = wkt.find_first_of(g_IDENTIFIER_STARTCHARS);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_mathTransform = MathTransform::CreateMT(wkt.substr(pos));
	
	return;
}


PassthroughMT::PassthroughMT() //: m_mathTransform(0)
{
}


PassthroughMT::~PassthroughMT()
{
	//delete m_mathTransform;
}


int PassthroughMT::getInteger() const
{
	return m_integer;
}


const MathTransform &PassthroughMT::getMathTransform() const
{
	return *m_mathTransform;
}


void PassthroughMT::parseWKT(const std::string &wkt)
{
	// <passthrough mt> = PASSTHROUGH_MT[<integer>, <math transform>]

	// Parse integer
	std::string::size_type pos = 0;
	m_integer = parseInt(pos, wkt);

	// Parse math transforms
	pos = wkt.find_first_of(g_IDENTIFIER_STARTCHARS, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_mathTransform = MathTransform::CreateMT(wkt.substr(pos));

	return;
}


void Authority::parseWKT(const std::string &wkt)
{
	// <authority> = AUTHORITY["<name>", "<code>"]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	name = parseName(pos, wkt);

	// Parse code
	pos = wkt.find('\"', pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	code = parseName(pos, wkt);

	return;
}


Projection::Projection() //: m_tp(0), m_fp(0)
{
}


Projection::~Projection()
{
	//delete m_tp;
	//delete m_fp;
}


bool Projection::operator==(const Projection &other) const
{
	return this->m_name == other.m_name;
}


void Projection::parseWKT(const std::string &wkt)
{
	// <projection> = PROJECTION["<name>" {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


void Projection::setParams(const std::map<ProjParam, double> &params_p)
{
	// Parse projection
	// TODO case insensitive
	if(this->m_name.compare(g_PROJ_TM) == 0)
	{
		m_tp = boost::shared_ptr<ToProjection>(new ToTransverseMercator(params_p));
		m_fp = boost::shared_ptr<FromProjection>(new FromTransverseMercator(params_p));
	}
	else if(this->m_name.compare(g_PROJ_MERC_1SP) == 0 ||
		this->m_name.compare(g_PROJ_MERC) == 0)
	{
		m_tp = boost::shared_ptr<ToProjection>(new ToMercator(params_p));
		m_fp = boost::shared_ptr<FromProjection>(new FromMercator(params_p));
	}
	else if(this->m_name.compare(g_PROJ_BONNE) == 0)
	{
		m_tp = boost::shared_ptr<ToProjection>(new ToBonne(params_p));
		m_fp = boost::shared_ptr<FromProjection>(new FromBonne(params_p));
	}
	else if(this->m_name.compare(g_PROJ_OBLSTER) == 0)
	{
		m_tp = boost::shared_ptr<ToProjection>(new ToObliqueStereographic(params_p));
		m_fp = boost::shared_ptr<FromProjection>(new FromObliqueStereographic(params_p));
	}
	else if(this->m_name.compare(g_PROJ_LCC_1SP) == 0 ||
		this->m_name.compare(g_PROJ_LCC) == 0)
	{
		m_tp = boost::shared_ptr<ToProjection>(new ToLambertConformalConic_1SP(params_p));
		m_fp = boost::shared_ptr<FromProjection>(new FromLambertConformalConic_1SP(params_p));
	}
	else if(this->m_name.compare(g_PROJ_POLYCONIC) == 0)
	{
		m_tp = boost::shared_ptr<ToProjection>(new ToPolyconic(params_p));
		m_fp = boost::shared_ptr<FromProjection>(new FromPolyconic(params_p));
	}
	else if(this->m_name.compare(g_PROJ_PREUSS) == 0)
	{
		m_tp = boost::shared_ptr<ToProjection>(new ToPreussischePolyeder(params_p));
		m_fp = boost::shared_ptr<FromProjection>(new FromPreussischePolyeder(params_p));
	}
	else if(this->m_name.compare(g_PROJ_CASSINI) == 0 || this->m_name.compare(g_PROJ_CASSINI_SOLDNER) == 0)
	{
		m_tp = boost::shared_ptr<ToProjection>(new ToCassiniSoldner(params_p));
		m_fp = boost::shared_ptr<FromProjection>(new FromCassiniSoldner(params_p));
	}
	else
	{
		throw "Unknown projection";
	}


	return;
}


const boost::shared_ptr<ToProjection> &Projection::getToProjection() const
{
	return m_tp;
}


const boost::shared_ptr<FromProjection> &Projection::getFromProjection() const
{
	return m_fp;
}


boost::shared_ptr<Unit> Unit::CreateUnit(const std::string &wkt)
{
	//Unit *unit = 0;
	boost::shared_ptr<Unit> unit;

	return unit;
}


bool LinearUnit::operator==(const Unit &other) const
{
	if(dynamic_cast<const LinearUnit *>(&other) != 0)
		return this->operator==(*dynamic_cast<const LinearUnit *>(&other));
	else
		return false;
}


bool LinearUnit::operator==(const LinearUnit &other) const
{
	return this->conversionFactor == other.conversionFactor;
}


void LinearUnit::parseWKT(const std::string &wkt)
{
	// <unit> = UNIT["<name>", <conversion factor> {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	name = parseName(pos, wkt);

	// Parse conversion factor
	conversionFactor = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


bool AngularUnit::operator==(const Unit &other) const
{
	if(dynamic_cast<const AngularUnit *>(&other) != 0)
		return this->operator==(*dynamic_cast<const AngularUnit *>(&other));
	else
		return false;
}


bool AngularUnit::operator==(const AngularUnit &other) const
{
	return this->conversionFactor == other.conversionFactor;
}


void AngularUnit::parseWKT(const std::string &wkt)
{
	// <unit> = UNIT["<name>", <conversion factor> {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	name = parseName(pos, wkt);

	// Parse conversion factor
	conversionFactor = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


bool Axis::operator==(const Axis &other) const
{
	return this->direction == other.direction;
}


void Axis::parseWKT(const std::string &wkt)
{
	// AXIS["<name>", NORTH | SOUTH | EAST | WEST | UP | DOWN | OTHER]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	name = parseName(pos, wkt);

	// Parse axis direction
	std::string sDirection = parseIdentifier(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	direction = axdrOther;
	for(int i = 0; i < g_NUM_AXES; ++i)
	{
		if(sDirection.compare(g_AXES[i]) == 0)
			direction = static_cast<AxisDirection>(i);
	}

	return;
}


Spheroid::Spheroid(): m_ell(Ellipsoid(1, INFINITE))
{
}


Spheroid::~Spheroid()
{
}


bool Spheroid::operator==(const Spheroid &other) const
{
	return this->m_semiMajorAxis == other.m_semiMajorAxis &&
		this->m_inverseFlattening == other.m_inverseFlattening;
}


const std::string &Spheroid::getName() const
{
	return m_name;
}


double Spheroid::getSemiMajorAxis() const
{
	return m_semiMajorAxis;
}


double Spheroid::getInverseFlattening() const
{
	return m_inverseFlattening;
}


const Authority &Spheroid::getAuthority() const
{
	return m_authority;
}


void Spheroid::parseWKT(const std::string &wkt)
{
	// <spheroid> = SPHEROID["<name>", <semi-major axis>, <inverse flattening>
	//   {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse semi-major axis
	m_semiMajorAxis = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse inverse flattening
	m_inverseFlattening = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}


	m_ell = Ellipsoid(m_semiMajorAxis, m_inverseFlattening);


	return;
}


const Ellipsoid &Spheroid::getEllipsoid() const
{
	return m_ell;
}


bool ToWGS84::operator==(const ToWGS84 &other) const
{
	return this->dx == other.dx &&
		this->dy == other.dy &&
		this->dz == other.dz &&
		this->ex == other.ex &&
		this->ey == other.ey &&
		this->ez == other.ez &&
		this->ppm == other.ppm;
}


void ToWGS84::parseWKT(const std::string &wkt)
{
	// <to wgs84s> = TOWGS84[<seven param>]
	std::string::size_type pos = 0;

	// Parse dx
	dx = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse dy
	dy = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse dz
	dz = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse ex
	ex = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse ey
	ey = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse ez
	ez = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse ppm
	ppm = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	return;
}


bool PrimeMeridian::operator==(const PrimeMeridian &other) const
{
	return this->m_longitude == other.m_longitude;
}


const std::string &PrimeMeridian::getName() const
{
	return m_name;
}


double PrimeMeridian::getLongitude() const
{
	return m_longitude;
}


const Authority &PrimeMeridian::getAuthority() const
{
	return m_authority;
}


void PrimeMeridian::parseWKT(const std::string &wkt)
{
	// <prime meridian> = PRIMEM["<name>", <longitude> {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse longitude
	m_longitude = parseDouble(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


bool Datum::operator==(const Datum &other) const
{
	return this->m_spheroid == other.m_spheroid &&
		this->m_toWGS84 == other.m_toWGS84;
}


const std::string &Datum::getName() const
{
	return m_name;
}


const Spheroid &Datum::getSpheroid() const
{
	return m_spheroid;
}


const ToWGS84 &Datum::getToWGS84() const
{
	return m_toWGS84;
}


const Authority &Datum::getAuthority() const
{
	return m_authority;
}


void Datum::parseWKT(const std::string &wkt)
{
	// <datum> = DATUM["<name>", <spheroid> {,<to wgs84>} {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse spheroid
	pos = wkt.find(g_SPHEROID, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_SPHEROID.length() + 1;
	m_spheroid.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse to WGS84
	if(wkt.find(g_TOWGS84, pos) != std::string::npos)
	{
		pos = wkt.find(g_TOWGS84, pos);
		pos += g_TOWGS84.length() + 1;
		m_toWGS84.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}
	else
	{
		m_toWGS84.dx = m_toWGS84.dy = m_toWGS84.dz = m_toWGS84.ex = m_toWGS84.ey = m_toWGS84.ez = m_toWGS84.ppm = 0;
	}

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


const std::string &VerticalDatum::getName() const
{
	return m_name;
}


int VerticalDatum::getDatumType() const
{
	return m_datumType;
}


const Authority &VerticalDatum::getAuthority() const
{
	return m_authority;
}


void VerticalDatum::parseWKT(const std::string &wkt)
{
	// <vert datum> = VERT_DATUM["<name>", <datum type> {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse datum type
	m_datumType = parseInt(pos, wkt);

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


CoordinateSystem::CoordinateSystem()
{
}


CoordinateSystem::~CoordinateSystem()
{
}


boost::shared_ptr<CoordinateSystem> CoordinateSystem::CreateCS(const std::string &wkt)
{
	boost::shared_ptr<CoordinateSystem> cs;

	std::string::size_type pos = wkt.find("[");
	std::string front = wkt.substr(0, pos);
	if(front.compare(g_PROJCS) == 0)
		cs = boost::shared_ptr<CoordinateSystem>(new ProjectedCS());
	else if(front.compare(g_GEOGCS) == 0)
		cs = boost::shared_ptr<CoordinateSystem>(new GeographicCS());
	else if(front.compare(g_GEOCCS) == 0)
		cs = boost::shared_ptr<CoordinateSystem>(new GeocentricCS());
	//else if(front.compare(g_VERT_CS) == 0)
	//	cs = boost::shared_ptr<CoordinateSystem>(new VerticalCS());
	//else if(front.compare(g_COMPD_CS) == 0)
	//	cs = boost::shared_ptr<CoordinateSystem>(new CompoundCS());
	//else if(front.compare(g_FITTED_CS) == 0)
	//	cs = boost::shared_ptr<CoordinateSystem>(new FittedCS());
	//else if(front.compare(g_LOCAL_CS) == 0)
	//	cs = boost::shared_ptr<CoordinateSystem>(new LocalCS());
	else
		return cs;   // CS not recognized
	cs->parseWKT(wkt.substr(pos + 1));

	// Go to next pos
	pos = findMatchingBracket(pos, wkt);

	return cs;
}


const std::string &CoordinateSystem::getName() const
{
	return m_name;
}


const Authority &CoordinateSystem::getAuthority() const
{
	return m_authority;
}


GeographicCS::GeographicCS()
{
}


GeographicCS::~GeographicCS()
{
}


bool GeographicCS::operator==(const CoordinateSystem &other) const
{
	if(dynamic_cast<const GeographicCS *>(&other) != 0)
		return this->operator==(*dynamic_cast<const GeographicCS *>(&other));
	else
		return false;
}


bool GeographicCS::operator==(const GeographicCS &other) const
{
	return this->m_datum == other.m_datum &&
		this->m_primeMeridian == other.m_primeMeridian &&
		this->m_angularUnit == other.m_angularUnit &&
		this->m_axis1 == other.m_axis1 &&
		this->m_axis2 == other.m_axis2;
}


const Datum &GeographicCS::getDatum() const
{
	return m_datum;
}


const PrimeMeridian &GeographicCS::getPrimeMeridian() const
{
	return m_primeMeridian;
}


const AngularUnit &GeographicCS::getAngularUnit() const
{
	return m_angularUnit;
}


const Axis &GeographicCS::getAxis1() const
{
	return m_axis1;
}


const Axis &GeographicCS::getAxis2() const
{
	return m_axis2;
}


void GeographicCS::parseWKT(const std::string &wkt)
{
	// <geographic cs> = GEOGCS["<name>", <datum>, <prime meridian>, <angular
	//   unit> {,<twin axes>} {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse datum
	pos = wkt.find(g_DATUM, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_DATUM.length() + 1;
	m_datum.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse prime meridian
	pos = wkt.find(g_PRIMEM, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_PRIMEM.length() + 1;
	m_primeMeridian.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse angular unit
	pos = wkt.find(g_UNIT, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_UNIT.length() + 1;
	m_angularUnit.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse axes
	if(wkt.find(g_AXIS, pos) != std::string::npos)
	{
		pos = wkt.find(g_AXIS, pos);
		pos += g_AXIS.length() + 1;
		m_axis1.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";

		pos = wkt.find(g_AXIS, pos);
		if(pos == std::string::npos)
			throw "WKT syntax error";
		pos += g_AXIS.length() + 1;
		m_axis2.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


boost::shared_ptr<XYTrans<CoordLL, CoordLL>> GeographicCS::getTransformation(
	const boost::shared_ptr<GeographicCS> &target) const
{
	if(*this == *target.get())
		return boost::shared_ptr<XYTrans<CoordLL, CoordLL>>(new IdentityTransform<CoordLL>());

	const ToWGS84 &tw1 = this->m_datum.getToWGS84();
	const ToWGS84 &tw2 = target->m_datum.getToWGS84();

	boost::shared_ptr<DatumTrans> dt1(new DatumTrans(
		this->m_datum.getSpheroid().getEllipsoid(), g_spheroidWGS84.getEllipsoid(),
		this->m_primeMeridian.getLongitude(), 0,
		tw1.dx, tw1.dy, tw1.dz, tw1.ppm, tw1.ex, tw1.ey, tw1.ez));
	boost::shared_ptr<DatumTrans> dt2(new DatumTrans(
		g_spheroidWGS84.getEllipsoid(), target->m_datum.getSpheroid().getEllipsoid(),
		0, target->m_primeMeridian.getLongitude(),
		-tw2.dx, -tw2.dy, -tw2.dz, -tw2.ppm, -tw2.ex, -tw2.ey, -tw2.ez));

	if(this->m_datum.getSpheroid() == g_spheroidWGS84 && this->m_primeMeridian.getLongitude() == 0)
	{
		// First datum is WGS84 
		return dt2;
	}
	else if(target->m_datum.getSpheroid() == g_spheroidWGS84 && target->m_primeMeridian.getLongitude() == 0)
	{
		// Second datum is WGS84
		return dt1;
	}

	return boost::shared_ptr<XYTrans<CoordLL, CoordLL>>(
		new TransChain<CoordLL, CoordLL, CoordLL>(dt1, dt2));
}


boost::shared_ptr<XYTrans<CoordLL, CoordXY>> GeographicCS::getTransformation(
	const boost::shared_ptr<ProjectedCS> &target) const
{
	// NOTE: 1st transformation is created anew!
	boost::shared_ptr<XYTrans<CoordLL, CoordLL>> temp = this->getTransformation(
		target->getGeographicCS());

	return boost::shared_ptr<XYTrans<CoordLL, CoordXY>>(new TransChain<CoordLL, CoordXY, CoordLL>(
		temp,
		target->getProjection().getToProjection()));
}


ProjectedCS::ProjectedCS(): m_geographicCS(new GeographicCS())
{
}


ProjectedCS::~ProjectedCS()
{
}


bool ProjectedCS::operator==(const CoordinateSystem &other) const
{
	if(dynamic_cast<const ProjectedCS *>(&other) != 0)
		return this->operator==(*dynamic_cast<const ProjectedCS *>(&other));
	else
		return false;
}


bool ProjectedCS::operator==(const ProjectedCS &other) const
{
	return *this->m_geographicCS == *other.m_geographicCS &&
		this->m_projection == other.m_projection &&
		this->m_params == other.m_params &&
		this->m_linearUnit == other.m_linearUnit &&
		this->m_axis1 == other.m_axis1 &&
		this->m_axis2 == other.m_axis2;
}


const boost::shared_ptr<GeographicCS> &ProjectedCS::getGeographicCS() const
{
	return m_geographicCS;
}


const Projection &ProjectedCS::getProjection() const
{
	return m_projection;
}


const std::vector<Parameter> &ProjectedCS::getParameters() const
{
	return m_params;
}


const LinearUnit &ProjectedCS::getLinearUnit() const
{
	return m_linearUnit;
}


const Axis &ProjectedCS::getAxis1() const
{
	return m_axis1;
}


const Axis &ProjectedCS::getAxis2() const
{
	return m_axis2;
}


void ProjectedCS::parseWKT(const std::string &wkt)
{
	// <projected cs> = PROJCS["<name>", <geographic cs>, <projection>,
	//   {<parameter>,}* <linear unit> {,<twin axes>}{,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse geographic CS
	pos = wkt.find(g_GEOGCS, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_GEOGCS.length() + 1;
	m_geographicCS->parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse projection
	pos = wkt.find(g_PROJECTION, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_PROJECTION.length() + 1;
	m_projection.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse parameters
	while(wkt.find(g_PARAMETER, pos) != std::string::npos)
	{
		pos = wkt.find(g_PARAMETER, pos);
		pos += g_PARAMETER.length() + 1;
		Parameter param;
		param.parseWKT(wkt.substr(pos));
		m_params.push_back(param);
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	// Parse linear unit
	if(wkt.find(g_UNIT, pos) != std::string::npos)
	{
		pos = wkt.find(g_UNIT, pos);
		if(pos == std::string::npos)
			throw "WKT syntax error";
		pos += g_UNIT.length() + 1;
		m_linearUnit.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	// Parse axes
	if(wkt.find(g_AXIS, pos) != std::string::npos)
	{
		pos = wkt.find(g_AXIS, pos);
		pos += g_AXIS.length() + 1;
		m_axis1.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";

		pos = wkt.find(g_AXIS, pos);
		if(pos == std::string::npos)
			throw "WKT syntax error";
		pos += g_AXIS.length() + 1;
		m_axis2.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);;
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}


	// Create ellipsoid
	Ellipsoid ell_p(
		m_geographicCS->getDatum().getSpheroid().getSemiMajorAxis(),
		m_geographicCS->getDatum().getSpheroid().getInverseFlattening());

	// Parse parameters
	std::map<ProjParam, double> params_p;
	params_p.insert(pairParam(pparE2, ell_p.e2()));
	params_p.insert(pairParam(pparA, ell_p.a()));

	params_p.insert(pairParam(pparPM, m_geographicCS->getPrimeMeridian().getLongitude()));

	for(std::vector<Parameter>::const_iterator iterParam = m_params.begin(); \
		iterParam != m_params.end(); ++iterParam)
	{
		if(iterParam->name.compare(g_PARAM_LAT0) == 0)
		{
			params_p.insert(pairParam(pparLat0, iterParam->value));
		}
		else if(iterParam->name.compare(g_PARAM_LON0) == 0)
		{
			params_p.insert(pairParam(pparLon0, iterParam->value));
		}
		else if(iterParam->name.compare(g_PARAM_K) == 0)
		{
			params_p.insert(pairParam(pparK, iterParam->value));
		}
		else if(iterParam->name.compare(g_PARAM_FE) == 0)
		{
			params_p.insert(pairParam(pparFE, iterParam->value));
		}
		else if(iterParam->name.compare(g_PARAM_FN) == 0)
		{
			params_p.insert(pairParam(pparFN, iterParam->value));
		}
		else if(iterParam->name.compare(g_PARAM_LAT1) == 0)
		{
			params_p.insert(pairParam(pparLat1, iterParam->value));
		}
		else if(iterParam->name.compare(g_PARAM_LAT2) == 0)
		{
			params_p.insert(pairParam(pparLat2, iterParam->value));
		}

		// Parameters:
		// num_row
		// num_col
		// elt_(r)_(c) (depends on num_row and num_col)
		// latitude_of_origin
		// central_meridian
		// standard_parallel_1
		// standard_parallel_2
		// scale_factor
		// false_easting, false_northing
		// semi_major, semi_minor
		// dim
		// dx, dy, dz
		// src_semi_major, src_semi_minor
		// tgt_semi_major, tgt_semi_minor
		// rotation
	}

	m_projection.setParams(params_p);

	return;
}


boost::shared_ptr<XYTrans<CoordXY, CoordLL>> ProjectedCS::getTransformation(
	const boost::shared_ptr<GeographicCS> &target) const
{
	// NOTE: 2nd transformation is created anew!
	boost::shared_ptr<XYTrans<CoordLL, CoordLL>> temp =
		this->m_geographicCS->getTransformation(target);

	return boost::shared_ptr<XYTrans<CoordXY, CoordLL>>(new TransChain<CoordXY, CoordLL, CoordLL>(
		this->m_projection.getFromProjection(),
		temp));
}


boost::shared_ptr<XYTrans<CoordXY, CoordXY>> ProjectedCS::getTransformation(
	const boost::shared_ptr<ProjectedCS> &target) const
{
	if(*this == *target.get())
		return boost::shared_ptr<XYTrans<CoordXY, CoordXY>>(new IdentityTransform<CoordXY>());

	// NOTE: 2nd transformation is created anew!
	boost::shared_ptr<XYTrans<CoordLL, CoordXY>> temp =
		this->m_geographicCS->getTransformation(target);

	return boost::shared_ptr<XYTrans<CoordXY, CoordXY>>(new TransChain<CoordXY, CoordXY, CoordLL>(
		this->m_projection.getFromProjection(),
		temp));
}


GeocentricCS::GeocentricCS()
{
}


GeocentricCS::~GeocentricCS()
{
}


bool GeocentricCS::operator==(const CoordinateSystem &other) const
{
	if(dynamic_cast<const GeocentricCS *>(&other) != 0)
		return this->operator==(*dynamic_cast<const GeocentricCS *>(&other));
	else
		return false;
}


bool GeocentricCS::operator==(const GeocentricCS &other) const
{
	return this->m_datum == other.m_datum &&
		this->m_primeMeridian == other.m_primeMeridian &&
		this->m_linearUnit == other.m_linearUnit &&
		this->m_axis1 == other.m_axis1 &&
		this->m_axis2 == other.m_axis2 &&
		this->m_axis3 == other.m_axis3;
}


const Datum &GeocentricCS::getDatum() const
{
	return m_datum;
}


const PrimeMeridian &GeocentricCS::getPrimeMeridian() const
{
	return m_primeMeridian;
}


const LinearUnit &GeocentricCS::getLinearUnit() const
{
	return m_linearUnit;
}


const Axis &GeocentricCS::getAxis1() const
{
	return m_axis1;
}


const Axis &GeocentricCS::getAxis2() const
{
	return m_axis2;
}


const Axis &GeocentricCS::getAxis3() const
{
	return m_axis3;
}


void GeocentricCS::parseWKT(const std::string &wkt)
{
	// <geocentric cs> = GEOCCS["<name>", <datum>, <prime meridian>, <linear unit> {,<axis>,
	//   <axis>, <axis>} {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse datum
	pos = wkt.find(g_DATUM, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_DATUM.length() + 1;
	m_datum.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse prime meridian
	pos = wkt.find(g_PRIMEM, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_PRIMEM.length() + 1;
	m_primeMeridian.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse linear unit
	pos = wkt.find(g_UNIT, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_UNIT.length() + 1;
	m_linearUnit.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse axes
	if(wkt.find(g_AXIS, pos) != std::string::npos)
	{
		pos = wkt.find(g_AXIS, pos);
		pos += g_AXIS.length() + 1;
		m_axis1.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";

		pos = wkt.find(g_AXIS, pos);
		if(pos == std::string::npos)
			throw "WKT syntax error";
		pos += g_AXIS.length() + 1;
		m_axis2.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


VerticalCS::VerticalCS()
{
}


VerticalCS::~VerticalCS()
{
}


const VerticalDatum &VerticalCS::getVerticalDatum() const
{
	return m_verticalDatum;
}


const LinearUnit &VerticalCS::getLinearUnit() const
{
	return m_linearUnit;
}


const Axis &VerticalCS::getAxis() const
{
	return m_axis;
}


void VerticalCS::parseWKT(const std::string &wkt)
{
	// <vert cs> = VERT_CS["<name>", <vert datum>, <linear unit>, {<axis>,}
	//   {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse vertical datum
	pos = wkt.find(g_VERT_DATUM, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_VERT_DATUM.length() + 1;
	m_verticalDatum.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse linear unit
	pos = wkt.find(g_UNIT, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_UNIT.length() + 1;
	m_linearUnit.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse axis
	if(wkt.find(g_AXIS, pos) != std::string::npos)
	{
		pos = wkt.find(g_AXIS, pos);
		pos += g_AXIS.length() + 1;
		m_axis.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


CompoundCS::CompoundCS()
{
}


CompoundCS::~CompoundCS()
{
}


const CoordinateSystem &CompoundCS::getHeadCS() const
{
	return *m_headCS;
}


const CoordinateSystem &CompoundCS::getTailCS() const
{
	return *m_tailCS;
}


void CompoundCS::parseWKT(const std::string &wkt)
{
	// <compd cs> = COMPD_CS["<name>", <head cs>, <tail cs> {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse head CS
	pos = wkt.find_first_of(g_IDENTIFIER_STARTCHARS, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_headCS = CoordinateSystem::CreateCS(wkt.substr(pos));
	pos = findMatchingBracket(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse tail CS
	pos = wkt.find_first_of(g_IDENTIFIER_STARTCHARS, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_tailCS = CoordinateSystem::CreateCS(wkt.substr(pos));
	pos = findMatchingBracket(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


FittedCS::FittedCS()
{
}


FittedCS::~FittedCS()
{
}


const MathTransform &FittedCS::getToBase() const
{
	return *m_toBase;
}


const CoordinateSystem &FittedCS::getBaseCS() const
{
	return *m_baseCS;
}


void FittedCS::parseWKT(const std::string &wkt)
{
	// <fitted cs> = FITTED_CS["<name>", <to base>, <base cs>]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse toBase
	pos = wkt.find_first_of(g_IDENTIFIER_STARTCHARS, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_toBase = MathTransform::CreateMT(wkt.substr(pos));
	pos = findMatchingBracket(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse base CS
	pos = wkt.find_first_of(g_IDENTIFIER_STARTCHARS, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_baseCS = CoordinateSystem::CreateCS(wkt.substr(pos));
	pos = findMatchingBracket(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	return;
}


LocalCS::LocalCS()
{
}


LocalCS::~LocalCS()
{
}


const LocalDatum &LocalCS::getLocalDatum() const
{
	return m_localDatum;
}


const boost::shared_ptr<Unit> &LocalCS::getUnit() const
{
	return m_unit;
}


const std::vector<Axis> &LocalCS::getAxes() const
{
	return m_axes;
}


void LocalCS::parseWKT(const std::string &wkt)
{
	// <local cs> = LOCAL_CS["<name>", <local datum>, <unit>, <axis>,
	//   {,<axis>}* {,<authority>}]

	// Parse name
	std::string::size_type pos = wkt.find('\"');
	if(pos == std::string::npos)
		throw "WKT syntax error";
	m_name = parseName(pos, wkt);

	// Parse local datum
	pos = wkt.find(g_LOCAL_DATUM, pos);
	if(pos == std::string::npos)
		throw "WKT syntax error";
	pos += g_LOCAL_DATUM.length() + 1;
	m_localDatum.parseWKT(wkt.substr(pos));
	pos = findMatchingBracket(pos - 1, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse unit
	pos = wkt.find_first_of(g_IDENTIFIER_STARTCHARS, pos);
	m_unit = Unit::CreateUnit(wkt.substr(pos));
	pos = findMatchingBracket(pos, wkt);
	if(pos == std::string::npos)
		throw "WKT syntax error";

	// Parse axes
	while(wkt.find(g_AXIS, pos) != std::string::npos)
	{
		pos = wkt.find(g_AXIS, pos);
		pos += g_AXIS.length() + 1;
		Axis axis;
		axis.parseWKT(wkt.substr(pos));
		m_axes.push_back(axis);
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	// Parse authority
	if(wkt.find(g_AUTHORITY, pos) != std::string::npos)
	{
		pos = wkt.find(g_AUTHORITY, pos);
		pos += g_AUTHORITY.length() + 1;
		m_authority.parseWKT(wkt.substr(pos));
		pos = findMatchingBracket(pos - 1, wkt);
		if(pos == std::string::npos)
			throw "WKT syntax error";
	}

	return;
}


std::string::size_type findMatchingBracket(std::string::size_type pos, const std::string &text)
{
	const std::string BRACKETS = "[{(<";
	const std::string MATCHES  = "]})>";

	if(pos < 0 || pos >= text.length() || text.find_first_of(BRACKETS, pos) == std::string::npos)
		return std::string::npos;   // Position doesn't indicate a bracket

	pos = text.find_first_of(BRACKETS, pos);
	char bracket = text[pos];
	char match = MATCHES[BRACKETS.find(bracket)];


	int level = 1;
	std::string::size_type pb, pm;
	do
	{
		pb = text.find(bracket, pos + 1);
		pm = text.find(match, pos + 1);
		
		if(pb < pm && pb != std::string::npos)
		{
			level++;
			pos = pb;
		}
		else
		{
			level--;
			pos = pm;
		}
	} while(level > 0 && pm != std::string::npos);


	return pm;
}


std::string parseName(std::string::size_type &pos, const std::string &wkt)
{
	if(wkt[pos] != '\"')
		return "";

	std::string::size_type pos2 = wkt.find('\"', pos + 1);
	std::string result;

	if(pos2 == std::string::npos)
		result = wkt.substr(pos + 1);
	else
		result = wkt.substr(pos + 1, pos2 - pos - 1);

	pos = pos2 + 1;
	return result;
}


double parseDouble(std::string::size_type &pos, const std::string &wkt)
{
	pos = wkt.find_first_of(g_DIGITS, pos);
	double result = atof(&wkt[pos]);
	pos = wkt.find_first_not_of(g_DIGITS, pos);
	
	if(wkt[pos] == 'e' || wkt[pos] == 'E')   // Skip exponent
		pos = wkt.find_first_not_of(g_DIGITS, pos + 1);

	return result;
}


int parseInt(std::string::size_type &pos, const std::string &wkt)
{
	pos = wkt.find_first_of(g_DIGITS, pos);
	int result = atoi(&wkt[pos]);
	pos = wkt.find_first_not_of(g_DIGITS, pos);

	return result;
}


std::string parseIdentifier(std::string::size_type &pos, const std::string &wkt)
{
	pos = wkt.find_first_of(g_IDENTIFIER_STARTCHARS, pos);
	std::string::size_type pos2 = wkt.find_first_not_of(g_IDENTIFIER_CHARS, pos);
	std::string result = wkt.substr(pos, pos2 - pos);
	pos = pos2;

	return result;
}
