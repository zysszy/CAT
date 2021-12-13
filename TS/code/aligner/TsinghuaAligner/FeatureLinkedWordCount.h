#ifndef FEATURE_LINKED_WORD_COUNT_H
#define FEATURE_LINKED_WORD_COUNT_H

#include "Alignment.h"

class FeatureLinkedWordCount
{
public:
	/* member functions */
	// constructor
	FeatureLinkedWordCount();
	// compute feature values
	void computeValues(const Alignment& alignment,
	                   float& srcLWCValue,
					   float& trgLWCValue) const;
	// compute feature gains
	void computeGains(const Alignment& alignment,
	                  const pair<int, int>& link,
					  float& srcLWCGain,
					  float& trgLWCGain) const;
};

#endif
