#include "FeatureMaxFertility.h"

/************************************************
  constructor
************************************************/
FeatureMaxFert::FeatureMaxFert()
{
}

/************************************************
  compute values
************************************************/
void FeatureMaxFert::computeValues(const Alignment& alignment,
	                               float& srcMFValue,
					               float& trgMFValue) const
{
	srcMFValue = alignment.srcMaxFert;
	trgMFValue = alignment.trgMaxFert;
}

/************************************************
  compute gains
************************************************/
void FeatureMaxFert::computeGains(const Alignment& alignment,
                                  const pair<int, int>& link,
	                              float& srcMFGain,
					              float& trgMFGain) const
{
	int newSrcFert = (int)alignment.srcAligned[link.first].size() + 1,
	    newTrgFert = (int)alignment.trgAligned[link.second].size() + 1;

	if (newSrcFert > alignment.srcMaxFert)
	{
		srcMFGain = newSrcFert - alignment.srcMaxFert;
	}

	if (newTrgFert > alignment.trgMaxFert)
	{
		trgMFGain = newTrgFert - alignment.trgMaxFert;
	}
}
