#ifndef FEATURE_NEIGHBOR_COUNT_H
#define FEATURE_NEIGHBOR_COUNT_H

#include "Alignment.h"

class FeatureNeighborCount
{
public:
	/* member functions */
	// constructor
	FeatureNeighborCount();
	// compute feature values
	void computeValues(const Alignment& alignment,
	                   float& monoNCValue,
					   float& swapNCValue) const;
	// compute feature gains
	void computeGains(const Alignment& alignment,
	                  const pair<int, int>& link,
	                  float& monoNCGain,
					  float& swapNCGain) const;
};

#endif
