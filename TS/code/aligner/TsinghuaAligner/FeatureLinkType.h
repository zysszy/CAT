#ifndef FEATURE_LINK_TYPE_H
#define FEATURE_LINK_TYPE_H

#include "Alignment.h"

class FeatureLinkType
{
public:
	/* member functions */
	// constructor
	FeatureLinkType();
	// compute feature values
	void computeValues(const Alignment& alignment,
	                   float& o2oValue,
					   float& o2mValue,
					   float& m2oValue,
					   float& m2mValue) const;
	// compute feature gains
	void computeGains(const Alignment& alignment,
	                  const pair<int, int>& link,
					  float& o2oGain,
					  float& o2mGain,
					  float& m2oGain,
					  float& m2mGain) const;
};

#endif
