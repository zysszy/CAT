#include <cmath>
#include "FeatureRelPosAbsDist.h"

/************************************************
  constructor
************************************************/
FeatureRelPosAbsDist::FeatureRelPosAbsDist()
{
}

/************************************************
  compute feature value
************************************************/
float FeatureRelPosAbsDist::computeValue(const vector<int>& srcIDVec,
	                                     const vector<int>& trgIDVec,
					                     const Alignment& alignment) const
{
	float value = 0.0;

	float srcSentLen = (float)srcIDVec.size() - 1.0,
	      trgSentLen = (float)trgIDVec.size() - 1.0;

	for (int i = 0; i < (int)alignment.linkVec.size(); i++)
	{
		value += fabs(alignment.linkVec[i].first / srcSentLen -
		              alignment.linkVec[i].second / trgSentLen);
	}

	return value;
}

/************************************************
  compute feature gain
************************************************/
float FeatureRelPosAbsDist::computeGain(const vector<int>& srcIDVec,
	                                    const vector<int>& trgIDVec,
					                    const Alignment& alignment,
					                    const pair<int, int>& link) const
{
	float gain = 0.0;

	float srcSentLen = (float)srcIDVec.size() - 1.0,
	      trgSentLen = (float)trgIDVec.size() - 1.0;

	gain = fabs(link.first / srcSentLen - link.second / trgSentLen);

	return gain;
}

