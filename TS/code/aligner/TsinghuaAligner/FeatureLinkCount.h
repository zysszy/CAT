#ifndef FEATURE_LINK_COUNT_H
#define FEATURE_LINK_COUNT_H

#include "Alignment.h"

class FeatureLinkCount
{
public:
	/* member functions */
	// constructors
	FeatureLinkCount();
	// compute feature value
	float computeValue(const Alignment& alignment) const;
	// compute feature gain
	float computeGain() const;
};

#endif
