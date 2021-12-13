#include "FeatureLinkCount.h"

/************************************************
  constructors
************************************************/
FeatureLinkCount::FeatureLinkCount()
{
}

/************************************************
  compute feature value
************************************************/
float FeatureLinkCount::computeValue(const Alignment& alignment) const
{
	return (float)alignment.linkVec.size();
}

/************************************************
  compute feature gain
************************************************/
float FeatureLinkCount::computeGain() const
{
	return 1.0;
}

