// ImageCoord.cpp
// FS, 23-12-2011

#include "Rectify.h"


ImageCoord::ImageCoord(const std::vector<Anchor> &anchors):
	m_anchors(anchors)
{
}

// This method calculates the 'w' and 'a' params for the Thin Plate Spline (TPS) algorithm
void ImageCoord::calculateTPSParams() const
{
	//std::cout << "calculateTPSParams" << std::endl;

	if(!m_wParams_x.empty())
		return;   // Calculations already done

	const double lambda = 0;
	unsigned int p = static_cast<unsigned int>(m_anchors.size());


	// Calculate z array
	std::vector<double> z_x, z_y, z_x_inv, z_y_inv;
	for(unsigned int idx = 0; idx < p; ++idx)
	{
		//showVal(m_anchors[idx].rect);
		z_x.push_back(m_anchors[idx].orig.x - m_anchors[idx].rect.x);
		z_y.push_back(m_anchors[idx].orig.y - m_anchors[idx].rect.y);
		z_x_inv.push_back(m_anchors[idx].rect.x - m_anchors[idx].orig.x);
		z_y_inv.push_back(m_anchors[idx].rect.y - m_anchors[idx].orig.y);
	}


	// Calculate difference matrix
	Matrix<double> Diff(p, p), Diff_inv(p, p);
	double alpha = 0, alpha_inv = 0;   // will be average of all distances
	for(unsigned int i = 0; i < p; ++i)
	{
		for(unsigned int j = i + 1; j < p; ++j)
		{
			Diff[i][j] = Diff[j][i] = dist(m_anchors[i].rect, m_anchors[j].rect);
			alpha += Diff[i][j];

			Diff_inv[i][j] = Diff_inv[j][i] = dist(m_anchors[i].orig, m_anchors[j].orig);
			alpha_inv += Diff_inv[i][j];
		}
	}
	alpha /= pow2(static_cast<double>(p));
	alpha_inv /= pow2(static_cast<double>(p));


	// Create and fill K matrix
	Matrix<double> K(p + 3, p + 3), K_inv(p + 3, p + 3);
	for(unsigned int i = 0; i < p; ++i)
	{
		for(unsigned int j = 0; j < p; ++j)
		{
			if(i == j)
			{
				K[i][j] = pow2(alpha) * lambda;
				K_inv[i][j] = pow2(alpha_inv) * lambda;
			}
			else
			{
				K[i][j] = U(Diff[i][j]);
				K_inv[i][j] = U(Diff_inv[i][j]);
			}
		}

		K[i][p] = K[p][i] = 1;
		K[i][p + 1] = K[p + 1][i] = m_anchors[i].rect.x;
		K[i][p + 2] = K[p + 2][i] = m_anchors[i].rect.y;

		K_inv[i][p] = K_inv[p][i] = 1;
		K_inv[i][p + 1] = K_inv[p + 1][i] = m_anchors[i].orig.x;
		K_inv[i][p + 2] = K_inv[p + 2][i] = m_anchors[i].orig.y;
	}


	MatrixRow<double> b_x(p + 3), b_y(p + 3), b_x_inv(p + 3), b_y_inv(p + 3);
	for(unsigned int i = 0; i < p; ++i)
	{
		b_x[i] = z_x[i];
		b_y[i] = z_y[i];

		b_x_inv[i] = z_x_inv[i];
		b_y_inv[i] = z_y_inv[i];
	}

	MatrixRow<double> wa_x = K.solve(b_x);
	MatrixRow<double> wa_y = K.solve(b_y);
	MatrixRow<double> wa_x_inv = K_inv.solve(b_x_inv);
	MatrixRow<double> wa_y_inv = K_inv.solve(b_y_inv);


	// Fill w and a arrays
	for(unsigned int i = 0; i < p; ++i)
	{
		m_wParams_x.push_back(wa_x[i]);
		m_wParams_y.push_back(wa_y[i]);
		m_wInvParams_x.push_back(wa_x_inv[i]);
		m_wInvParams_y.push_back(wa_y_inv[i]);
	}

	for(unsigned int idx = 0; idx < 3; ++idx)
	{
		m_aParams_x.push_back(wa_x[p + idx]);
		m_aParams_y.push_back(wa_y[p + idx]);
		m_aInvParams_x.push_back(wa_x_inv[p + idx]);
		m_aInvParams_y.push_back(wa_y_inv[p + idx]);
	}


	return;
}

ToImageCoord::ToImageCoord(const std::vector<Anchor> &anchors):
	ImageCoord(anchors)
{
}

PixelCoord ToImageCoord::execute(const CoordXY &mapCoord) const
{
	if(m_wParams_x.empty())
		calculateTPSParams();

	double dx = m_aParams_x[0] + m_aParams_x[1] * mapCoord.x + m_aParams_x[2] * mapCoord.y;
	double dy = m_aParams_y[0] + m_aParams_y[1] * mapCoord.x + m_aParams_y[2] * mapCoord.y;

	std::vector<double>::const_iterator iterWX = m_wParams_x.begin();
	std::vector<double>::const_iterator iterWY = m_wParams_y.begin();
	std::vector<Anchor>::const_iterator iterAnchor = m_anchors.begin();

	for(; iterAnchor != m_anchors.end(); ++iterAnchor, ++iterWX, ++iterWY)
	{
		double dDist2 = dist2(iterAnchor->rect, mapCoord);
		if(dDist2 < EPSILON2)
		{
			// Exact coord found
			return iterAnchor->orig;
		}

		double u = dDist2 * log(dDist2) * 0.5;
		dx += *iterWX * u;
		dy += *iterWY * u;
	}

	return PixelCoord(mapCoord.x + dx, mapCoord.y + dy);
}

FromImageCoord::FromImageCoord(const std::vector<Anchor> &anchors):
	ImageCoord(anchors)
{
}

CoordXY FromImageCoord::execute(const PixelCoord &imageCoord) const
{
	if(m_wParams_x.empty())
		calculateTPSParams();

	double dx = m_aInvParams_x[0] + m_aInvParams_x[1] * imageCoord.x + m_aInvParams_x[2] * imageCoord.y;
	double dy = m_aInvParams_y[0] + m_aInvParams_y[1] * imageCoord.x + m_aInvParams_y[2] * imageCoord.y;

	for(unsigned int i = 0; i < m_anchors.size(); ++i)
	{
		double dDist = dist(m_anchors[i].orig, imageCoord);
		if(dDist < EPSILON)
		{
			//Exact coord found
			return m_anchors[i].rect;
		}

		dx += m_wInvParams_x[i] * U(dDist);
		dy += m_wInvParams_y[i] * U(dDist);
	}

	return CoordXY(imageCoord.x + dx, imageCoord.y + dy);
}

ImageProjection::ImageProjection(const std::vector<Anchor> &anchors):
	m_anchors(anchors)
{
	// Beware: m_anchors is a _copy_ of anchors, so this copy needs to be passed to the To- and From-classes.
	m_tp = boost::shared_ptr<ToImageCoord>(new ToImageCoord(m_anchors));
	m_fp = boost::shared_ptr<FromImageCoord>(new FromImageCoord(m_anchors));
}

const boost::shared_ptr<ToImageCoord> &ImageProjection::getToImageCoord() const
{
	return m_tp;
}

const boost::shared_ptr<FromImageCoord> &ImageProjection::getFromImageCoord() const
{
	return m_fp;
}

std::vector<Anchor> &ImageProjection::getAnchors()
{
	return m_anchors;
}

//ImageCS::ImageCS()
//{
//}
//
//ImageCS::~ImageCS()
//{
//}
