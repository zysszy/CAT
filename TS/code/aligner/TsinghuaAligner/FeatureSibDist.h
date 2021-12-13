#ifndef FEATURE_SIB_DIST_H
#define FEATURE_SIB_DIST_H

#include "Alignment.h"

class FeatureSibDist
{
public:
	/* member functions */
	// constructor
	FeatureSibDist();
	// compute feature values
	void computeValues(const Alignment& alignment,
	                   float& srcSDValue,
					   float& trgSDValue) const;
	// compute feature gains
	void computeGains(const Alignment& alignment,
	                  const pair<int, int>& link,
					  float& srcSDGain,
					  float&trgSDGain) const;
};

#endif
