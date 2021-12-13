#ifndef FEATURE_REL_POS_ABS_DIST_H
#define FEATURE_REL_POS_ABS_DIST_H

#include "Alignment.h"

class FeatureRelPosAbsDist
{
public:
	/* member functions */
	// constructor
	FeatureRelPosAbsDist();
	// compute feature value
	float computeValue(const vector<int>& srcIDVec,
	                   const vector<int>& trgIDVec,
					   const Alignment& alignment) const;
	// compute feature gain
	float computeGain(const vector<int>& srcIDVec,
	                  const vector<int>& trgIDVec,
					  const Alignment& alignment,
					  const pair<int, int>& link) const;
};

#endif
