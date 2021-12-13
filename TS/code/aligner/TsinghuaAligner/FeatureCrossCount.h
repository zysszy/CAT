#ifndef FEATURE_CROSS_COUNT_H
#define FEATURE_CROSS_COUNT_H

#include "Alignment.h"

class FeatureCrossCount
{
public:
	/* member functions */
	// constructor
	FeatureCrossCount();
	// compute feature value
	float computeValue(const Alignment& alignment) const;
	// compute feature gain
	float computeGain(const Alignment& alignment,
	                  const pair<int, int>& link) const;
};

#endif
