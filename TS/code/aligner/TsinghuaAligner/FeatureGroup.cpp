#include <cmath>
#include "FeatureGroup.h"

/************************************************
  constructor
************************************************/
FeatureGroup::FeatureGroup()
{
}

/************************************************
  initialize
************************************************/
void FeatureGroup::init(TTable* s2tTTablePtr,
	                    TTable* t2sTTablePtr)
{
	featTPP.init(s2tTTablePtr, t2sTTablePtr);
}

/************************************************
  compute score
************************************************/
void FeatureGroup::computeScore(const vector<int>& srcIDVec,
	                            const vector<int>& trgIDVec,
					            Alignment& alignment,
					            const vector<float>& weightVec) const
{
	// initialize feature value vector
	vector<float> valueVec(weightVec.size(), 0.0);

	// translation probability product
	if (fabs(weightVec[0]) > 1e-7)
	{
		valueVec[0] = featTPP.computeValue(srcIDVec, trgIDVec, alignment);
	}

	// link count
	if (fabs(weightVec[1]) > 1e-7)
	{
		valueVec[1] = featLC.computeValue(alignment);
	}

	// relative position absolute distance
	if (fabs(weightVec[2]) > 1e-7)
	{
		valueVec[2] = featRPAD.computeValue(srcIDVec, trgIDVec, alignment);
	}

	// cross count
	if (fabs(weightVec[3]) > 1e-7)
	{
		valueVec[3] = featCC.computeValue(alignment);
	}

	// neighbor count
	if (fabs(weightVec[4]) > 1e-7 ||
		fabs(weightVec[5]) > 1e-7)
	{
		featNC.computeValues(alignment, valueVec[4], valueVec[5]);
	}

	// linked word count
	if (fabs(weightVec[6]) > 1e-7 ||
		fabs(weightVec[7]) > 1e-7)
	{
		featLWC.computeValues(alignment, valueVec[6], valueVec[7]);
	}

	// maximal fertility
	if (fabs(weightVec[8]) > 1e-7 ||
		fabs(weightVec[9]) > 1e-7)
	{
		featMF.computeValues(alignment, valueVec[8], valueVec[9]);
	}

	// sibling distance
	if (fabs(weightVec[10]) > 1e-7 ||
		fabs(weightVec[11]) > 1e-7)
	{
		featSD.computeValues(alignment, valueVec[10], valueVec[11]);
	}

	// link type
	if (fabs(weightVec[12]) > 1e-7 ||
		fabs(weightVec[13]) > 1e-7 ||
		fabs(weightVec[14]) > 1e-7 ||
		fabs(weightVec[15]) > 1e-7)
	{
		featLT.computeValues(alignment, valueVec[12], valueVec[13], valueVec[14], valueVec[15]);
	}

	// compute score
	alignment.fvv = valueVec;
	alignment.score = dotProduct(weightVec, alignment.fvv);
}

void FeatureGroup::computeScore(const vector<int>& srcIDVec,
	                            const vector<int>& trgIDVec,
					            Alignment& alignment,
					            const pair<int, int>& link,
					            const vector<float>& weightVec) const
{
	// initialize the gain vector
	vector<float> gainVec(weightVec.size(), 0.0);
	
	// translation probability product
	if (fabs(weightVec[0]) > 1e-7)
	{
		gainVec[0] = featTPP.computeGain(srcIDVec, trgIDVec, alignment, link);
	}

	// link count
	if (fabs(weightVec[1]) > 1e-7)
	{
		gainVec[1] = featLC.computeGain();
	}

	// relative position absolute distance
	if (fabs(weightVec[2]) > 1e-7)
	{
		gainVec[2] = featRPAD.computeGain(srcIDVec, trgIDVec, alignment, link);
	}

	// cross count
	if (fabs(weightVec[3]) > 1e-7)
	{
		gainVec[3] = featCC.computeGain(alignment, link);
	}

	// neighbor count
	if (fabs(weightVec[4]) > 1e-7 ||
		fabs(weightVec[5]) > 1e-7)
	{
		featNC.computeGains(alignment, link, gainVec[4], gainVec[5]);
	}

	// linked word count
	if (fabs(weightVec[6]) > 1e-7 ||
		fabs(weightVec[7]) > 1e-7)
	{
		featLWC.computeGains(alignment, link, gainVec[6], gainVec[7]);
	}

	// maximal fertility
	if (fabs(weightVec[8]) > 1e-7 ||
		fabs(weightVec[9]) > 1e-7)
	{
		featMF.computeGains(alignment, link, gainVec[8], gainVec[9]);
	}

	// sibling distance
	if (fabs(weightVec[10]) > 1e-7 ||
		fabs(weightVec[11]) > 1e-7)
	{
		featSD.computeGains(alignment, link, gainVec[10], gainVec[11]);
	}

	// link type
	if (fabs(weightVec[12]) > 1e-7 ||
		fabs(weightVec[13]) > 1e-7 ||
		fabs(weightVec[14]) > 1e-7 ||
		fabs(weightVec[15]) > 1e-7)
	{
		featLT.computeGains(alignment, link, gainVec[12], gainVec[13], gainVec[14], gainVec[15]);
	}

	// update the alignment
	alignment.addLink(link);

	for (int i = 0; i < (int)gainVec.size(); i++)
	{
		alignment.fvv[i] += gainVec[i];
	}

	alignment.score = dotProduct(weightVec, alignment.fvv);
}

/************************************************
  dot product
************************************************/
float FeatureGroup::dotProduct(const vector<float>& v1,
	                           const vector<float>& v2) const
{
	float product = 0.0;

	for (int i = 0; i < (int)v1.size() && i < (int)v2.size(); i++)
	{
		product += v1[i] * v2[i];
	}

	return product;
}

