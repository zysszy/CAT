#include "FeatureCrossCount.h"

/************************************************
  constructor
************************************************/
FeatureCrossCount::FeatureCrossCount()
{
}

/************************************************
  compute feature value
************************************************/
float FeatureCrossCount::computeValue(const Alignment& alignment) const
{
	float value = 0.0;

	for (int i = 0; i < (int)alignment.linkVec.size(); i++)
	{
		pair<int, int> l1 = alignment.linkVec[i];

		for (int j = i + 1; j < (int)alignment.linkVec.size(); j++)
		{
			pair<int, int> l2 = alignment.linkVec[j];

			if ((l1.first - l2.first) * (l1.second - l2.second) < 0)
			{
				value++;
			}
		}
	}

	return value;
}

/************************************************
  compute feature gain
************************************************/
float FeatureCrossCount::computeGain(const Alignment& alignment,
	                                 const pair<int, int>& link) const
{
	float gain = 0.0;

	for (int i = 0; i < (int)alignment.linkVec.size(); i++)
	{
		if ((alignment.linkVec[i].first - link.first) * (alignment.linkVec[i].second - link.second) < 0)
		{
			gain++;
		}
	}

	return gain;
}

