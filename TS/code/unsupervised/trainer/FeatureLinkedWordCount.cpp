#include "FeatureLinkedWordCount.h"

/************************************************
  constructor
************************************************/
FeatureLinkedWordCount::FeatureLinkedWordCount()
{
}

/************************************************
  compute feature values
************************************************/
void FeatureLinkedWordCount::computeValues(const Alignment& alignment,
	                                       float& srcLWCValue,
					                       float& trgLWCValue) const
{
	srcLWCValue = 0.0;
	trgLWCValue = 0.0;

	for (int i = 1; i < (int)alignment.srcAligned.size(); i++)
	{
		if (!alignment.srcAligned[i].empty())
		{
			srcLWCValue++;
		}
	}

	for (int i = 1; i < (int)alignment.trgAligned.size(); i++)
	{
		if (!alignment.trgAligned[i].empty())
		{
			trgLWCValue++;
		}
	}
}

/************************************************
  compute feature gains
************************************************/
void FeatureLinkedWordCount::computeGains(const Alignment& alignment,
	                                      const pair<int, int>& link,
					                      float& srcLWCGain,
					                      float& trgLWCGain) const
{
	srcLWCGain = 0.0;
	trgLWCGain = 0.0;

	if (alignment.srcAligned[link.first].empty())
	{
		srcLWCGain = 1.0;
	}

	if (alignment.trgAligned[link.second].empty())
	{
		trgLWCGain = 1.0;
	}
}
