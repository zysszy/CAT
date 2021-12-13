#include "FeatureNeighborCount.h"

/************************************************
  constructor
************************************************/
FeatureNeighborCount::FeatureNeighborCount()
{
}

/************************************************
  compute feature values
************************************************/
void FeatureNeighborCount::computeValues(const Alignment& alignment,
	                                     float& monoNCValue,
					                     float& swapNCValue) const
{
	monoNCValue = 0.0;
	swapNCValue = 0.0;

	for (int i = 0; i < (int)alignment.linkVec.size(); i++)
	{
		for (int j = i + 1; j < (int)alignment.linkVec.size(); j++)
		{
			int product = (alignment.linkVec[i].first - alignment.linkVec[j].first) *
			              (alignment.linkVec[i].second - alignment.linkVec[j].second);

			if (product == 1)
			{
				monoNCValue++;
			}

			if (product == -1)
			{
				swapNCValue++;
			}
		}
	}
}

/************************************************
  compute feature gains
************************************************/
void FeatureNeighborCount::computeGains(const Alignment& alignment,
                                        const pair<int, int>& link,
	                                    float& monoNCGain,
					                    float& swapNCGain) const
{
	monoNCGain = 0.0;
	swapNCGain = 0.0;

	for (int i = 0; i < (int)alignment.linkVec.size(); i++)
	{
		int product = (alignment.linkVec[i].first - link.first) *
		              (alignment.linkVec[i].second - link.second);

		if (product == 1)
		{
			monoNCGain++;
		}

		if (product == -1)
		{
			swapNCGain++;
		}
	}
}
