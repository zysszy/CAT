#ifndef FEATURE_MAXIMAL_FERTILITY_H
#define FEATURE_MAXIMAL_FERTILITY_H

#include "Alignment.h"

class FeatureMaxFert
{
public:
	/* member functions */
	// constructor
	FeatureMaxFert();
	// compute values
	void computeValues(const Alignment& alignment,
	                   float& srcMFValue,
					   float& trgMFValue) const;
	// compute gains
	void computeGains(const Alignment& alignment,
	                  const pair<int, int>& link,
	                  float& srcMFGain,
					  float& trgMFGain) const;
};

#endif
