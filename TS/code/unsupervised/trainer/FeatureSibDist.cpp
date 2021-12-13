#include "FeatureSibDist.h"

/************************************************
  constructor
************************************************/
FeatureSibDist::FeatureSibDist()
{
}

/************************************************
  compute feature values
************************************************/
void FeatureSibDist::computeValues(const Alignment& alignment,
	                               float& srcSDValue,
					               float& trgSDValue) const
{
	srcSDValue = 0.0;
	trgSDValue = 0.0;

	for (int i = 1; i < (int)alignment.srcAligned.size(); i++)
	{
		if (!alignment.srcAligned[i].empty())
		{
			srcSDValue += alignment.srcAligned[i].back() -
			              alignment.srcAligned[i][0] - 
						  (int)alignment.srcAligned.size() +
						  1.0;
		}
	}

	for (int i = 1; i < (int)alignment.trgAligned.size(); i++)
	{
		if (!alignment.trgAligned[i].empty())
		{
			trgSDValue += alignment.trgAligned[i].back() -
			              alignment.trgAligned[i][0] -
						  (int)alignment.trgAligned.size() + 
						  1.0;
		}
	}
}

/************************************************
  compute feature gains
************************************************/
void FeatureSibDist::computeGains(const Alignment& alignment,
	                              const pair<int, int>& link,
					              float& srcSDGain,
					              float& trgSDGain) const
{
	srcSDGain = 0.0;
	trgSDGain = 0.0;

	if (!alignment.srcAligned[link.first].empty())
	{
		// left
		if (link.second < alignment.srcAligned[link.first][0])
		{
			srcSDGain = alignment.srcAligned[link.first][0] - link.second - 1;
		}
		// right
		else if (link.second > alignment.srcAligned[link.first].back())
		{
			srcSDGain = link.second - alignment.srcAligned[link.first].back() - 1;
		}
		// middle
		else
		{
			srcSDGain = -1;
		}
	}

	if (!alignment.trgAligned[link.second].empty())
	{
		// left
		if (link.first < alignment.trgAligned[link.second][0])
		{
			trgSDGain = alignment.trgAligned[link.second][0] - link.first - 1;
		}
		// right
		else if (link.first > alignment.trgAligned[link.second].back())
		{
			trgSDGain = link.first - alignment.trgAligned[link.second].back() - 1;
		}
		// middle
		else
		{
			trgSDGain = -1;
		}
	}
}
