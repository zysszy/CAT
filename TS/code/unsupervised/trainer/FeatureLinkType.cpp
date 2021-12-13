#include "FeatureLinkType.h"

/************************************************
  constructor
************************************************/
FeatureLinkType::FeatureLinkType()
{
}

/************************************************
  compute feature values
************************************************/
void FeatureLinkType::computeValues(const Alignment& alignment,
	                                float& o2oValue,
					                float& o2mValue,
					                float& m2oValue,
					                float& m2mValue) const
{
	o2oValue = 0.0;
	o2mValue = 0.0;
	m2oValue = 0.0;
	m2mValue = 0.0;

	for (int i = 0; i < (int)alignment.linkVec.size(); i++)
	{
		int sp = alignment.linkVec[i].first,
		    tp = alignment.linkVec[i].second,
			srcFert = (int)alignment.srcAligned[sp].size(),
			trgFert = (int)alignment.trgAligned[tp].size();

		// one-to-one
		if (srcFert == 1 && trgFert == 1)
		{
			o2oValue++;
		}
		// one-to-many
		else if (srcFert > 1 && trgFert == 1)
		{
			o2mValue++;
		}
		// many-to-one
		else if (srcFert == 1 && trgFert > 1)
		{
			m2oValue++;
		}
		// many-to-many
		else
		{
			m2mValue++;
		}
	}
}

/************************************************
  compute feature gains
************************************************/
void FeatureLinkType::computeGains(const Alignment& alignment,
	                               const pair<int, int>& link,
					               float& o2oGain,
					               float& o2mGain,
					               float& m2oGain,
					               float& m2mGain) const
{
	o2oGain = 0.0;
	o2mGain = 0.0;
	m2oGain = 0.0;
	m2mGain = 0.0;

	int srcFert = (int)alignment.srcAligned[link.first].size(),
	    trgFert = (int)alignment.trgAligned[link.second].size();

	// new link
	if (srcFert == 0 && trgFert == 0)
	{
		o2oGain++;
	}
	else if (srcFert > 0 && trgFert == 0)
	{
		o2mGain++;
	}
	else if (srcFert == 0 && trgFert > 0)
	{
		m2oGain++;
	}
	else
	{
		m2mGain++;
	}

	// existing links
	if (srcFert == 1)
	{
		int tp = alignment.srcAligned[link.first][0];

		if ((int)alignment.trgAligned[tp].size() == 1)
		{
			o2oGain--;
			o2mGain++;
		}
		else
		{
			m2oGain--;
			m2mGain++;
		}
	}

	if (trgFert == 1)
	{
		int sp = alignment.trgAligned[link.second][0];

		if ((int)alignment.srcAligned[sp].size() == 1)
		{
			o2oGain--;
			m2oGain++;
		}
		else
		{
			o2mGain--;
			m2mGain++;
		}
	}
}

